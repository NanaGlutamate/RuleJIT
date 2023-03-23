#pragma once

#include <cmath>
#include <functional>
#include <iostream>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/type.hpp"
#include "backend/cq/cqresourcehandler.h"

namespace rulejit::cq {

// will change original AST
struct CQInterpreter : public ASTVisitor {
    std::map<std::string, std::unique_ptr<FunctionDefAST>> func;
    CQInterpreter() : symbolStack({{{}}}) {}

    void setResourceHandler(ResourceHandler *h) { handler = h; };
    void friend operator|(std::unique_ptr<ExprAST> &expr, CQInterpreter &interpreter) { expr->accept(&interpreter); }

    VISIT_FUNCTION(IdentifierExprAST) {
        auto find = seekValue(v.name);
        if (!find) {
            returned.token = handler->readIn(v.name);
            returned.type = Value::TOKEN;
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        v.baseVar->accept(this);
        auto base = returned;
        if (returned.type != Value::TOKEN) {
            setError("f64 have no members");
        }
        if (*(v.memberToken->type) == StringType) {
            if (v.memberToken->type->isArrayType()) {
                if (returned.type == Value::TOKEN) {
                    returned.value = (double)handler->arrayLength(returned.token);
                    returned.type = Value::VALUE;
                } else {
                    setError("only array have \"length\" member");
                }
                return;
            }
            returned.token =
                handler->memberAccess(base.token, dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value);
        } else if (*(v.memberToken->type) == RealType) {
            returned.token =
                handler->arrayAccess(base.token, std::stoi(dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value));
        } else {
            setError("member access only accept string or int");
        }
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (*(v.type) == RealType) {
            returned.value = std::stod(v.value);
            returned.type = Value::VALUE;
        } else if (*(v.type) == StringType) {
            // TODO: check
            returned.token = handler->take(v.value);
            returned.type = Value::TOKEN;
        } else {
            returned.type = Value::EMPTY;
        }
        // nop?
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        if (!isType<IdentifierExprAST>(v.functionIdent.get())) {
            setError("only allow direct function(no member, no returned funciton) for now");
        }
        static std::map<std::string, std::function<double(double)>> oneParamFunc{
            {"-", [](double x) {return -x; }},         {"not", [](double x) {return !x; }},
            {"sin", [](double x) {return sin(x); }},   {"cos", [](double x) {return cos(x); }},
            {"tan", [](double x) {return tan(x); }},   {"cot", [](double x) {return 1.0/tan(x); }},
            {"atan", [](double x) {return atan(x); }}, {"asin", [](double x) {return asin(x); }},
            {"acos", [](double x) {return acos(x); }}, {"fabs", [](double x) {return fabs(x); }},
            {"exp", [](double x) {return exp(x); }},   {"abs", [](double x) {return fabs(x); }},
        };
        static std::map<std::string, std::function<double(double, double)>> twoParamFunc{
            {"pow", [](double x, double y) { return pow(x, y); }},
            {"atan2", [](double x, double y) { return atan2(x, y); }},
        };
        auto p = dynamic_cast<IdentifierExprAST *>(v.functionIdent.get());
        if (p->name == "print") {
            my_assert(v.params.size() == 1, "\"print\" only accept 1 param");
            v.params[0]->accept(this);
            if (returned.type == Value::TOKEN) {
                if (handler->isString(returned.token)) {
                    std::cout << handler->readString(returned.token) << std::endl;
                } else {
                    std::cout << handler->readValue(returned.token) << std::endl;
                }
            } else {
                std::cout << returned.value << std::endl;
            }
            returned.type = Value::EMPTY;
        } else if (auto it = oneParamFunc.find(p->name); it != oneParamFunc.end()) {
            my_assert(v.params.size() == 1, std::format("\"{}\" only accept 1 param", p->name));
            v.params[0]->accept(this);
            getReturnedValue();
            returned.value = it->second(returned.value);
        } else if (auto it = twoParamFunc.find(p->name); it != twoParamFunc.end()) {
            my_assert(v.params.size() == 2, std::format("\"{}\" only accept 2 param", p->name));
            v.params[0]->accept(this);
            getReturnedValue();
            double tmp = returned.value;
            v.params[1]->accept(this);
            getReturnedValue();
            returned.value = it->second(tmp, returned.value);
        } else {
            auto f = func.find(p->name);
            if (f == func.end()) {
                setError(std::format("function \"{}\" not found", p->name));
            }
            auto &callee = f->second;
            std::vector<std::map<std::string, rulejit::cq::CQInterpreter::Value>> frame{{}};
            my_assert(callee->params.size() == v.params.size(),
                      std::format("\"{}\" only accept {} params, but {} was given", p->name, callee->params.size(),
                                  v.params.size()));
            for (size_t i = 0; i < callee->params.size(); i++) {
                auto &param = callee->params[i];
                auto &arg = v.params[i];
                arg->accept(this);
                if (isSupportType(*(param->type))) {
                    try {
                        getReturnedValue();
                    } catch (...) {
                        setError("try express complex type as numerical type");
                    }
                }
                frame.back()[param->name] = returned;
            }
            symbolStack.push_back(std::move(frame));
            returned.type = Value::EMPTY;
            callee->returnValue->accept(this);
            symbolStack.pop_back();
        }
    }
    VISIT_FUNCTION(BinOpExprAST) {
        static std::map<std::string, std::function<double(double, double)>> normalBinOp{
            {"+", [](auto x, auto y) { return x + y; }},   {"-", [](auto x, auto y) { return x - y; }},
            {"*", [](auto x, auto y) { return x * y; }},   {"/", [](auto x, auto y) { return x / y; }},
            {">", [](auto x, auto y) { return x > y; }},   {"<", [](auto x, auto y) { return x < y; }},
            {"==", [](auto x, auto y) { return x == y; }}, {"!=", [](auto x, auto y) { return x != y; }},
            {">=", [](auto x, auto y) { return x >= y; }}, {"<=", [](auto x, auto y) { return x <= y; }},
        };
        static std::map<std::string, std::function<double(double, double)>> shortCutBinOp{
            {"&&", [](auto x, auto y) { return x && y; }},
            {"and", [](auto x, auto y) { return x && y; }},
            {"||", [](auto x, auto y) { return x || y; }},
            {"or", [](auto x, auto y) { return x || y; }},
        };
        if (v.op == "=") {
            if (isType<IdentifierExprAST>(v.lhs.get())) {
                auto p = dynamic_cast<IdentifierExprAST *>(v.lhs.get());
                p->accept(this);
                auto lhs = returned;
                v.rhs->accept(this);
                if (lhs.type == Value::TOKEN) {
                    if (returned.type == Value::TOKEN) {
                        handler->assign(lhs.token, returned.token);
                    } else {
                        handler->writeValue(lhs.token, returned.value);
                    }
                } else {
                    getReturnedValue();
                    for (auto it = symbolStack.back().rbegin(); it != symbolStack.back().rend(); it++) {
                        if (it->find(p->name) != it->end()) {
                            (*it)[p->name] = returned;
                            return;
                        }
                    }
                }
            } else if (isType<MemberAccessExprAST>(v.lhs.get())) {
                v.lhs->accept(this);
                auto tmp = returned;
                v.rhs->accept(this);
                if (returned.type == Value::VALUE) {
                    handler->writeValue(tmp.token, returned.value);
                } else {
                    handler->assign(tmp.token, returned.token);
                }
            } else {
                setError("only allow direct or member variable assignment for now");
            }
            returned.type = Value::EMPTY;
        } else if (auto it = normalBinOp.find(v.op); it != normalBinOp.end()) {
            v.lhs->accept(this);
            if (v.op == "==" && returned.type == Value::TOKEN && handler->isString(returned.token)) {
                auto tmp = returned.token;
                v.rhs->accept(this);
                if (returned.type != Value::TOKEN || !handler->isString(returned.token)) {
                    setError("can't compare string with non-string");
                }
                returned.type = Value::VALUE;
                // TODO: compare between taked string and normal input string
                returned.value = tmp == returned.token;
                return;
            }
            getReturnedValue();
            auto tmp = returned.value;
            v.rhs->accept(this);
            getReturnedValue();
            returned.value = it->second(tmp, returned.value);
        } else if (auto it = shortCutBinOp.find(v.op); it != shortCutBinOp.end()) {
            v.lhs->accept(this);
            getReturnedValue();
            auto tmp = returned.value;
            if (it->second(tmp, 0) != it->second(tmp, 1)) {
                v.rhs->accept(this);
                getReturnedValue();
                returned.value = it->second(tmp, returned.value);
            }
            returned.type = Value::VALUE;
        } else {
            setError(std::format("bin op \"{}\" not support for now", v.op));
        }
    }
    VISIT_FUNCTION(BranchExprAST) {
        returned.type = Value::EMPTY;
        v.condition->accept(this);
        getReturnedValue();
        if (returned.value != 0) {
            v.trueExpr->accept(this);
        } else {
            v.falseExpr->accept(this);
        }
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        auto typeName = v.type->toString();
        auto tmp = handler->makeInstance(typeName);
        for (auto &&[name, value] : v.members) {
            auto p = dynamic_cast<LiteralExprAST *>(name.get());
            if (p == nullptr || !(*(p->type) == StringType)) {
                setError("only allow string literal as key for now");
            }
            value->accept(this);
            auto memberToken = handler->memberAccess(tmp, p->value);
            if (returned.type == Value::VALUE) {
                handler->writeValue(memberToken, returned.value);
            } else {
                handler->assign(memberToken, returned.token);
            }
        }
        returned.token = tmp;
        returned.type = Value::TOKEN;
    }
    VISIT_FUNCTION(LoopAST) {
        returned.type = Value::EMPTY;
        symbolStack.back().emplace_back();
        v.init->accept(this);
        while (v.condition->accept(this), getReturnedValue(), returned.value != 0) {
            v.body->accept(this);
        }
        symbolStack.back().pop_back();
    }
    VISIT_FUNCTION(BlockExprAST) {
        returned.type = Value::EMPTY;
        symbolStack.back().emplace_back();
        for (auto it = v.exprs.begin(); it != v.exprs.end();) {
            if (isType<FunctionDefAST>(it->get())) {
                auto funcDef = std::unique_ptr<FunctionDefAST>(dynamic_cast<FunctionDefAST *>(it->release()));
                if (func.contains(funcDef->name)) {
                    setError("redefined function: " + funcDef->name);
                }
                func[funcDef->name] = std::move(funcDef);
                it = v.exprs.erase(it, it + 1);
            } else {
                (*it)->accept(this);
                it++;
            }
        }
        symbolStack.back().pop_back();
    }
    VISIT_FUNCTION(ControlFlowAST) { setError("ControlFlowAST should never be visit directly"); }
    VISIT_FUNCTION(TypeDefAST) {
        if (!v.definedType->isComplexType()) {
            setError("only allow complex type define for now");
        }
        auto name = v.name;
        if (!v.definedType->isComplexType() || v.definedType->idents[0] != "struct") {
            setError("only allow struct type (no reference type support) define for now");
        }
        std::unordered_map<std::string, std::string> t;
        for (int i = 0; i < v.definedType->subTypes.size(); ++i) {
            static std::unordered_map<std::string, std::string> typeAlias{
                {"i8", "int8"},    {"u8", "uint8"},  {"i16", "int16"},  {"u16", "uint16"},  {"i32", "int32"},
                {"u32", "uint32"}, {"i64", "int64"}, {"u64", "uint64"}, {"f32", "float32"}, {"f64", "float64"},
            };
            auto tmp = v.definedType->subTypes[i].toString();
            if (typeAlias.contains(tmp)) {
                tmp = typeAlias[tmp];
            }
            t[v.definedType->idents[i + 1]] = tmp;
        }
        handler->defineType(name, t);
        returned.type = Value::EMPTY;
    }
    VISIT_FUNCTION(VarDefAST) {
        returned.type = Value::EMPTY;
        if (auto it = symbolStack.back().back().find(v.name); it != symbolStack.back().back().end()) {
            setError("redefine variable: " + v.name);
        }
        // if (!isSupportType(*(v.valueType))) {
        //     setError("unsupported type: " + v.valueType->toString());
        // }
        v.definedValue->accept(this);
        if (returned.type == Value::TOKEN) {
            auto tmp = handler->makeInstanceAs(returned.token);
            handler->assign(tmp, returned.token);
            returned.token = tmp;
            symbolStack.back().back()[v.name] = returned;
        } else if (returned.type == Value::VALUE) {
            symbolStack.back().back()[v.name] = returned;
        } else if (returned.type == Value::EMPTY) {
            setError("def var to empty value");
        } else {
            setError("unsupported type");
        }
        returned.type = Value::EMPTY;
    }
    VISIT_FUNCTION(FunctionDefAST) { setError("function def should never be visit directly"); }

    // private:
    struct Value {
        union {
            size_t token;
            double value;
        };
        enum valueType {
            TOKEN,
            VALUE,
            EMPTY,
        } type;
    };

    ResourceHandler *handler;
    bool seekValue(const std::string &s) {
        for (auto it = symbolStack.back().rbegin(); it != symbolStack.back().rend(); ++it) {
            if (auto it2 = it->find(s); it2 != it->end()) {
                returned = it2->second;
                return true;
            }
        }
        return false;
    }
    // caller pop stack
    std::vector<std::vector<std::map<std::string, Value>>> symbolStack;
    Value returned;
    bool isSupportType(const TypeInfo &type) {
        return type.isValid() && type.isBaseType() && (type.idents[0] == "f64" || type == AutoType);
    }
    void getReturnedValue() {
        if (returned.type == Value::EMPTY) {
            setError("no value returned");
        }
        if (returned.type == Value::TOKEN) {
            returned.value = handler->readValue(returned.token);
            returned.type = Value::VALUE;
        }
    }
    [[noreturn]] void setError(const std::string &msg) { throw std::logic_error(msg); }
};

} // namespace rulejit::cq