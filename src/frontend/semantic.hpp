#pragma once

#include <format>
#include <map>
#include <memory>
#include <ranges>
#include <source_location>
#include <string>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/context.hpp"
#include "defines/language.hpp"
#include "tools/myassert.hpp"

#define __RULEJIT_SEMANTIC_PUSH pushStack(v.get());
#define __RULEJIT_SEMANTIC_POP popStack();

namespace rulejit {

// 1. type inference
// 2. TODO: name unnamed complex type(in vardef, typedef, literal and funcdef)
// 3. scope process
// 4. capture analysis
// 5. redefined symbol name(var, type and func name cannot be same)
struct ExpressionSemantic : public ASTVisitor {
    ExpressionSemantic() = default;
    // ContextStack cannot be destructed before last process call of this
    void loadContext(ContextStack *context) { c = context; }
    std::vector<std::unique_ptr<ExprAST>> friend operator|(std::vector<std::unique_ptr<ExprAST>> ast,
                                                           ExpressionSemantic &t) {
        for (auto &i : ast) {
            if (i && isType<TypeDefAST>(i.get())) {
                i->accept(&t);
                t.afterAccept(i);
                i = nullptr;
            }
        }
        for (auto &i : ast) {
            if (i && isType<FunctionDefAST>(i.get())) {
                auto v = dynamic_cast<FunctionDefAST *>(i.get());
                if (t.c->size() != 1)
                    throw std::logic_error("no top-level function def not supported");
                std::string realFuncName = t.c->generateUniqueName("func", t.toLegalName(v->name));
                if (v->funcDefType == FunctionDefAST::FuncDefType::SYMBOLIC) {
                    my_assert(v->params.size() == 2);
                    t.globalInfo().symbolicFuncDef[v->name].emplace(
                        std::tuple<rulejit::TypeInfo, rulejit::TypeInfo>{*(v->params[0]->type), *(v->params[1]->type)},
                        realFuncName);
                    t.globalInfo().realFuncDefinition[realFuncName] =
                        std::unique_ptr<rulejit::FunctionDefAST>(dynamic_cast<FunctionDefAST *>(i.release()));
                } else if (v->funcDefType == FunctionDefAST::FuncDefType::MEMBER) {
                    throw std::logic_error("member function def not supported");
                } else {
                    auto newExpr =
                        std::unique_ptr<rulejit::FunctionDefAST>(dynamic_cast<FunctionDefAST *>(i.release()));
                    i = std::make_unique<VarDefAST>(
                        newExpr->name, std::make_unique<TypeInfo>(*(newExpr->funcType)),
                        std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(*(newExpr->funcType)),
                                                         realFuncName));
                    t.globalInfo().realFuncDefinition[realFuncName] = std::move(newExpr);
                }
            } else if (i && isType<SymbolDefAST>(i.get())) {
                auto v = dynamic_cast<SymbolDefAST *>(i.get());
                if (v->symbolCommandType == SymbolDefAST::SymbolCommandType::EXTERN) {
                    t.globalInfo().externFuncDef[v->name] = *(v->definedType);
                    i = std::make_unique<VarDefAST>(
                        v->name, std::make_unique<TypeInfo>(*(v->definedType)),
                        std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(*(v->definedType)), v->name));
                } else {
                    throw std::logic_error("only support extern func command");
                }
            }
        }
        for (auto &i : ast) {
            if (i) {
                i->accept(&t);
                t.afterAccept(i);
            }
        }
        for (auto &[name, func] : t.globalInfo().realFuncDefinition) {
            t.c->push();
            for (auto &&param : func->params) {
                t.c->top().varDef[param->name] = *(param->type);
            }
            func->returnValue->accept(&t);
            t.afterAccept(func->returnValue);
            if (*(func->returnValue->type) != func->funcType->getReturnedType()) {
                throw std::logic_error(std::format("function \"{}\" declared return \"{}\", but return \"{}\" actually",
                                                   func->name, func->funcType->getReturnedType().toString(),
                                                   func->returnValue->type->toString()));
            }
            t.c->pop();
        }
        std::erase_if(ast, [](auto &i) { return i == nullptr; });
        return std::move(ast);
    }
    std::unique_ptr<ExprAST> friend operator|(std::unique_ptr<ExprAST> i, ExpressionSemantic &t) {
        // TODO: extend
        // t.callStack.clear();
        t.callParam = nullptr;
        t.needChange = nullptr;
        t.needCheckFunc.clear();

        i->accept(&t);
        t.afterAccept(i);
        while (!t.needCheckFunc.empty()) {
            auto name = t.needCheckFunc.begin().operator*();
            if (auto it = t.globalInfo().UncheckedFunc.find(name); it != t.globalInfo().UncheckedFunc.end()) {
                t.checkRealFunction(*it);
                t.globalInfo().UncheckedFunc.erase(it);
            }
            t.needCheckFunc.erase(name);
        }
        return std::move(i);
    }
    VISIT_FUNCTION(IdentifierExprAST) {
        auto [find, type] = c->seekVarDef(v.name);
        if (auto it = globalInfo().funcDef.find(v.name); it != globalInfo().funcDef.end()) {
            // normal func
            auto realName = it->second;
            // TODO: only check if called?(but how to check called through variable?)
            needCheckFunc.insert(realName);
            needChange = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(c->getRealFunctionType(realName)),
                                                          realName);
        } else if (auto it = globalInfo().externFuncDef.find(v.name); it != globalInfo().externFuncDef.end()) {
            // extern func
            needChange = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(it->second), v.name);
        } else if (callParam) {
            // cannot act as func, only allow direct call
            // member func TODO: member function to pointer
            if (auto it = globalInfo().memberFuncDef.find(v.name); it != globalInfo().memberFuncDef.end()) {
                // std::vector<rulejit::TypeInfo> paramType =
                //     (*callParam) | std::views::all |
                //     std::views::transform([](std::unique_ptr<rulejit::ExprAST> &i) { return *(i->type); }) |
                //     std::ranges::to<std::vector>();
                std::vector<rulejit::TypeInfo> paramType;
                for (auto &&i : *callParam) {
                    paramType.emplace_back(*(i->type));
                }
                if (auto it2 = it->second.find(paramType); it2 != it->second.end()) {
                    auto realName = it2->second;
                    needCheckFunc.insert(realName);
                    auto &type = c->getRealFunctionType(realName);
                    needChange = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(type), realName);
                    return;
                }
            }
        }
        if (find) {
            if (needChange) {
                needChange = nullptr;
                return setError(std::format("Variable \"{}\" has same name with function", v.name));
            }
            // normal var
            v.type = std::make_unique<TypeInfo>(type);
        } else {
            return setError(std::format("Variable \"{}\" not defined", v.name));
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        v.baseVar->accept(this);
        afterAccept(v.baseVar);
        v.memberToken->accept(this);
        afterAccept(v.memberToken);
        if (isType<LiteralExprAST>(v.memberToken.get())) {
            auto p = dynamic_cast<LiteralExprAST *>(v.memberToken.get());
            if (*(p->type) == StringType && v.baseVar->type->isBaseType()) {
                // struct member access; TODO: member function, access overload
                auto definedType = globalInfo().typeDef.find(v.baseVar->type->getBaseType());
                if (definedType == globalInfo().typeDef.end()) {
                    return setError(std::format("Type \"{}\" not defined", v.baseVar->type->getBaseType()));
                }
                auto memberType = definedType->second.getMemberType(p->value);
                *(v.type) = memberType;
                return;
            }
        }
        if (v.baseVar->type->isArrayType()) {
            // array access
            if (*(v.memberToken->type) != RealType && *(v.memberToken->type) != IntType) {
                return setError(std::format("array index must be real or int type, but get \"{}\"",
                                            v.memberToken->type->toString()));
            }
            *(v.type) = v.baseVar->type->getElementType();
        } else {
            return setError(std::format("unknown member access {}[\"{}\"]", v.baseVar->type->toString(),
                                        v.memberToken->type->toString()));
        }
        processType(v.type);
    }
    VISIT_FUNCTION(LiteralExprAST) { my_assert(bool(v.type)); }
    VISIT_FUNCTION(FunctionCallExprAST) {
        for (auto &arg : v.params) {
            arg->accept(this);
            afterAccept(arg);
        }
        callParam = &v.params;
        v.functionIdent->accept(this);
        afterAccept(v.functionIdent);
        callParam = nullptr;
        if (!v.functionIdent->type->isFunctionType()) {
            return setError(std::format("Cannot call to non-function type \"{}\"", v.functionIdent->type->toString()));
        }
        auto needParamNum =
            v.functionIdent->type->subTypes.size() - v.functionIdent->type->isReturnedFunctionType() ? 1 : 0;
        my_assert(needParamNum == v.params.size(),
                  std::format("Function need {} params, {} given", needParamNum, v.params.size()));
        for (size_t i = 0; i < v.params.size(); i++) {
            if (*(v.params[i]->type) != v.functionIdent->type->getArgType(i)) {
                return setError(std::format(
                    "Arg number {} type mismatch in call to function type \"{}\", \"{}\" required, \"{}\" given", i,
                    v.functionIdent->type->toString(), v.params[i]->type->toString(),
                    v.functionIdent->type->getArgType(i).toString()));
            }
        }
        v.type = std::make_unique<TypeInfo>(v.functionIdent->type->getReturnedType());
        processType(v.type);
    }
    VISIT_FUNCTION(BinOpExprAST) {
        v.lhs->accept(this);
        afterAccept(v.lhs);
        v.rhs->accept(this);
        afterAccept(v.rhs);
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
            // TODO: disable overload of "="
            buildIn = true;
            if (*(v.lhs->type) != *(v.rhs->type) || *(v.lhs->type) == NoInstanceType) {
                return setError(std::format("Assign between \"{}\" and \"{}\" not allowed", v.lhs->type->toString(),
                                            v.rhs->type->toString()));
            }
        }
        if (auto overLoad = globalInfo().symbolicFuncDef.find(v.op); overLoad != globalInfo().symbolicFuncDef.end()) {
            if (auto it = overLoad->second.find({*(v.lhs->type), *(v.rhs->type)}); it != overLoad->second.end()) {
                if (buildIn) {
                    return setError(std::format("Cannot overload operator \"{}\" bwtween \"{}\" and \"{}\"", v.op,
                                                v.lhs->type->toString(), v.rhs->type->toString()));
                }
                std::vector<std::unique_ptr<ExprAST>> tmp;
                tmp.push_back(std::move(v.lhs));
                tmp.push_back(std::move(v.rhs));
                auto RealName = it->second;
                needCheckFunc.insert(RealName);
                auto &funcType = c->getRealFunctionType(RealName);
                needChange = std::make_unique<FunctionCallExprAST>(
                    std::make_unique<TypeInfo>(funcType.getReturnedType()),
                    std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(funcType), it->second), std::move(tmp));
                return;
            }
        }
        return setError(std::format("Operator \"{}\" between \"{}\" and \"{}\" not defined", v.op,
                                    v.lhs->type->toString(), v.rhs->type->toString()));
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        v.rhs->accept(this);
        afterAccept(v.rhs);
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
        if (auto it = globalInfo().symbolicFuncDef.find(v.op); it != globalInfo().symbolicFuncDef.end()) {
            if (auto it2 = it->second.find({*((*callParam)[0]->type)}); it2 != it->second.end()) {
                if (buildIn) {
                    return setError(
                        std::format("Cannot overload unary operator \"{}\" to \"{}\"", v.op, v.rhs->type->toString()));
                }
                auto realName = it2->second;
                needCheckFunc.insert(realName);
                auto &type = c->getRealFunctionType(realName);
                auto param = std::vector<std::unique_ptr<ExprAST>>{};
                param.push_back(std::move(v.rhs));
                needChange = std::make_unique<FunctionCallExprAST>(
                    std::make_unique<TypeInfo>(type.getReturnedType()),
                    std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(type), realName), std::move(param));
                return;
            }
        }
        return setError(std::format("Operator \"{}\" to \"{}\" not defined", v.op, v.rhs->type->toString()));
    }
    VISIT_FUNCTION(BranchExprAST) {
        v.condition->accept(this);
        afterAccept(v.condition);
        if (*(v.condition->type) != RealType || *(v.condition->type) != IntType) {
            return setError("Branch condition must be bool");
        }
        v.trueExpr->accept(this);
        afterAccept(v.trueExpr);
        v.falseExpr->accept(this);
        afterAccept(v.falseExpr);
        if (*(v.trueExpr->type) != *(v.falseExpr->type)) {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
        } else {
            v.type = std::make_unique<TypeInfo>(*(v.trueExpr->type));
        }
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        processType(v.type);
        if (v.type->isBaseType()) {
            auto it = globalInfo().typeDef.find(v.type->getBaseType());
            if (it == globalInfo().typeDef.end()) {
                return setError(std::format("Type \"{}\" not defined", v.type->toString()));
            }
            auto def = it->second;
            if (v.members.size() == 0) {
                return;
            }
            bool designate = std::get<0>(v.members[0]) != nullptr;
            if (!designate) {
                return setError("Complex type must be designate");
            }
            size_t cnt = 0;
            for (auto &&[ident, member] : v.members) {
                member->accept(this);
                afterAccept(member);
                ident->accept(this);
                afterAccept(ident);
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
                member->accept(this);
                afterAccept(member);
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
        c->push();
        v.init->accept(this);
        afterAccept(v.init);
        v.condition->accept(this);
        afterAccept(v.condition);
        if (*(v.condition->type) != RealType || *(v.condition->type) != IntType) {
            return setError("Loop condition must be bool");
        }
        v.body->accept(this);
        afterAccept(v.body);
        if (*(v.init->type) != *(v.body->type)) {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
        } else {
            v.type = std::make_unique<TypeInfo>(*(v.body->type));
        }
        c->pop();
    }
    VISIT_FUNCTION(BlockExprAST) {
        c->push();
        TypeInfo *last = nullptr;
        if (v.exprs.size() == 0) {
            return setError("BlockExprAST must have at least one expr");
        }
        for (auto &stmt : v.exprs) {
            stmt->accept(this);
            afterAccept(stmt);
            last = stmt->type.get();
        }
        v.type = std::make_unique<TypeInfo>(*last);
        c->pop();
    }

    VISIT_FUNCTION(ControlFlowAST) {
        // type define only allowed top-level
        return setError("ControlFlowAST not supported");
    }

    VISIT_FUNCTION(TypeDefAST) {
        // disable type alias
        if (c->size() != 1 || v.typeDefType != TypeDefAST::TypeDefType::NORMAL) {
            return setError("Only allow top-level typedef, and no type alias supported");
        }
        if (!v.definedType->isComplexType() || v.definedType->idents[0] != "struct") {
            return setError("only allow struct type define");
        }
        for (auto &&t : v.definedType->subTypes) {
            if (t.isComplexType()) {
                return setError(std::format("unnamed type \"{}\" is not allowed", t.toString()));
            }
            if (t != RealType && t != IntType && t.isBaseType() && !globalInfo().typeDef.contains(t.toString())) {
                return setError(std::format("type \"{}\" is not defined", t.toString()));
            }
        }
        globalInfo().typeDef[v.name] = *(v.definedType);
        return;
    }
    VISIT_FUNCTION(VarDefAST) {
        v.definedValue->accept(this);
        afterAccept(v.definedValue);
        if (*(v.definedValue->type) != *(v.valueType) && *(v.valueType) != AutoType) {
            return setError("Var def type mismatch");
        } else if (*(v.valueType) == AutoType) {
            v.valueType = std::make_unique<TypeInfo>(*(v.definedValue->type));
        }
        c->top().varDef[v.name] = *(v.valueType);
    }
    VISIT_FUNCTION(FunctionDefAST) {
        // func define only allowed top-level
        if (c->size() != 1) {
            return setError("Only allow top-level func def");
        }
        // how to avoid same name of func and var?
        // TODO: check copilot generated code
        std::string realFuncName = c->generateUniqueName("func", toLegalName(v.name));
        if (v.funcDefType == FunctionDefAST::FuncDefType::SYMBOLIC) {
            my_assert(v.params.size() == 2);
            needChange = nop();
            globalInfo().symbolicFuncDef[v.name].emplace(
                std::tuple<rulejit::TypeInfo, rulejit::TypeInfo>{*(v.params[0]->type), *(v.params[1]->type)},
                realFuncName);
            globalInfo().realFuncDefinition[realFuncName] = std::move(asType<FunctionDefAST>(std::move(i)));
        } else if (v->funcDefType == FunctionDefAST::FuncDefType::MEMBER) {
            needChange = nop();
            globalInfo().memberFuncDef[v.name].emplace(*(v.params[0]->type), realFuncName);
            globalInfo().realFuncDefinition[realFuncName] = std::move(asType<FunctionDefAST>(std::move(i)));
        } else if (v->funcDefType == FunctionDefAST::FuncDefType::NORMAL) {
            needChange = nop;
            globalInfo().realFuncDefinition[realFuncName] = std::move(asType<FunctionDefAST>(std::move(i)));
        } else {
            return setError("only allow symbolic/member/normal function define");
        }
    }
    VISIT_FUNCTION(SymbolDefAST) {
        if (c->size() != 1 || v.typeDefType != SymbolDefAST::SymbolCommandType::EXTERN) {
            return setError("Only allow top-level, extern symbol define");
        }
        t.globalInfo().externFuncDef[v.name] = *(v.definedType);
    }

  private:
    rulejit::ContextGlobal &globalInfo() { return c->global; }
    std::string toLegalName(const std::string &token) {
        std::string tmp;
        for (auto c : token) {
            if (isalpha(c) || c == '_') {
                tmp += c;
            } else {
                tmp += "_" + std::to_string((size_t)c);
            }
        }
        return tmp;
    }
    [[noreturn]] void setError(const std::string &info,
                               const std::source_location location = std::source_location::current()) {

        throw std::logic_error(std::format("Semantic Error{}: {}", location.line(), info));
        // return nullptr;
    }
    void afterAccept(std::unique_ptr<ExprAST> &p) {
        if (needChange) {
            p = std::move(needChange);
            needChange = nullptr;
        }
    }
    void processType(std::unique_ptr<TypeInfo> &type) {
        if (!type) {
            return;
        }
        if (type->isComplexType()) {
            return setError(std::format("unnamed type \"{}\" is not allowed", type->toString()));
        }
    }
    void checkRealFunction(const std::string &name) {
        my_assert(c->size() == 1);
        auto &func = globalInfo().realFuncDefinition.find(name)->second;
        c->push();
        for (auto &&param : func->params) {
            c->top().varDef[param->name] = *(param->type);
        }
        func->returnValue->accept(this);
        afterAccept(func->returnValue);
        if (*(func->returnValue->type) != func->funcType->getReturnedType()) {
            throw std::logic_error(std::format("function \"{}\" declared return \"{}\", but return \"{}\" actually",
                                               func->name, func->funcType->getReturnedType().toString(),
                                               func->returnValue->type->toString()));
        }
        c->pop();
    }
    // void pushStack(AST* v){callStack.push_back(v);}
    // void popStack(){callStack.pop_back();}
    ContextStack *c;

    // temp variable need trans through function
    std::unique_ptr<ExprAST> needChange;
    std::set<std::string> needCheckFunc;
    std::vector<std::unique_ptr<rulejit::ExprAST>> *callParam;
};

} // namespace rulejit