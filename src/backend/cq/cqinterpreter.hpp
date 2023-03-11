#pragma once

#include <functional>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/type.hpp"
#include "backend/cq/cqresourcehandler.h"

namespace rulejit::cq {

struct CQInterpreter : public ASTVisitor {

    void setResourceHandler(ResourceHandler *h) { handler = h; };
    void friend operator|(std::unique_ptr<ExprAST> &expr, CQInterpreter &interpreter) { expr->accept(&interpreter); }

    VISIT_FUNCTION(IdentifierExprAST) {
        try {
            returned = seekValue(v.name);
        } catch (...) {
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
            returned.token =
                handler->memberAccess(base.token, dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value);
        } else if (*(v.memberToken->type) == RealType) {
            returned.token =
                handler->arrayAccess(base.token, std::stoi(dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value));
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
        }
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        if (!isType<IdentifierExprAST>(v.functionIdent.get())) {
            setError("only allow direct function(no member, no returned funciton) for now");
        }
        auto p = dynamic_cast<IdentifierExprAST *>(v.functionIdent.get());
        if (p->name == "len") {
            my_assert(v.params.size() == 1, "\"len\" only accept 1 param");
            v.params[0]->accept(this);
            if (returned.type == Value::TOKEN) {
                returned.value = handler->arrayLength(returned.token);
                returned.type = Value::VALUE;
            } else {
                setError("\"len\" only accept array");
            }
        } else if (p->name == "not") {
            my_assert(v.params.size() == 1, "\"not\" only accept 1 param");
            v.params[0]->accept(this);
            getReturnedValue();
            if (returned.value == 0) {
                returned.value = 1;
            } else {
                returned.value = 0;
            }
        } else {
            auto &callee = func[p->name];
            symbolStack.push_back({{}});
            my_assert(callee->params.size() == v.params.size(),
                      std::format("\"{}\" only accept {} params, but {} was given", p->name, callee->params.size(),
                                  v.params.size()));
            for (size_t i = 0; i < callee->params.size(); i++) {
                auto &param = callee->params[i];
                auto &arg = v.params[i];
                arg->accept(this);
                symbolStack.back().back()[param->name] = returned;
            }
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
                v.rhs->accept(this);
                symbolStack.back().back()[p->name] = returned;
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
        v.condition->accept(this);
        getReturnedValue();
        if (returned.value != 0) {
            v.trueExpr->accept(this);
        } else {
            v.falseExpr->accept(this);
        }
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) { setError("execution ComplexLiteralExprAST not support for now"); }
    VISIT_FUNCTION(LoopAST) {
        symbolStack.back().emplace_back();
        v.init->accept(this);
        while (v.condition->accept(this), getReturnedValue(), returned.value != 0) {
            v.body->accept(this);
        }
        symbolStack.back().pop_back();
    }
    VISIT_FUNCTION(BlockExprAST) {
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
    VISIT_FUNCTION(ControlFlowAST) {
        switch (v.controlFlowType) { setError("execution ControlFlowAST not support for now"); }
    }
    VISIT_FUNCTION(TypeDefAST) { setError("execution TypeDefAST not support for now"); }
    VISIT_FUNCTION(VarDefAST) {
        if (auto it = symbolStack.back().back().find(v.name); it != symbolStack.back().back().end()) {
            setError("redefine variable: " + v.name);
        }
        if (!isSupportType(*(v.type))) {
            setError("unsupported type: " + v.type->toString());
        }
        v.definedValue->accept(this);
        symbolStack.back().back()[v.name] = returned;
    }
    VISIT_FUNCTION(FunctionDefAST) { setError("function def should never be visit directly"); }

  private:
    struct Value {
        union {
            size_t token;
            double value;
        };
        enum valueType {
            TOKEN,
            VALUE,
        } type;
    };

    ResourceHandler *handler;
    Value seekValue(const std::string &s) {
        for (auto it = symbolStack.back().rbegin(); it != symbolStack.back().rend(); ++it) {
            if (auto it2 = it->find(s); it2 != it->end()) {
                return (it2->second);
            }
        }
        setError("unknown variable: " + s);
    }
    // caller pop stack
    std::vector<std::vector<std::map<std::string, Value>>> symbolStack;
    std::map<std::string, std::unique_ptr<FunctionDefAST>> func;
    Value returned;
    bool isSupportType(const TypeInfo &type) {
        return type.isValid() && type.isSingleToken() && (type.idents[0] == "f64" || type == AutoType);
    }
    void getReturnedValue() {
        if (returned.type == Value::TOKEN) {
            returned.value = handler->readValue(returned.token);
            returned.type = Value::VALUE;
        }
    }
    [[noreturn]] void setError(const std::string &msg) { throw std::logic_error(msg); }
};

} // namespace rulejit::cq