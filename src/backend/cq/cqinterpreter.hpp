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
#include "ast/context.hpp"
#include "ast/decompiler.hpp"
#include "ast/type.hpp"
#include "backend/cq/cqresourcehandler.h"
#include "defines/marco.hpp"
#include "tools/seterror.hpp"

#define setErrorWhenFailed(cond, info)                                                                                 \
    if (!(cond))                                                                                                       \
    error((info))

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
 * TODO: remove type check; only check when runtime error
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
    CQInterpreter(ContextStack &c, ResourceHandler &h) : context(c), symbolStack({{{}}}), handler(h) {}

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
        interpreter.currentExpr.clear();
        interpreter.callAccept(expr);
    }

  protected:
    VISIT_FUNCTION(IdentifierExprAST) {
        auto find = seekValue(v.name);
        if (!find) {
            // !find means its a variable hold by CQResourceHandler, so read it
            returned.token = handler.readIn(v.name);
            returned.type = Value::TOKEN;
        }
    }
    VISIT_FUNCTION(MemberAccessExprAST) {
        callAccept(v.baseVar);
        auto base = returned;
        if (returned.type != Value::TOKEN) {
            setError("number have no members");
        }
        if (*(v.memberToken->type) == StringType) {
            returned.token =
                handler.memberAccess(base.token, dynamic_cast<LiteralExprAST *>(v.memberToken.get())->value);
        } else if (*(v.memberToken->type) == RealType) {
            callAccept(v.memberToken);
            getReturnedValue();
            setErrorWhenFailed(returned.value == (double)floor(returned.value),
                               "array index out of range (should can be cast to int)");
            returned.token = handler.arrayAccess(base.token, (size_t)returned.value);
        } else {
            setError("member access only accept string or int");
        }
        returned.type = Value::TOKEN;
    }
    VISIT_FUNCTION(LiteralExprAST) {
        if (*(v.type) == RealType) {
            returned.value = std::stod(v.value);
            returned.type = Value::VALUE;
        } else if (*(v.type) == StringType) {
            // TODO: check
            returned.token = handler.takeString(v.value);
            returned.type = Value::TOKEN;
        } else if (v.type->isFunctionType()) {
            returned.token = handler.takeString(v.value);
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
        // should provide all function defined in initprocess in frontend/ruleset/rulesetparser.cpp
        std::string funcName;
        if (isType<LiteralExprAST>(v.functionIdent.get())) {
            auto p = dynamic_cast<LiteralExprAST *>(v.functionIdent.get());
            funcName = p->value;
        } else {
            return setError("only allow direct function call, cannot call function through variable");
        }
        if (funcName == "print") {
            setErrorWhenFailed(v.params.size() == 1, "\"print\" only accept 1 param");
            callAccept(v.params[0]);
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
        } else if (funcName == "length") {
            setErrorWhenFailed(v.params.size() == 1, "\"length\" only accept 1 param");
            callAccept(v.params[0]);
            setErrorWhenFailed(returned.type == Value::TOKEN, "expect array as receiver of \"length\"");
            returned.type = Value::VALUE;
            returned.value = static_cast<double>(handler.arrayLength(returned.token));
        } else if (funcName == "push") {
            setErrorWhenFailed(v.params.size() == 2, "\"push\" only accept 2 param");
            callAccept(v.params[0]);
            auto arg1 = returned;
            callAccept(v.params[1]);
            auto arg2 = returned;
            setErrorWhenFailed(arg1.type == Value::TOKEN, "expect array as receiver of \"push\"");
            if (arg2.type == Value::VALUE) {
                handler.arrayExtend(arg1.token, arg2.value);
            } else if (arg2.type == Value::TOKEN) {
                handler.arrayExtend(arg1.token, arg2.token);
            } else {
                setError("arg2 of \"push\" has no return");
            }
        } else if (funcName == "resize") {
            setErrorWhenFailed(v.params.size() == 2, "\"resize\" only accept 2 param");
            callAccept(v.params[0]);
            auto arg1 = returned;
            callAccept(v.params[1]);
            getReturnedValue();
            auto arg2 = returned.value;
            setErrorWhenFailed(arg1.type == Value::TOKEN, "expect array as receiver of \"resize\"");
            setErrorWhenFailed(arg2 == (double)floor(arg2), "array index out of range (should can be cast to int)");
            handler.arrayResize(arg1.token, static_cast<size_t>(arg2));
        } else if (funcName == "strEqual") {
            callAccept(v.params[0]);
            returned.type == Value::TOKEN &&handler.isString(returned.token);
            auto tmp = returned.token;
            callAccept(v.params[1]);
            if (returned.type != Value::TOKEN || !handler.isString(returned.token)) {
                setError("can't compare string with non-string");
            }
            returned.type = Value::VALUE;
            returned.value = handler.stringComp(tmp, returned.token);
        } else if (auto it = oneParamFunc.find(funcName); it != oneParamFunc.end()) {
            setErrorWhenFailed(v.params.size() == 1, std::format("\"{}\" only accept 1 param", funcName));
            callAccept(v.params[0]);
            getReturnedValue();
            returned.value = it->second(returned.value);
            returned.type = Value::VALUE;
        } else if (auto it = twoParamFunc.find(funcName); it != twoParamFunc.end()) {
            setErrorWhenFailed(v.params.size() == 2, std::format("\"{}\" only accept 2 param", funcName));
            callAccept(v.params[0]);
            getReturnedValue();
            double tmp = returned.value;
            callAccept(v.params[1]);
            getReturnedValue();
            returned.value = it->second(tmp, returned.value);
            returned.type = Value::VALUE;
        } else {
            auto f = context.global.realFuncDefinition.find(funcName);
            if (f == context.global.realFuncDefinition.end()) {
                setError(std::format("function \"{}\" not found", funcName));
            }
            auto &callee = f->second;
            std::vector<std::map<std::string, rulejit::cq::CQInterpreter::Value>> frame{{}};
            for (size_t i = 0; i < callee->params.size(); i++) {
                auto &param = callee->params[i];
                auto &arg = v.params[i];
                callAccept(arg);
                if (isNumericalType(*(param->type))) {
                    getReturnedValue();
                }
                frame.back()[param->name] = returned;
            }
            symbolStack.push_back(std::move(frame));
            returned.type = Value::EMPTY;
            callAccept(callee->returnValue);
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
            // TODO: not make sense
            {"%", [](auto x, auto y) { return static_cast<int64_t>(x) % static_cast<int64_t>(y); }},
        };
        static std::map<std::string, std::function<double(double, double)>> shortCutBinOp{
            {"&&", [](auto x, auto y) { return x && y; }},
            {"and", [](auto x, auto y) { return x && y; }},
            {"||", [](auto x, auto y) { return x || y; }},
            {"or", [](auto x, auto y) { return x || y; }},
        };
        if (v.op == "=") {
            if (isType<IdentifierExprAST>(v.lhs.get())) {
                callAccept(v.rhs);
                auto rhs = returned;
                auto p = dynamic_cast<IdentifierExprAST *>(v.lhs.get());
                callAccept(v.lhs);
                auto lhs = returned;
                if (lhs.type == Value::TOKEN) {
                    if (rhs.type == Value::TOKEN) {
                        handler.assign(lhs.token, rhs.token);
                    } else {
                        handler.writeValue(lhs.token, rhs.value);
                    }
                } else {
                    // lhs is defined in program and stored in symbol stack
                    returned = rhs;
                    getReturnedValue();
                    for (auto it = symbolStack.back().rbegin(); it != symbolStack.back().rend(); it++) {
                        if (auto varIt = it->find(p->name); varIt != it->end()) {
                            varIt->second = returned;
                            returned.type = Value::EMPTY;
                            return;
                        }
                    }
                }
            } else if (isType<MemberAccessExprAST>(v.lhs.get())) {
                callAccept(v.lhs);
                auto lhs = returned;
                callAccept(v.rhs);
                auto rhs = returned;
                if (returned.type == Value::VALUE) {
                    handler.writeValue(lhs.token, rhs.value);
                } else {
                    handler.assign(lhs.token, rhs.token);
                }
                returned.type = Value::EMPTY;
            } else {
                setError("only allow direct or member variable assignment for now");
            }
            returned.type = Value::EMPTY;
        } else if (auto it = normalBinOp.find(v.op); it != normalBinOp.end()) {
            callAccept(v.lhs);
            getReturnedValue();
            auto tmp = returned.value;
            callAccept(v.rhs);
            getReturnedValue();
            returned.value = it->second(tmp, returned.value);
        } else if (auto it = shortCutBinOp.find(v.op); it != shortCutBinOp.end()) {
            callAccept(v.lhs);
            getReturnedValue();
            auto tmp = returned.value;
#ifdef __RULEJIT_INTERPRETER_DEBUG
            bool rhsEvaluate = false;
            double tmp1;
#endif
            if (it->second(tmp, 0) != it->second(tmp, 1)) {
                callAccept(v.rhs);
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
        callAccept(v.rhs);
        getReturnedValue();
        returned.value = unaryFunc[v.op](returned.value);
    }
    VISIT_FUNCTION(BranchExprAST) {
#ifdef __RULEJIT_INTERPRETER_DEBUG
        // TODO: if expr in condition?
        std::cout << "[Check Rule No." << ruleCnt++ << "]" << std::endl;
#endif
        returned.type = Value::EMPTY;
        callAccept(v.condition);
        getReturnedValue();
#ifdef __RULEJIT_INTERPRETER_DEBUG
        std::cout << std::format("Evaluate: {}", returned.value ? "true" : "false") << std::endl;
#endif
        if (returned.value != 0) {
            callAccept(v.trueExpr);
        } else {
            callAccept(v.falseExpr);
        }
    }
    VISIT_FUNCTION(ComplexLiteralExprAST) {
        auto typeName = v.type->toString();
        auto tmp = handler.makeInstance(typeName);
        if (v.type->isArrayType()) {
            for (auto &&[name, value] : v.members) {
                if (name.get()) {
                    error("do not support designated initializer for array");
                }
                callAccept(value);
                switch(returned.type){
                    case Value::VALUE:
                        handler.arrayExtend(tmp, returned.value);
                        break;
                    case Value::TOKEN:
                        handler.arrayExtend(tmp, returned.token);
                        break;
                    default:
                        error("invalid value type");
                }
            }
        } else {
            for (auto &&[name, value] : v.members) {
                auto p = dynamic_cast<LiteralExprAST *>(name.get());
                if (p == nullptr || !(*(p->type) == StringType)) {
                    setError("only allow string literal as key for now");
                }
                callAccept(value);
                auto memberToken = handler.memberAccess(tmp, p->value);
                switch(returned.type){
                    case Value::VALUE:
                        handler.writeValue(memberToken, returned.value);
                        break;
                    case Value::TOKEN:
                        handler.assign(memberToken, returned.token);
                        break;
                    default:
                        error("invalid value type");
                }
            }
        }
        returned.token = tmp;
        returned.type = Value::TOKEN;
    }
    VISIT_FUNCTION(LoopAST) {
        returned.type = Value::EMPTY;
        symbolStack.back().emplace_back();
        callAccept(v.init);
        while (callAccept(v.condition), getReturnedValue(), returned.value != 0) {
            callAccept(v.body);
        }
        symbolStack.back().pop_back();
    }
    VISIT_FUNCTION(BlockExprAST) {
        returned.type = Value::EMPTY;
        symbolStack.back().emplace_back();
        for (auto it = v.exprs.begin(); it != v.exprs.end();) {
            callAccept((*it));
            it++;
        }
        symbolStack.back().pop_back();
    }
    VISIT_FUNCTION(ControlFlowAST) { setError("ControlFlowAST should never be visit directly"); }
    VISIT_FUNCTION(TypeDefAST) { setError("TypeDefAST should never be visit directly"); }
    VISIT_FUNCTION(VarDefAST) {
        returned.type = Value::EMPTY;
        if (auto it = symbolStack.back().back().find(v.name); it != symbolStack.back().back().end()) {
            setError("redefine variable: " + v.name);
        }
        callAccept(v.definedValue);
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

  public:
    std::vector<ExprAST *> currentExpr;
    double getReturned() {
        my_assert(returned.type == Value::VALUE);
        return returned.value;
    }
  private:
    void callAccept(std::unique_ptr<ExprAST> &v) {
        currentExpr.push_back(v.get());
        if (v != nullptr) {
            v->accept(this);
        }
        currentExpr.pop_back();
    }

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
     * @brief check if given type is numerical type
     *
     * @param type string of type name
     * @return true if is numerical type
     */
    bool isNumericalType(const TypeInfo &type) { return type == RealType || type == IntType; }

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
     * @brief context, includes function defines
     *
     */
    ContextStack &context;

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

    SET_ERROR_MEMBER("(Interpreter)Runtime", void)

#ifdef __RULEJIT_INTERPRETER_DEBUG
    size_t ruleCnt;
#endif
};

} // namespace rulejit::cq