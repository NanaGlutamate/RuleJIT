#pragma once
/**
 * @file semantic.hpp
 * @author djw
 * @brief FrontEnd/Semantic
 * @date 2023-03-28
 *
 * @details Includes ExpressionSemantic which does semantic analysis
 * TODO: template restriction
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-30</td><td>Change function check logic.</td></tr>
 * <tr><td>djw</td><td>2023-04-20</td><td>Change extern function logic, and make vardef whose name same with func
 * legal.</td></tr>
 * <tr><td>djw</td><td>2023-04-21</td><td>Add template support.</td></tr>
 * <tr><td>djw</td><td>2023-04-23</td><td>Add pure lambda support.</td></tr>
 * </table>
 */

#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <unordered_map>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "ast/escapedanalyzer.hpp"
#include "defines/language.hpp"
#include "frontend/parser.h"
#include "frontend/semantic.hpp"
#include "tools/myassert.hpp"
#include "tools/pointercast.hpp"
#include "tools/seterror.hpp"
#include "tools/stringprocess.hpp"

#define __RULEJIT_SEMANTIC_PUSH pushStack(v.get());
#define __RULEJIT_SEMANTIC_POP popStack();

namespace rulejit {

constexpr inline auto reservedPrefix = "buildIn";

/**
 * @brief main class for semantic analyzer
 *
 * @details jobs:
 *
 * 1. type inference
 * 2. TODO: name unnamed complex type(in vardef, typedef, literal and funcdef)
 * 3. scope process
 * 4. TODO: capture analysis
 * 5. TODO: make extern func consistent with normal func def(add real_func_name directly?)
 *
 */
struct ExpressionSemantic : public ASTVisitor {
    ExpressionSemantic(ContextStack &context) : c(context), needChange(nullptr){};
    ExpressionSemantic(const ExpressionSemantic &) = delete;
    ExpressionSemantic(ExpressionSemantic &&) = delete;
    ExpressionSemantic &operator=(const ExpressionSemantic &) = delete;
    ExpressionSemantic &operator=(ExpressionSemantic &&) = delete;

    /**
     * @brief get the call stack
     * 
     * @return std::vector<ExprAST*>& reference to call stack
     */
    std::vector<ExprAST *> &getCallStack() { return callStack; }

    /**
     * @brief pipe operator| used for file parsing
     *
     * @param parser parser which already loaded lexer which load with file content
     * @param semantic receiver ExpressionSemantic
     * @return std::string real function name which contains top-level expressions and global variable assignment
     */
    std::string friend operator|(ExpressionParser &parser, ExpressionSemantic &semantic) {
        std::vector<std::unique_ptr<ExprAST>> topLevelExprs;
        std::unique_ptr<ExprAST> tmp;
        while ((tmp = parser) != nullptr) {
            topLevelExprs.push_back(std::move(tmp));
        }
        return semantic.addUnnamedFunction(std::move(topLevelExprs));
    }

    /**
     * @brief pipe operator| used for repl
     *
     * @param ast AST of current input
     * @param semantic receiver ExpressionSemantic
     * @return std::string real function name which contains current input expression
     */
    std::string friend operator|(std::unique_ptr<ExprAST> ast, ExpressionSemantic &semantic) {
        std::vector<std::unique_ptr<ExprAST>> tmp;
        tmp.push_back(std::move(ast));
        return semantic.addUnnamedFunction(std::move(tmp));
    }

    /**
     * @brief check function with given real name. automatically check all
     * unchecked function this function calls.
     *
     * @attention a function is marked checked if and only if itself checked, discard it's dependencies
     *
     * @param name real function name of given function
     */
    void checkFunction(const std::string &name) {
        init();
        if (globalInfo().externFuncDef.contains(name)) {
            // do not check extern func
            return;
        }
        std::set<std::string> checkedTemp;
        auto tmp = checkSingleRealFunction(name, checkedTemp);
        checkRealFunctionDependencySet(tmp, checkedTemp);
    }

  protected:
    VISIT_FUNCTION(IdentifierExprAST) {
        auto [find, type] = c.seekVarDef(v.name);
        if (find) {
            // var
            v.type = std::make_unique<TypeInfo>(type);
            processType(*v.type);
        } else if (auto [findc, literal] = c.seekConstDef(v.name); findc) {
            if (std::get<0>(literal).isFunctionType()) {
                // func act as const global
                funcDependencyRealName.insert(std::get<1>(literal));
            }
            needChange = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(std::get<0>(literal)),
                                                          std::get<1>(literal));
            processType(*needChange->type);
        } else {
            return setError(std::format("Variable \"{}\" not defined", v.name));
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        callAccept(v.baseVar);
        callAccept(v.memberToken);
        if (isType<LiteralExprAST>(v.memberToken.get())) {
            auto p = dynamic_cast<LiteralExprAST *>(v.memberToken.get());
            if (*(p->type) == StringType && v.baseVar->type->isBaseType()) {
                // struct member access; TODO: access overload
                auto definedType = globalInfo().typeDef.find(v.baseVar->type->getBaseTypeString());
                if (definedType == globalInfo().typeDef.end()) {
                    return setError(std::format("Type \"{}\" not defined", v.baseVar->type->getBaseTypeString()));
                }
                auto memberType = definedType->second.getMemberType(p->value);
                v.type = std::make_unique<TypeInfo>(memberType);
                processType(*v.type);
                return;
            }
        }
        if (v.baseVar->type->isArrayType()) {
            // array access
            if (*(v.memberToken->type) != RealType && *(v.memberToken->type) != IntType) {
                return setError(std::format("array index must be real or int type, but get \"{}\"",
                                            v.memberToken->type->toString()));
            }
            v.type = std::make_unique<TypeInfo>(v.baseVar->type->getElementType());
        } else {
            return setError(std::format("unknown member access {}[\"{}\"]", v.baseVar->type->toString(),
                                        v.memberToken->type->toString()));
        }
        processType(*v.type);
    }
    VISIT_FUNCTION(LiteralExprAST) {
        my_assert(bool(v.type));
        if (v.type->isFunctionType()) {
            if (auto it = globalInfo().realFuncDefinition.find(v.value); it != globalInfo().realFuncDefinition.end()) {
                auto realName = it->first;
                funcDependencyRealName.insert(realName);
            } else if (auto it = globalInfo().externFuncDef.find(v.value); it != globalInfo().externFuncDef.end()) {
                // do nothing
                auto realName = it->first;
                funcDependencyRealName.insert(realName);
            } else {
                // build in template function for array
                // TODO: must match array function provided in VISIT_FUNCTION(FunctionCallExprAST)
                if (v.value != "length" && v.value != "resize" && v.value != "push") {
                    return setError(std::format("Real function name \"{}\" not found", v.value));
                }
            }
        }
        processType(*v.type);
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        for (auto &arg : v.params) {
            callAccept(arg);
        }
        if (auto [p, p1] = getConstStringMemberAccessInfo(v.functionIdent.get()); p && p1) {
            // maybe a member function call
            callAccept(p->baseVar);

            std::vector<TypeInfo> paramType{*(p->baseVar->type)};
            for (auto &arg : v.params) {
                paramType.push_back(*(arg->type));
            }
            bool isMember = canAccess(*p);
            auto [isMemberFunc, realFuncName] = seekMemberFunc(p1->value, paramType);
            if (!isMember && isMemberFunc) {
                // exactly member func, transform
                v.params.insert(v.params.begin(), unique_cast<ExprAST>(p->baseVar));
                v.functionIdent = std::make_unique<LiteralExprAST>(
                    std::make_unique<TypeInfo>(c.getRealFunctionType(realFuncName)), realFuncName);
                // after change, continue to process v, not return
            } else if (isMember && isMemberFunc) {
                return setError(std::format("Member and member function have same name: {}::{}",
                                            p->baseVar->type->toString(), p1->value));
            } else if (!isMember && !isMemberFunc) {
                if (p->baseVar->type->isArrayType()) {
                    // member functions every array has
                    static const std::map<std::string, TypeInfo> arrayMemberFunc{
                        {"length", make_type("func([]T)->f64")},
                        {"resize", make_type("func([]T, f64)")},
                        {"push", make_type("func([]T, T)")},
                        // pop, back
                    };
                    std::map<std::string, TypeInfo> spec{{"T", p->baseVar->type->getElementType()}};
                    if (auto it = arrayMemberFunc.find(p1->value); it != arrayMemberFunc.end()) {
                        auto specType = it->second | TypeInfo::where(spec);
                        v.params.insert(v.params.begin(), unique_cast<ExprAST>(p->baseVar));
                        v.functionIdent =
                            std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(specType), p1->value);
                    } else {
                        return setError(
                            std::format("No member function: {}::{}", p->baseVar->type->toString(), p1->value));
                    }
                } else {
                    setError(
                        std::format("No member or member function: {}::{}", p->baseVar->type->toString(), p1->value));
                }
            }
        }

        if (auto p = dynamic_cast<IdentifierExprAST *>(v.functionIdent.get());
            p && !std::get<0>(c.seekVarDef(p->name)) && !std::get<0>(c.seekConstDef(p->name))) {
            // template function call
            if (auto it = globalInfo().templateFuncDef.find(p->name); it != globalInfo().templateFuncDef.end()) {
                std::vector<TypeInfo> paramType;
                for (auto &arg : v.params) {
                    paramType.push_back(*(arg->type));
                }
                if (auto insit = it->second.instantiationRealName.find(paramType);
                    insit != it->second.instantiationRealName.end()) {
                    v.functionIdent = std::make_unique<LiteralExprAST>(
                        std::make_unique<TypeInfo>(c.getRealFunctionType(insit->second)), insit->second);
                } else {
                    auto [instantiated, matched] = it->second.instantiate(paramType);
                    if (instantiated) {
                        auto templateParamList =
                            it->second.paramNames |
                            std::views::transform([&](const std::string &param) { return matched[param].toString(); }) |
                            mystr::join(",");
                        auto name = std::format("<{}>{}", templateParamList, instantiated->name);
                        auto realName = c.generateUniqueName(reservedPrefix, name + instantiated->funcType->toString());
                        globalInfo().realFuncDefinition.emplace(realName, std::move(instantiated));
                        funcDependencyRealName.insert(realName);
                        it->second.instantiationRealName.emplace(std::move(paramType), std::move(realName));
                    } else {
                        setError(std::format("No template function match: {}({})", p->name,
                                             paramType | std::views::transform(&TypeInfo::toString) | mystr::join(", ")));
                    }
                }
            }
        }

        callAccept(v.functionIdent);

        if (!v.functionIdent->type->isFunctionType()) {
            return setError(std::format("Cannot call to non-function type \"{}\"", v.functionIdent->type->toString()));
        }
        auto needParamNum = v.functionIdent->type->getParamCount();
        if (needParamNum != v.params.size()) {
            return setError(std::format("Function need {} params, {} given", needParamNum, v.params.size()));
        }
        for (size_t i = 0; i < v.params.size(); i++) {
            if (*(v.params[i]->type) != v.functionIdent->type->getParamType(i)) {
                return setError(std::format(
                    "Arg number {} type mismatch in call to function type \"{}\", \"{}\" required, \"{}\" given", i,
                    v.functionIdent->type->toString(), v.params[i]->type->toString(),
                    v.functionIdent->type->getParamType(i).toString()));
            }
        }
        v.type = std::make_unique<TypeInfo>(v.functionIdent->type->getReturnedType());
        processType(*v.type);
    }
    VISIT_FUNCTION(BinOpExprAST) {
        if (v.op == "="){
            callAccept(v.rhs);
            callAccept(v.lhs);
        } else {
            callAccept(v.lhs);
            callAccept(v.rhs);
        }
        // only real binop allowed
        const static std::set<std::string> binOp{
            "+", "-", "*", "/", ">", "<", "==", "!=", ">=", "<=", "&&", "and", "||", "or", "%",
        };
        bool buildIn = false;
        if (*(v.lhs->type) == RealType && *(v.rhs->type) == RealType && binOp.contains(v.op)) {
            v.type = std::make_unique<TypeInfo>(RealType);
            buildIn = true;
        } else if (v.op == "=") {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
            // disable overload of "="
            buildIn = true;
            if (!isAssignable(v.lhs.get())) {
                return setError("Assign to non-assignable expression");
            }
            if (*(v.lhs->type) != *(v.rhs->type) || *(v.lhs->type) == NoInstanceType) {
                return setError(std::format("Assign between \"{}\" and \"{}\" not allowed", v.lhs->type->toString(),
                                            v.rhs->type->toString()));
            }
        }

        auto varType = std::vector<TypeInfo>{*(v.lhs->type), *(v.rhs->type)};
        if (auto [find, realName] = seekSymbolicFunc(v.op, varType); find) {
            if (buildIn) {
                return setError(std::format("Cannot overload operator \"{}\" bwtween \"{}\" and \"{}\"", v.op,
                                            v.lhs->type->toString(), v.rhs->type->toString()));
            }
            std::vector<std::unique_ptr<ExprAST>> tmp;
            tmp.push_back(std::move(v.lhs));
            tmp.push_back(std::move(v.rhs));
            auto &funcType = c.getRealFunctionType(realName);
            needChange = std::make_unique<FunctionCallExprAST>(
                std::make_unique<TypeInfo>(funcType.getReturnedType()),
                std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(funcType), realName), std::move(tmp));
            processType(*needChange->type);
            return;
        }
        if (!buildIn) {
            return setError(std::format("Operator \"{}\" between \"{}\" and \"{}\" not defined", v.op,
                                        v.lhs->type->toString(), v.rhs->type->toString()));
        }
        processType(*v.type);
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        callAccept(v.rhs);
        const static std::set<std::string> unaryOp{
            "-",
            "!",
            "not",
        };
        bool buildIn = false;
        if (*(v.rhs->type) == RealType && unaryOp.contains(v.op)) {
            v.type = std::make_unique<TypeInfo>(RealType);
            buildIn = true;
        }
        // else if (v.op == "&") {
        //     v.type = std::make_unique<TypeInfo>(v.rhs->type->getPointerType());
        //     buildIn = true;
        // }
        if (auto it = globalInfo().symbolicFuncDef.find(v.op); it != globalInfo().symbolicFuncDef.end()) {
            if (auto it2 = it->second.find({*(v.rhs->type)}); it2 != it->second.end()) {
                if (buildIn) {
                    return setError(
                        std::format("Cannot overload unary operator \"{}\" to \"{}\"", v.op, v.rhs->type->toString()));
                }
                auto realName = it2->second;
                funcDependencyRealName.insert(realName);
                auto &type = c.getRealFunctionType(realName);
                auto param = std::vector<std::unique_ptr<ExprAST>>{};
                param.push_back(std::move(v.rhs));
                needChange = std::make_unique<FunctionCallExprAST>(
                    std::make_unique<TypeInfo>(type.getReturnedType()),
                    std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(type), realName), std::move(param));
                processType(*needChange->type);
                return;
            }
        }
        if (!buildIn) {
            return setError(std::format("Operator \"{}\" to \"{}\" not defined", v.op, v.rhs->type->toString()));
        }
        processType(*v.type);
    }
    VISIT_FUNCTION(BranchExprAST) {
        callAccept(v.condition);
        if (*(v.condition->type) != RealType || *(v.condition->type) != IntType) {
            return setError("Branch condition must be bool");
        }
        callAccept(v.trueExpr);
        callAccept(v.falseExpr);
        if (*(v.trueExpr->type) != *(v.falseExpr->type)) {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
        } else {
            v.type = std::make_unique<TypeInfo>(*(v.trueExpr->type));
        }
        processType(*v.type);
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        processType(*v.type);
        if (v.type->isBaseType()) {
            if (BuildInType.contains(*v.type)) {
                if (*v.type == NoInstanceType) {
                    return setError("Cannot create a NoInstanceType");
                }
                if (*v.type == RealType || *v.type == IntType || *v.type == StringType) {
                    if (v.members.size() == 1) {
                        auto &[ind, val] = v.members[0];
                        if (ind) {
                            return setError("Cannot create a \"" + v.type->toString() + "\" with index");
                        }
                        callAccept(val);
                        if (*val->type == *v.type) {
                            needChange = std::move(val);
                            return;
                        }
                        return setError("Cannot create a \"" + v.type->toString() + "\" with a \"" +
                                        val->type->toString() + "\"");
                    } else if (v.members.size() > 1) {
                        return setError("Cannot create a \"" + v.type->toString() + "\" with more than one member");
                    }
                    std::string value = (*v.type == StringType) ? "" : "0";
                    needChange = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(*v.type), value);
                }
                return setError("Unknown base type \"" + v.type->toString() + "\"");
            }
            auto it = globalInfo().typeDef.find(v.type->getBaseTypeString());
            if (it == globalInfo().typeDef.end()) {
                return setError(std::format("Type \"{}\" not defined", v.type->toString()));
            }
            auto def = it->second;
            if (!def.isComplexType()) {
                return setError(std::format("Defined type \"{}\" required to be a complex type", v.type->toString()));
            }
            if (v.members.size() == 0) {
                return;
            }
            bool designate = std::get<0>(v.members[0]) != nullptr;
            if (!designate) {
                if (v.members.size() != def.getMemberCount()) {
                    return setError(std::format("Type \"{}\" need {} members, {} given", v.type->toString(),
                                                def.getMemberCount(), v.members.size()));
                }
                size_t cnt = 0;
                for (auto &&[index, _] : v.members) {
                    index = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(StringType),
                                                             def.getMemberName(cnt++));
                }
            }
            size_t cnt = 0;
            for (auto &&[ident, member] : v.members) {
                callAccept(member);
                callAccept(ident);
                auto p = dynamic_cast<LiteralExprAST *>(ident.get());
                if (!p || *(p->type) != StringType) {
                    return setError("Non literal designate do not supported");
                }
                if (def.getMemberType(p->value) != *(member->type)) {
                    return setError(std::format("Designate type mismatch, \"{}\" expected, \"{}\" given",
                                                def.getMemberType(p->value).toString(), member->type->toString()));
                }
            }
        } else if (v.type->isArrayType()) {
            auto def = v.type->getElementType();
            if (v.members.size() == 0) {
                return;
            }
            bool designate = std::get<0>(v.members[0]) != nullptr;
            if (designate) {
                return setError("Array can't be designated");
            }
            for (auto &&[ident, member] : v.members) {
                callAccept(member);
                if (*(member->type) != def) {
                    return setError(std::format("Array element type mismatch, \"{}\" expected, \"{}\" given",
                                                def.toString(), member->type->toString()));
                }
            }
        } else {
            return setError(std::format("literal type of \"{}\" not supported", v.type->toString()));
        }
    }
    VISIT_FUNCTION(LoopAST) {
        c.push();
        callAccept(v.init);
        callAccept(v.condition);
        if (*(v.condition->type) != RealType || *(v.condition->type) != IntType) {
            return setError("Loop condition must be bool");
        }
        callAccept(v.body);
        if (*(v.init->type) != *(v.body->type)) {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
        } else {
            v.type = std::make_unique<TypeInfo>(*(v.body->type));
        }
        c.pop();
        processType(*v.type);
    }
    VISIT_FUNCTION(BlockExprAST) {
        c.push();
        TypeInfo *last = nullptr;
        for (auto &stmt : v.exprs) {
            callAccept(stmt);
            last = stmt->type.get();
        }
        if (last != nullptr) {
            v.type = std::make_unique<TypeInfo>(*last);
        } else {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
        }
        c.pop();
        processType(*v.type);
    }
    VISIT_FUNCTION(ControlFlowAST) {
        // type define only allowed top-level
        processType(*v.type);
        return setError("ControlFlowAST not supported");
    }
    VISIT_FUNCTION(TypeDefAST) {
        // disable type alias
        if (c.size() != 1) {
            return setError("Only allow top-level typedef");
        }
        if (v.typeDefType != TypeDefAST::TypeDefType::NORMAL) {
            return setError("Only support normal type def for now");
        }
        if (!v.definedType->isComplexType() || v.definedType->getIdent() != "struct") {
            return setError("only allow struct type define");
        }
        if (auto type = TypeInfo(v.name); BuildInType.contains(type)) {
            return setError(std::format("Type \"{}\" is a build-in type", v.name));
        }
        for (auto &&t : v.definedType->getSubTypes()) {
            if (t.isComplexType()) {
                return setError(std::format("unnamed type \"{}\" is not allowed", t.toString()));
            }
            processType(t);
            if (t != RealType && t != IntType && t.isBaseType() && !globalInfo().typeDef.contains(t.toString())) {
                // TODO: process each type
                // TODO: array member/func member/pointer member?
                return setError(std::format("type \"{}\" is not defined", t.toString()));
            }
        }
        if (globalInfo().typeDef.contains(v.name)) {
            return setError(std::format("Type \"{}\" redefined", v.name));
        }
        globalInfo().typeDef.emplace(v.name, *(v.definedType));
        needChange = nop();
        return;
    }
    VISIT_FUNCTION(VarDefAST) {
        callAccept(v.definedValue);
        if (*(v.definedValue->type) == NoInstanceType) {
            return setError("Var defined value has no return");
        }
        if (*(v.definedValue->type) != *(v.valueType) && *(v.valueType) != AutoType) {
            return setError("Var def type mismatch");
        } else if (*(v.valueType) == AutoType) {
            v.valueType = std::make_unique<TypeInfo>(*(v.definedValue->type));
        }
        switch (v.varDefType) {
        case VarDefAST::VarDefType::NORMAL:
            if (!c.addVarDef(v.name, *(v.valueType))) {
                return setError(std::format("Var \"{}\" already defined", v.name));
            }
            if (c.size() == 1) {
                needChange = std::make_unique<BinOpExprAST>(
                    std::make_unique<TypeInfo>(NoInstanceType), "=",
                    std::make_unique<IdentifierExprAST>(std::move(v.valueType), v.name), std::move(v.definedValue));
                processType(*needChange->type);
            }
            break;
        case VarDefAST::VarDefType::CONSTANT:
            if (auto p = dynamic_cast<LiteralExprAST *>(v.definedValue.get()); p == nullptr) {
                return setError("Const var must be defined as literal");
            } else if (!c.addConstDef(v.name, *(v.valueType), p->value)) {
                return setError(std::format("Const \"{}\" already defined", v.name));
            }
            needChange = nop();
            break;
        }
    }
    VISIT_FUNCTION(FunctionDefAST) {
        // func define only allowed top-level
        if (c.size() != 1) {
            // TODO: named lambda
            return setError("Only allow top-level func def");
        }
        std::string realFuncName = c.generateUniqueName(reservedPrefix, v.name + v.funcType->toString());
        if (v.funcDefType == FunctionDefAST::FuncDefType::SYMBOLIC) {
            if (v.params.size() != 2 && (v.params.size() != 1 || !BUILDIN_UNARY.contains(v.name))) {
                return setError("Infix function must have 2 params, "
                                "and operator overload must match the number of params");
            }
            std::vector<rulejit::TypeInfo> tmp;
            for (auto &&p : v.params) {
                tmp.emplace_back(*(p->type));
            }
            if (auto it = globalInfo().symbolicFuncDef.find(v.name); it != globalInfo().symbolicFuncDef.end()) {
                if (it->second.contains(tmp)) {
                    return setError(
                        std::format("Function redefined: \"{}\" with type \"{}\"", v.name, v.funcType->toString()));
                }
            }
            globalInfo().symbolicFuncDef[v.name].emplace(std::move(tmp), realFuncName);
        } else if (v.funcDefType == FunctionDefAST::FuncDefType::MEMBER) {
            std::vector<rulejit::TypeInfo> tmp;
            for (auto &&p : v.params) {
                tmp.emplace_back(*(p->type));
            }
            if (auto it = globalInfo().memberFuncDef.find(v.name); it != globalInfo().memberFuncDef.end()) {
                if (it->second.contains(tmp)) {
                    return setError(std::format("Member function redefined: \"{}\" with type \"{}\"", v.name,
                                                v.funcType->toString()));
                }
                it->second.emplace(std::move(tmp), realFuncName);
            } else {
                globalInfo().memberFuncDef.emplace(
                    v.name, std::map<std::vector<TypeInfo>, std::string>{{std::move(tmp), realFuncName}});
            }
        } else if (v.funcDefType == FunctionDefAST::FuncDefType::NORMAL) {
            if (!c.addConstDef(v.name, *(v.funcType), realFuncName)) {
                return setError(std::format("Function redefined: \"{}\"", v.name));
            }
            globalInfo().funcDef.emplace(v.name, realFuncName);
        } else {
            // TODO: lambda/named lambda
            return setError("only allow symbolic/member/normal function define");
        }
        globalInfo().realFuncDefinition.emplace(
            realFuncName, std::make_unique<FunctionDefAST>(std::move(v.name), std::move(v.funcType), std::move(v.params),
                                                          std::move(v.returnValue), v.funcDefType));
        needChange = nop();
    }
    VISIT_FUNCTION(SymbolDefAST) {
        if (c.size() != 1 || v.symbolCommandType != SymbolDefAST::SymbolCommandType::EXTERN) {
            return setError("Only allow top-level, extern symbol define");
        }
        if (!c.addConstDef(v.name, *(v.definedType), v.name)) {
            return setError(std::format("Extern func name \"{}\" already defined", v.name));
        }
        globalInfo().funcDef.emplace(v.name, v.name);
        globalInfo().externFuncDef.emplace(v.name, *(v.definedType));
        needChange = nop();
    }
    VISIT_FUNCTION(TemplateDefAST) {
        if (c.size() != 1) {
            // TODO: named lambda
            return setError("Only allow top-level func def");
        }
        auto p = dynamic_cast<FunctionDefAST *>(v.def.get());
        if (!p) {
            return setError("Template type/var not supported");
        }
        auto name = p->name;
        if (!c.isSymbolUnique(name)) {
            return setError(std::format("Template function \"{}\" redefined", name));
        }
        switch (p->funcDefType) {
        case FunctionDefAST::FuncDefType::NORMAL:
            globalInfo().templateFuncDef.emplace(
                name,
                ContextGlobal::TemplateFunctionInfo{{}, std::move(v.tparams), unique_cast<FunctionDefAST>(v.def)});
            break;
        case FunctionDefAST::FuncDefType::MEMBER:
            globalInfo().templateMemberFuncDef[name].push_back(
                {{}, std::move(v.tparams), unique_cast<FunctionDefAST>(v.def)});
            break;
        case FunctionDefAST::FuncDefType::SYMBOLIC:
            globalInfo().templateSymbolicFuncDef[name].push_back(
                {{}, std::move(v.tparams), unique_cast<FunctionDefAST>(v.def)});
            break;
        default:
            setError("Template not supported");
        }
        needChange = nop();
    }
    VISIT_FUNCTION(ClosureExprAST) {
        c.push();
        for (auto &&p : v.params) {
            my_assert(c.addVarDef(p->name, *(p->type)));
        }
        // any function that the closure called will become a dependency of the function where the closure is defined
        callAccept(v.returnValue);
        if (v.explicitCapture) {
            setError("Do not support explicit capture yet");
            // TODO: check type of captured variable
            for (auto &&p : v.captures) {
                if (!c.addVarDef(p->name, *(p->type))) {
                    return setError(std::format("Captured variable \"{}\" has same name as parameter", p->name));
                }
            }
        } else {
            // constant will be automatically changed to literal
            auto vars = v.returnValue | EscapedVarAnalyzer{};
            for (auto &name : vars) {
                if (!c.top().varDef.contains(name) && !c.scope[0].varDef.contains(name)) {
                    // captured var is not a parameter nor global variable
                    setError("Do not support capture local variable by reference yet");
                }
            }
        }
        if (*v.type == AutoType) {
            v.type = std::make_unique<TypeInfo>("func");
            for (auto &&param : v.params) {
                v.type->addParamType(*(param->type));
            }
            v.type->addParamType(*(v.returnValue->type));
        } else {
            if (v.type->getReturnedType() != *(v.returnValue->type)) {
                return setError("Return type mismatch, expected: " + v.type->getReturnedType().toString() +
                                ", got: " + v.returnValue->type->toString());
            }
        }
        auto name = c.generateUniqueName(reservedPrefix, "lambda" + v.type->toString());
        // lambda do not dependent on any function, it is directly checked when constructed
        globalInfo().funcDependency.emplace(name, std::set<std::string>{});
        funcDependencyRealName.emplace(name);
        auto funcDefAST =
            std::make_unique<FunctionDefAST>("", std::make_unique<TypeInfo>(*v.type), std::move(v.params), std::move(v.returnValue));
        funcDefAST->captures = std::move(v.captures);
        globalInfo().realFuncDefinition.emplace(name, std::move(funcDefAST));
        globalInfo().checkedFunc.emplace(name);
        // TODO: add to real func
        c.pop();
        needChange = std::make_unique<LiteralExprAST>(std::move(v.type), name);
    }

  private:
    void callAccept(std::unique_ptr<ExprAST> &tar) {
        callStack.push_back(tar.get());
        tar->accept(this);
        if (needChange) {
            tar = std::move(needChange);
            needChange = nullptr;
        }
        callStack.pop_back();
    };

    std::tuple<MemberAccessExprAST *, LiteralExprAST *> getConstStringMemberAccessInfo(ExprAST *v) {
        if (auto p = dynamic_cast<MemberAccessExprAST *>(v); p) {
            if (auto p1 = dynamic_cast<LiteralExprAST *>(p->memberToken.get()); p1 && *(p1->type) == StringType) {
                return {p, p1};
            }
        }
        return {nullptr, nullptr};
    }
    std::string addUnnamedFunction(std::vector<std::unique_ptr<ExprAST>> topLevelExpr) {
        init();
        std::unique_ptr<ExprAST> tmp;
        for (auto it = topLevelExpr.begin(); it != topLevelExpr.end();) {
            callAccept(*it);
            if (it != topLevelExpr.end() && it + 1 != topLevelExpr.end() && isType<LiteralExprAST>(*it) &&
                *(isType<LiteralExprAST>(*it)->type) == NoInstanceType) {
                it = topLevelExpr.erase(it);
            } else {
                ++it;
            }
        }
        if (topLevelExpr.size() == 0) {
            // TODO: merge all empty func;
            tmp = nop();
        } else if (topLevelExpr.size() == 1) {
            tmp = std::move(topLevelExpr[0]);
        } else {
            tmp = std::make_unique<BlockExprAST>(std::make_unique<TypeInfo>(*(topLevelExpr.back()->type)),
                                                 std::move(topLevelExpr));
        }
        auto type = std::make_unique<TypeInfo>("func");
        if (*(tmp->type) != NoInstanceType) {
            type->addParamType(*(tmp->type));
        } else {
            type->addParamType(NoInstanceType);
        }
        auto name = c.generateUniqueName(reservedPrefix, "unnamedfunc()");
        globalInfo().funcDependency.emplace(name, std::move(funcDependencyRealName));
        funcDependencyRealName.clear();
        std::vector<std::unique_ptr<IdentifierExprAST>> params;
        globalInfo().realFuncDefinition.emplace(
            name, std::make_unique<FunctionDefAST>(name, std::move(type), std::move(params), std::move(tmp)));
        // only check func when called, assume top-level expr is directly called, so check it
        checkFunction(name);
        return name;
    }
    void init() {
        my_assert(!needChange);
        funcDependencyRealName.clear();
        callStack.clear();
        while (c.size() != 1) {
            c.pop();
        }
    }
    
    rulejit::ContextGlobal &globalInfo() { return c.global; }

    SET_ERROR_MEMBER("Semantic Check", void)

    void processType(const TypeInfo &type) {
        if (!type.isValid() || type == NoInstanceType) {
            return;
        }
        if (type.isBaseType()) {
            if (!globalInfo().typeDef.contains(type.getBaseTypeString()) && !BuildInType.contains(type)) {
                return setError(std::format("type \"{}\" is not defined", type.toString()));
            }
        } else if (type.isPointerType()) {
            return setError(std::format("pointer type \"{}\" is not support for now", type.toString()));
        } else if (type.isComplexType()) {
            return setError(std::format("unnamed type \"{}\" is not allowed", type.toString()));
        } else if (type.isArrayType()) {
            return processType(type.getElementType());
        } else if (type.isFunctionType()) {
            for (auto &&sub : type.getSubTypes()) {
                processType(sub);
            }
        } else {
            return setError(std::format("type \"{}\" is not defined", type.toString()));
        }
    }

    /**
     * @brief check one real function and return all dependency of it.
     *
     * @details name in checked means the function itself is checked, and its dependencies are waiting for check
     *
     * @param name real function name need to check
     * @param checked temporary set to avoid infinite loop
     * @return const std::set<std::string>& to avoid unexpected copy
     */
    const std::set<std::string> &checkSingleRealFunction(const std::string &name, std::set<std::string> &checked) {
        if (globalInfo().externFuncDef.contains(name)) {
            // do not check extern func
            static const std::set<std::string> empty;
            return empty;
        }
        my_assert(c.size() == 1);
        if (globalInfo().checkedFunc.contains(name) || checked.contains(name)) {
            auto it = globalInfo().funcDependency.find(name);
            my_assert(it != globalInfo().funcDependency.end());
            return it->second;
        }
        auto &func = globalInfo().realFuncDefinition.find(name)->second;
        my_assert(funcDependencyRealName.empty());
        c.push();
        for (auto &&param : func->params) {
            // TODO: var name conflit with func?
            processType(*(param->type));
            c.top().varDef.emplace(param->name, *(param->type));
        }
        callAccept(func->returnValue);
        auto &returnedType = func->funcType->getReturnedType();
        if (*(func->returnValue->type) != returnedType) {
            setError(std::format("function \"{}\" declared return \"{}\", but return \"{}\" actually", func->name,
                                 returnedType.toString(), func->returnValue->type->toString()));
        }
        c.pop();
        if (auto it = globalInfo().funcDependency.find(name); it != globalInfo().funcDependency.end()) {
            my_assert(funcDependencyRealName == it->second);
        }
        globalInfo().funcDependency.emplace(name, std::move(funcDependencyRealName));
        funcDependencyRealName.clear();
        checked.insert(name);
        return globalInfo().funcDependency[name];
    }

    /**
     * @brief check set of dependency real function name generated by ExpressionSemantic::checkSingleRealFunction
     *
     * @details name in checked means the function itself is checked, and its dependencies are waiting for check
     *
     * @param needCheck set of dependency real function name
     * @param checked temporary set to avoid infinite loop
     */
    void checkRealFunctionDependencySet(std::set<std::string> &needCheck, std::set<std::string> &checked) {
        while (!needCheck.empty()) {
            auto name = *needCheck.begin();
            needCheck.erase(needCheck.begin());
            for (auto &&name : checkSingleRealFunction(name, checked)) {
                if (!checked.contains(name) && !globalInfo().checkedFunc.contains(name)) {
                    needCheck.insert(name);
                }
            }
        }
        globalInfo().checkedFunc.merge(checked);
    }

    /**
     * @brief check if member access is valid
     *
     * @param v AST of member access
     * @return bool
     */
    bool canAccess(MemberAccessExprAST &v) {
        if (isType<LiteralExprAST>(v.memberToken.get())) {
            auto p = dynamic_cast<LiteralExprAST *>(v.memberToken.get());
            if (*(p->type) == StringType && v.baseVar->type->isBaseType()) {
                auto definedType = globalInfo().typeDef.find(v.baseVar->type->getBaseTypeString());
                if (definedType == globalInfo().typeDef.end()) {
                    return false;
                }
                return definedType->second.hasMember(p->value);
            }
        }
        return (v.baseVar->type->isArrayType() &&
                (*(v.memberToken->type) == IntType || *(v.memberToken->type) == RealType));
    }

    /**
     * @brief check if expression a assignable l-value
     *
     * @attention must call canAccess before call to this function, so that const var
     * already transfered to literal
     *
     * @param v expression to check
     * @return true
     * @return false
     */
    bool isAssignable(ExprAST *v) {
        if (auto p = dynamic_cast<IdentifierExprAST *>(v); p) {
            return true;
        }
        if (auto p = dynamic_cast<UnaryOpExprAST *>(v); p) {
            if (p->op == "*") {
                return true;
            } else {
                return false;
            }
        }
        if (auto p = dynamic_cast<MemberAccessExprAST *>(v); p) {
            // must not be member function
            my_assert(canAccess(*p));
            return isAssignable(p->baseVar.get());
        }
        return false;
    }

    /**
     * @brief seek member function with given name and param type
     *
     * @attention this function will automatically instantiate template,
     * and add real function name to funcDependencyRealName
     *
     * @param name name of member function
     * @param paramType parameter types
     * @return std::tuple<bool, std::string> (is_find, realname)
     */
    std::tuple<bool, std::string> seekMemberFunc(const std::string &name, const std::vector<TypeInfo> &paramType) {
        return seekReloadableFunc(name, paramType, &ContextGlobal::memberFuncDef,
                                  &ContextGlobal::templateMemberFuncDef);
    }

    /**
     * @brief seeksymbolic function with given name and param type
     *
     * @attention this function will automatically instantiate template,
     * and add real function name to funcDependencyRealName
     *
     * @param name name of symbolic function
     * @param paramType parameter types
     * @return std::tuple<bool, std::string> (is_find, realname)
     */
    std::tuple<bool, std::string> seekSymbolicFunc(const std::string &name, const std::vector<TypeInfo> &paramType) {
        return seekReloadableFunc(name, paramType, &ContextGlobal::symbolicFuncDef,
                                  &ContextGlobal::templateSymbolicFuncDef);
    }

    /**
     * @brief inner function for seek reloadable function
     *
     * @tparam _Original storage of explicit defined function
     * @tparam _Template storage of template function
     * @param name function name
     * @param paramType parameter types
     * @param o storage of explicit defined function
     * @param t storage of template function
     * @return std::tuple<bool, std::string> (is_find, realname)
     */
    template <typename _Original, typename _Template>
    std::tuple<bool, std::string> seekReloadableFunc(const std::string &name, const std::vector<TypeInfo> &paramType,
                                                     _Original &&o, _Template &&t) {
        if (auto funcIt = (globalInfo().*o).find(name); funcIt != (globalInfo().*o).end()) {
            auto &tmp = funcIt->second;
            if (auto realFuncNameIt = tmp.find(paramType); realFuncNameIt != tmp.end()) {
                funcDependencyRealName.insert(realFuncNameIt->second);
                return {true, realFuncNameIt->second};
            }
        }
        // no direct define of func, try template
        if (auto templateFuncIt = (globalInfo().*t).find(name); templateFuncIt != (globalInfo().*t).end()) {
            for (auto &tmp : templateFuncIt->second) {
                if (auto realFuncNameIt = tmp.instantiationRealName.find(paramType);
                    realFuncNameIt != tmp.instantiationRealName.end()) {
                    // already instantiated
                    funcDependencyRealName.insert(realFuncNameIt->second);
                    return {true, realFuncNameIt->second};
                }
            }
            // not instantiated
            std::unordered_map<std::string, std::unique_ptr<FunctionDefAST>> alreadyMatch;
            ContextGlobal::TemplateFunctionInfo *from = nullptr;
            for (auto &tmp : templateFuncIt->second) {
                auto [instantiated, matched] = tmp.instantiate(paramType);
                if (!instantiated) {
                    continue;
                }
                from = &tmp;
                auto templateParamList =
                    tmp.paramNames |
                    std::views::transform([&](const std::string &param) { return matched[param].toString(); }) |
                    mystr::join(",");
                auto name = std::format("<{}>{}", templateParamList, instantiated->name);
                alreadyMatch[name] = unique_cast<FunctionDefAST>(instantiated);
            }
            if (alreadyMatch.empty()) {
                return {false, ""};
            } else if (alreadyMatch.size() != 1) {
                setError("ambiguous template instantiation: " + (std::views::keys(alreadyMatch) | mystr::join(", ")));
            }
            // 1 match, add to realfuncdef
            auto [matchName, funcAST] = std::move(*alreadyMatch.begin());
            auto realName = c.generateUniqueName(reservedPrefix, matchName + funcAST->funcType->toString());
            globalInfo().realFuncDefinition.emplace(realName, std::move(funcAST));
            funcDependencyRealName.emplace(realName);
            from->instantiationRealName.emplace(paramType, realName);
            return {true, realName};
        }
        return {false, ""};
    }

    ContextStack &c;

    // temp variable need trans through function
    std::vector<ExprAST *> callStack;
    std::unique_ptr<ExprAST> needChange;
    std::set<std::string> funcDependencyRealName;
};

} // namespace rulejit