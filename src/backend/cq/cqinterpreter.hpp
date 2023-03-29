/**
 * @file cqinterpreter.hpp
 * @author djw
 * @brief CQ/Interpreter/Interpreter
 * @date 2023-03-27
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-29</td><td>Add semantic support.</td></tr>
 * </table>
 */

#pragma once

#include <cmath>
#include <format>
#include <functional>
#include <iostream>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"
#include "ast/decompiler.hpp"
#include "ast/type.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "defines/marco.hpp"
#include "ast/context.hpp"
#include "tools/seterror.hpp"

/**
 * @brief front declaration of main function, used for repl_main.cpp
 *
 * @return int
 */
int main();

namespace rulejit::cq {

/**
 * @brief main class to interprete ast in cq environment
 *
 */
struct CQInterpreter : public ASTVisitor {
    friend int ::main();

    /**
     * @brief Constructor, will init symbolStack with empty stack and empty scope,
     * then set handler to h
     *
     * @param h ResourceHandler which handles variable for this object
     */
    CQInterpreter(ContextStack& c, ResourceHandler &h) : context(c), symbolStack({{{}}}), handler(h) {}

    /**
     * @brief reset the interpreter(reset symbolStack, specifically)
     * @attention will not remove the function definitions
     *
     */
    void reset() { symbolStack = {{{}}}; }

    /**
     * @brief pipe operator| to interprete ast
     *
     * @param expr expr ast need to be interpreted
     * @param interpreter acceptor interpreter object
     */
    void friend operator|(std::unique_ptr<ExprAST> &expr, CQInterpreter &interpreter) {
#ifdef __RULEJIT_INTERPRETER_DEBUG
        interpreter.ruleCnt = 1;
#endif
        expr->accept(&interpreter);
    }

  protected:
    VISIT_FUNCTION(IdentifierExprAST) {
        if (v.name == "true") {
            returned.value = 1.;
            returned.type = Value::VALUE;
        } else if (v.name == "false") {
            returned.value = 0.;
            returned.type = Value::VALUE;
        } else {
            auto find = seekValue(v.name);
            if (!find) {
                // !find means its a variable hold by CQResourceHandler, so read it
                returned.token = handler.readIn(v.name);
                returned.type = Value::TOKEN;
            }
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        v.baseVar->accept(this);
        auto base = returned;
        if (returned.type != Value::TOKEN) {
            setError("number have no members");
        }
        if (!v.memberToken->type) {
            // v.memberToken->type = nullptr means it need to be evaluate
            v.memberToken->accept(this);
            getReturnedValue();
            returned.token = handler.arrayAccess(base.token, static_cast<size_t>(returned.value));
            returned.type = Value::TOKEN;
            return;
        }
        if (*(v.memberToken->type) == StringType) {
            returned.token =
                handler.memberAccess(base.token, dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value);
        } else if (*(v.memberToken->type) == RealType) {
            returned.token =
                handler.arrayAccess(base.token, std::stoi(dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value));
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
            returned.token = handler.take(v.value);
            returned.type = Value::TOKEN;
        } else {
            returned.type = Value::EMPTY;
        }
    }
    VISIT_FUNCTION(FunctionCallExprAST) {
        // build in function with 1 param
        static std::map<std::string, std::function<double(double)>> oneParamFunc{
            {"sin", [](double x) {return sin(x); }},     {"cos", [](double x) {return cos(x); }},
            {"tan", [](double x) {return tan(x); }},     {"cot", [](double x) {return 1.0/tan(x); }},
            {"atan", [](double x) {return atan(x); }},   {"asin", [](double x) {return asin(x); }},
            {"acos", [](double x) {return acos(x); }},   {"fabs", [](double x) {return fabs(x); }},
            {"exp", [](double x) {return exp(x); }},     {"abs", [](double x) {return fabs(x); }},
            {"floor", [](double x) {return floor(x); }}, {"sqrt", [](double x) {return sqrt(x); }},
        };
        // build in function with 2 param
        static std::map<std::string, std::function<double(double, double)>> twoParamFunc{
            {"pow", [](double x, double y) { return pow(x, y); }},
            {"atan2", [](double x, double y) { return atan2(x, y); }},
        };
        std::string funcName;
        if (isType<IdentifierExprAST>(v.functionIdent.get())) {
            auto p = dynamic_cast<IdentifierExprAST *>(v.functionIdent.get());
            funcName = p->name;
        } else if (isType<LiteralExprAST>(v.functionIdent.get())) {
            auto p = dynamic_cast<LiteralExprAST *>(v.functionIdent.get());
            funcName = p->value;
        } else {
            return setError("only allow direct function(no member, no returned funciton) for now");
        }
        if (funcName == "print") {
            my_assert(v.params.size() == 1, "\"print\" only accept 1 param");
            v.params[0]->accept(this);
            if (returned.type == Value::TOKEN) {
                if (handler.isString(returned.token)) {
                    std::cout << handler.readString(returned.token) << std::endl;
                } else {
                    std::cout << handler.readValue(returned.token) << std::endl;
                }
            } else {
                std::cout << returned.value << std::endl;
            }
            returned.type = Value::EMPTY;
        } else if (auto it = oneParamFunc.find(funcName); it != oneParamFunc.end()) {
            my_assert(v.params.size() == 1, std::format("\"{}\" only accept 1 param", funcName));
            v.params[0]->accept(this);
            getReturnedValue();
            returned.value = it->second(returned.value);
        } else if (auto it = twoParamFunc.find(funcName); it != twoParamFunc.end()) {
            my_assert(v.params.size() == 2, std::format("\"{}\" only accept 2 param", funcName));
            v.params[0]->accept(this);
            getReturnedValue();
            double tmp = returned.value;
            v.params[1]->accept(this);
            getReturnedValue();
            returned.value = it->second(tmp, returned.value);
        } else {
            auto f = context.global.realFuncDefinition.find(funcName);
            if (f == context.global.realFuncDefinition.end()) {
                setError(std::format("function \"{}\" not found", funcName));
            }
            auto &callee = f->second;
            std::vector<std::map<std::string, rulejit::cq::CQInterpreter::Value>> frame{{}};
            my_assert(callee->params.size() == v.params.size(),
                      std::format("\"{}\" only accept {} params, but {} was given", funcName, callee->params.size(),
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
                        handler.assign(lhs.token, returned.token);
                    } else {
                        handler.writeValue(lhs.token, returned.value);
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
                    handler.writeValue(tmp.token, returned.value);
                } else {
                    handler.assign(tmp.token, returned.token);
                }
            } else {
                setError("only allow direct or member variable assignment for now");
            }
            returned.type = Value::EMPTY;
        } else if (auto it = normalBinOp.find(v.op); it != normalBinOp.end()) {
            v.lhs->accept(this);
            if (v.op == "==" && returned.type == Value::TOKEN && handler.isString(returned.token)) {
                auto tmp = returned.token;
                v.rhs->accept(this);
                if (returned.type != Value::TOKEN || !handler.isString(returned.token)) {
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
#ifdef __RULEJIT_INTERPRETER_DEBUG
            bool rhsEvaluate = false;
            double tmp1;
#endif
            if (it->second(tmp, 0) != it->second(tmp, 1)) {
                v.rhs->accept(this);
                getReturnedValue();
#ifdef __RULEJIT_INTERPRETER_DEBUG
                rhsEvaluate = true;
                tmp1 = returned.value;
#endif
                returned.value = it->second(tmp, returned.value);
            }
            returned.type = Value::VALUE;
#ifdef __RULEJIT_INTERPRETER_DEBUG
            if (rhsEvaluate) {
                std::cout << std::format("Evaluate: {0} {1} {2} =>\n"
                                         "          {3} {1} {4} =>\n"
                                         "          {5}",
                                         v.lhs | Decompiler(), v.op, v.rhs | Decompiler(), tmp, tmp1, returned.value)
                          << std::endl;
            } else {
                std::cout << std::format("Evaluate: {0} {1} {2} =>\n"
                                         "          {3} {1} [Shortcutted] =>\n"
                                         "          {4}",
                                         v.lhs | Decompiler(), v.op, v.rhs | Decompiler(), tmp, returned.value)
                          << std::endl;
            }
#endif
        } else {
            setError(std::format("bin op \"{}\" not support for now", v.op));
        }
    }
    VISIT_FUNCTION(UnaryOpExprAST) {
        static std::map<std::string, std::function<double(double)>> unaryFunc{
            {"-", [](double x) { return -x; }},
            {"not", [](double x) { return !x; }},
            {"!", [](double x) { return !x; }},
        };
        if (!unaryFunc.contains(v.op)) {
            setError(std::format("unary op \"{}\" not support for now", v.op));
        }
        v.rhs->accept(this);
        getReturnedValue();
        returned.value = unaryFunc[v.op](returned.value);
    }
    VISIT_FUNCTION(BranchExprAST) {
#ifdef __RULEJIT_INTERPRETER_DEBUG
        // TODO: if expr in condition?
        std::cout << "[Check Rule No." << ruleCnt++ << "]" << std::endl;
#endif
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
        auto tmp = handler.makeInstance(typeName);
        for (auto &&[name, value] : v.members) {
            auto p = dynamic_cast<LiteralExprAST *>(name.get());
            if (p == nullptr || !(*(p->type) == StringType)) {
                setError("only allow string literal as key for now");
            }
            value->accept(this);
            auto memberToken = handler.memberAccess(tmp, p->value);
            if (returned.type == Value::VALUE) {
                handler.writeValue(memberToken, returned.value);
            } else {
                handler.assign(memberToken, returned.token);
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
            (*it)->accept(this);
            it++;
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
        handler.defineType(name, t);
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
            auto tmp = handler.makeInstanceAs(returned.token);
            handler.assign(tmp, returned.token);
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
    VISIT_FUNCTION(SymbolDefAST) { setError("symbol def should never be visit directly"); }

  private:
    /**
     * @brief Value type passes through visit functions,
     * may be a double or a token which indicate a variable hold by CQResourceHandler
     *
     */
    struct Value {
        union {
            size_t token;
            double value;
        };
        /**
         * @brief type of value, for now only support VALUE(which mean Value::value is available),
         * TOKEN(which mean Value::token is available),
         * and EMPTY(which mean both is not available)
         *
         */
        enum valueType {
            TOKEN,
            VALUE,
            EMPTY,
        } type;
    };
    ResourceHandler &handler; /**< holds CQ-related variables*/
    /**
     * @brief seek variable with given name in current stack frame
     * @attention if found, will modify returned
     *
     * @param s variable name
     * @return bool true if found
     */
    bool seekValue(const std::string &s) {
        for (auto it = symbolStack.back().rbegin(); it != symbolStack.back().rend(); ++it) {
            if (auto it2 = it->find(s); it2 != it->end()) {
                returned = it2->second;
                return true;
            }
        }
        return false;
    }
    /**
     * @brief check if given type supported
     *
     * @param type string of type name
     * @return true if is supported type
     */
    bool isSupportType(const TypeInfo &type) {
        return type.isValid() && type.isBaseType() && (type.idents[0] == "f64" || type == AutoType);
    }
    /**
     * @brief make "returned" a value,
     * specifically, if returned is a token, get its value from CQResourceHandler;
     * if returned is already a value, do nothing.
     * if returned is empty, throw an exception.
     *
     */
    void getReturnedValue() {
        if (returned.type == Value::EMPTY) {
            setError("no value returned");
        }
        if (returned.type == Value::TOKEN) {
            returned.value = handler.readValue(returned.token);
            returned.type = Value::VALUE;
        }
    }
    /**
     * @brief set errors
     *
     * @param msg error message
     */
    [[noreturn]] void setError(const std::string &msg) { error(msg); }

    /**
     * @brief context, includes function defines
     * 
     */
    ContextStack& context;

    /**
     * @brief caller pop stack, stack frame is scope stack
     *
     */
    std::vector<std::vector<std::map<std::string, Value>>> symbolStack;

    /**
     * @brief value passed through visit functions
     *
     */
    Value returned;
#ifdef __RULEJIT_INTERPRETER_DEBUG
    size_t ruleCnt;
#endif
};

} // namespace rulejit::cq