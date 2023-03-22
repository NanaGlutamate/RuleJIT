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
#include "tools/myassert.hpp"

#define __RULEJIT_SEMANTIC_PUSH pushStack(v.get());
#define __RULEJIT_SEMANTIC_POP popStack();

namespace rulejit {

// 1. type inference
// 2. name unnamed complex type(in vardef, typedef, literal and funcdef)
// 3. scope process
// 4. capture analysis
// 5. redefined symbol name(var, type and func name cannot be same)
struct ExpressionSemantic : public ASTVisitor {
    ExpressionSemantic() = default;
    std::vector<ExprAST *> callStack;
    // ContextStack cannot be destructed before last process call of this
    void loadContext(ContextStack &context) { c = &context; }
    // collect defs and check top-level expressions
    std::vector<std::unique_ptr<ExprAST>> friend operator|(std::vector<std::unique_ptr<ExprAST>> ast, ExpressionSemantic &t) {
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
                if (t.c->stackFrame.size() != 1)
                    throw std::logic_error("no top-level function def not supported");
                std::string realFuncName = t.c->generateUniqueName("func", t.toLegalName(v->name));
                if (v->funcDefType == FunctionDefAST::FuncDefType::INFIX) {
                    my_assert(v->params.size() == 2);
                    t.c->global.infixFuncDef[v->name].emplace(
                        std::tuple<rulejit::TypeInfo, rulejit::TypeInfo>{*(v->params[0]->type), *(v->params[1]->type)},
                        realFuncName);
                    t.c->global.realFuncDefinition[realFuncName] =
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
                    t.c->global.realFuncDefinition[realFuncName] = std::move(newExpr);
                }
            } else if (i && isType<SymbolCommandAST>(i.get())) {
                auto v = dynamic_cast<SymbolCommandAST *>(i.get());
                if (v->symbolCommandType == SymbolCommandAST::SymbolCommandType::EXTERN) {
                    t.c->global.externFuncDef[v->name] = *(v->definedType);
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
        for (auto &[name, func] : t.c->global.realFuncDefinition) {
            t.c->push();
            for (auto &&param : func->params) {
                t.c->top().varDef[param->name] = *(param->type);
            }
            func->returnValue->accept(&t);
            t.afterAccept(func->returnValue);
            if (*(func->returnValue->type) != func->funcType->getReturnedType()) {
                throw std::logic_error(std::format("function {} declared return {}, but return {} actually", func->name,
                                                   func->funcType->getReturnedType().toString(),
                                                   func->returnValue->type->toString()));
            }
            t.c->pop();
        }
        std::erase_if(ast, [](auto &i) { return i == nullptr; });
        return std::move(ast);
    }
    std::unique_ptr<ExprAST> friend operator|(std::unique_ptr<ExprAST> ast, ExpressionSemantic &t) {
        t.callStack.clear();
        t.needChange = nullptr;
        ast->accept(&t);
        t.afterAccept(ast);
        return std::move(ast);
    }
    VISIT_FUNCTION(IdentifierExprAST) {
        auto [find, type] = c->seekVarDef(v.name);
        if (!find) {
            return setError(std::format("Variable {} not defined", v.name));
        }
        if (!v.type) {
            v.type = std::make_unique<TypeInfo>(type);
        } else {
            if (*(v.type) != type) {
                return setError(std::format("Variable {} type mismatch", v.name));
            }
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
                // struct member access; TODO: member function, map
                auto definedType = c->global.typeDef.find(v.baseVar->type->idents[0]);
                if (definedType == c->global.typeDef.end()) {
                    return setError(std::format("Type {} not defined", v.baseVar->type->idents[0]));
                }
                auto memberType = definedType->second.getMemberType(p->value);
                *(v.type) = memberType;
            } else {
                return setError(std::format("unknown member access {}[\"{}\"]", v.baseVar->type->toString(), p->value));
            }
        } else if (v.baseVar->type->isArrayType()) {
            // array access
            *(v.type) = v.baseVar->type->getElementType();
        } else {
            return setError(std::format("unknown member access {}[{}]", v.baseVar->type->toString(),
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
        // only member access, unary and binop can overload
        v.functionIdent->accept(this);
        afterAccept(v.functionIdent);
        auto needParamNum =
            v.functionIdent->type->subTypes.size() - v.functionIdent->type->isReturnedFunctionType() ? 1 : 0;
        my_assert(needParamNum == v.params.size(),
                  std::format("Function need {} params, {} given", needParamNum, v.params.size()));
        for (size_t i = 0; i < v.params.size(); i++) {
            if (*(v.params[i]->type) != v.functionIdent->type->getArgType(i)) {
                return setError(std::format("Function arg {} type mismatch", i));
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
        if (*(v.lhs->type) == RealType && *(v.rhs->type) == RealType && binOp.contains(v.op)) {
            v.type = std::make_unique<TypeInfo>(RealType);
        } else if (v.op == "=") {
            v.type = std::make_unique<TypeInfo>(NoInstanceType);
            if (*(v.lhs->type) != *(v.rhs->type) || *(v.lhs->type) == NoInstanceType) {
                return setError(std::format("Assign between {} and {} not allowed", v.lhs->type->toString(),
                                            v.rhs->type->toString()));
            }
        } else {
            auto overLoad = c->global.infixFuncDef.find(v.op);
            if (overLoad == c->global.infixFuncDef.end()) {
                return setError(std::format("Operator {} between {} and {} not defined", v.op, v.lhs->type->toString(),
                                            v.rhs->type->toString()));
            }
            if (auto it = overLoad->second.find({*(v.lhs->type), *(v.rhs->type)}); it != overLoad->second.end()) {
                std::vector<std::unique_ptr<ExprAST>> tmp;
                tmp.push_back(std::move(v.lhs));
                tmp.push_back(std::move(v.rhs));
                auto funcType = c->getTypeByRealFunctionName(it->second);
                needChange = std::make_unique<FunctionCallExprAST>(
                    std::make_unique<TypeInfo>(funcType.getReturnedType()),
                    std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(funcType), it->second), std::move(tmp));
                return;
            } else {
                return setError(std::format("Operator {} between {} and {} not defined", v.op, v.lhs->type->toString(),
                                            v.rhs->type->toString()));
            }
        }
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
            auto it = c->global.typeDef.find(v.type->idents[0]);
            if (it == c->global.typeDef.end()) {
                return setError(std::format("Type {} not defined", v.type->toString()));
            }
            auto def = it->second;
            if (v.members.size() == 0) {
                return;
            }
            bool designate = std::get<0>(v.members[0]) != nullptr;
            if(!designate) {
                return setError("Complex type must be designate");
            }
            size_t cnt = 0;
            for (auto &&[ident, member] : v.members) {
                member->accept(this);
                afterAccept(member);
                ident->accept(this);
                afterAccept(ident);
                auto p = dynamic_cast<LiteralExprAST *>(ident.get());
                if(!p || *(p->type) != StringType) {
                    return setError("Non literal designate do not supported");
                }
                if(def.getMemberType(p->value) != *(member->type)) {
                    return setError(std::format("Designate type mismatch, {} expected, {} given", def.getMemberType(p->value).toString(), member->type->toString()));
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
                if(*(member->type) != def){
                    return setError(std::format("Array element type mismatch, {} expected, {} given", def.toString(), member->type->toString()));
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
        if (c->stackFrame.size() == 1 && v.typeDefType == TypeDefAST::TypeDefType::NORMAL) {
            if (!v.definedType->isComplexType() || v.definedType->idents[0] != "struct") {
                return setError("only allow struct type define");
            }
            for (auto &&t : v.definedType->subTypes) {
                if (t.isComplexType()) {
                    return setError(std::format("unnamed type {} is not allowed", t.toString()));
                }
                if (t != RealType && t != IntType && t.isBaseType() && !c->global.typeDef.contains(t.toString())) {
                    return setError(std::format("type {} is not defined", t.toString()));
                }
            }
            c->global.typeDef[v.name] = *(v.definedType);
            return;
        }
        return setError("Only allow top-level typedef, and no type alias supported");
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
        // TODO: process param type
        // func define only allowed top-level
        return setError("only top-level function def allowed");

        // if (c->stackFrame.size() != 1)
        //     throw std::logic_error("no top-level function def not supported");
        // std::string realFuncName = c->generateUniqueName("func", toLegalName(v.name));
        // if (v.funcDefType == FunctionDefAST::FuncDefType::INFIX) {
        //     my_assert(v.params.size() == 2);
        //     needChange = nop();
        //     c->global.infixFuncDef[v.name].emplace(
        //         std::tuple<rulejit::TypeInfo, rulejit::TypeInfo>{*(v.params[0]->type), *(v.params[1]->type)},
        //         realFuncName);
        //     c->global.realFuncDefinition[realFuncName] = std::move(asType<FunctionDefAST>(std::move(i)));
        // } else if (v->funcDefType == FunctionDefAST::FuncDefType::MEMBER) {
        //     needChange = nop();
        //     throw std::logic_error("no top-level member function def not supported");
        // } else {
        //     c->global.realFuncDefinition[realFuncName] = std::move(asType<FunctionDefAST>(std::move(i)));
        //     needChange = std::make_unique<VarDefAST>(
        //         v.name, std::make_unique<TypeInfo>(*(v.funcType)),
        //         std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(*(v.funcType)), realFuncName));
        // }
    }
    VISIT_FUNCTION(SymbolCommandAST) {
        // type define only allowed top-level
        return setError("SymbolCommandAST not supported");
    }

  private:
    std::string toLegalName(const std::string &token) {
        std::string tmp;
        for (auto c : token) {
            if (isalpha(c) || isdigit(c) || c == '_') {
                tmp += c;
            } else {
                tmp += std::to_string((size_t)c) + "_";
            }
        }
        return tmp;
    }
    [[noreturn]] void setError(const std::string &info,
                               const std::source_location location = std::source_location::current()) {

        throw std::logic_error(std::format("Type Check Error in {}::{}, line{}: {}", location.file_name(),
                                           location.function_name(), location.line(), info));
        // return nullptr;
    }
    std::unique_ptr<ExprAST> needChange;
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
        // if (type->isBaseType()) {
        //     auto tmp = c->seekTypeAlias(type->idents[0]);
        //     while (std::get<0>(tmp)) {
        //         type = std::make_unique<TypeInfo>(std::vector<std::string>{std::get<1>(tmp)});
        //         tmp = c->seekTypeAlias(type->idents[0]);
        //     }
        // }
        if (type->isComplexType()) {
            return setError(std::format("unnamed type {} is not allowed", type->toString()));
        }
    }
    // void pushStack(AST* v){callStack.push_back(v);}
    // void popStack(){callStack.pop_back();}
    ContextStack *c;
};

} // namespace rulejit