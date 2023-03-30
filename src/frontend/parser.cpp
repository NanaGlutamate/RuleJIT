/**
 * @file parser.cpp
 * @author djw
 * @brief FrontEnd/Parser
 * @date 2023-03-28
 *
 * @details Parser
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-03-30</td><td>Rewrite func-def parsering.</td></tr>
 * </table>
 */
#include "frontend/parser.h"
#include "ast/type.hpp"

// DONE: check if poped token is as expected
// DONE: check assign to assignment
// TODO: lambda // meet fun() -> gen {func unnamedFa581(); unnamedFa581}
// TODO: directly-called lambda

namespace {

constexpr inline auto IGNORE_BREAK = rulejit::ExpressionLexer::Guidence::IGNORE_BREAK;

}

namespace rulejit {

// EXPR := UNARYEXPR (op UNARYEXPR)*
std::unique_ptr<ExprAST> ExpressionParser::parseExpr(bool ignoreBreak, bool allowTuple) {
    std::unique_ptr<ExprAST> ret;
    auto start = lexer->beginIndex();
    if (lexer->tokenType() == TokenType::SYM) {
        if (defKeyWords.contains(lexer->top())) {
            // move function def to primary(may have overload `func not (fun func():bool):bool->!func()`)
            ret = parseDef();
        } else if (commandKeyWords.contains(lexer->top())) {
            ret = parseCommand();
        } else {
            auto tmp = parseUnary();
            if (ignoreBreak) {
                eatBreak();
            }
            ret = parseBinOpRHS(0, std::move(tmp), ignoreBreak);
        }
    } else {
        auto tmp = parseUnary();
        if (ignoreBreak) {
            eatBreak();
        }
        ret = parseBinOpRHS(0, std::move(tmp), ignoreBreak);
    }
    AST2place[ret.get()] = start;
    return ret;
}

std::unique_ptr<ExprAST> ExpressionParser::parseBinOpRHS(Priority priority, std::unique_ptr<ExprAST> lhs,
                                                         bool ignoreBreak, bool allowTuple) {
    // todo: tuple definition, only aviliable when 'accept tuple'
    while (true) {
        Priority prec;
        auto op = lexer->topCopy();
        if (auto it = reloadableBuildInInfix.find(op); it == reloadableBuildInInfix.end()) {
            if (lexer->tokenType() == TokenType::IDENT) {
                // regard as user defined infix
                prec = UserDefinedPriority;
            } else {
                return lhs;
            }
        } else {
            prec = it->second;
        }
        if (prec < priority) {
            return lhs;
        }
        lexer->pop(IGNORE_BREAK);
        auto rhs = parseUnary();
        if (ignoreBreak && lexer->top() == "\n") {
            lexer->pop(IGNORE_BREAK);
        }
        std::string nextOp = lexer->topCopy();
        if (auto it = reloadableBuildInInfix.find(nextOp);
            (it != reloadableBuildInInfix.end() && prec < it->second) ||
            (lexer->tokenType() == TokenType::IDENT && prec < UserDefinedPriority)) {
            rhs = parseBinOpRHS(prec + 1, std::move(rhs));
        } else if (priority == AssignPriority + 1) {
            if (it != reloadableBuildInInfix.end() && it->second == AssignPriority) {
                return setError("assign to a assignment is not allowed");
            }
        }
        lhs = std::make_unique<BinOpExprAST>(op, std::move(lhs), std::move(rhs));
    }
}

// cannot return unary function; returned unary function act as normal function
// UNARYEXPR := unary UNARYEXPR | MEMBERACCESS
std::unique_ptr<ExprAST> ExpressionParser::parseUnary() {
    if (!reloadableBuildInUnary.contains(lexer->topCopy())) {
        return parsePrimary();
    }
    auto op = lexer->popCopy(IGNORE_BREAK);
    auto arg = parseUnary();
    return std::make_unique<UnaryOpExprAST>(op, std::move(arg));
}

// PRIMARYEXPR :=
//     IDENT                                                                                          |
//     literal                                                                                        |
//     '(' EXPR ')'                                                                                   |
//     '{' (VARDEF | TYPEDEF | ASSIGNMENT | EXPR ENDLINE)* EXPR? '}'                                  |
//     (IDENT | COMPLEXTYPE | FUNCTYPE | SLICETYPE | ARRAYTYPE) '{' ((IDENT ':')? EXPR ENDLINE)* '}'  |
//     'if' '(' EXPR ')' EXPR ('else' EXPR)?                                                          |
//     'while' '(' EXPR ')' EXPR                                                                      |
//     PRIMARYEXPR '.' IDENT | PRIMARYEXPR '(' EXPR ')'
std::unique_ptr<ExprAST> ExpressionParser::parsePrimary() {
    auto startIndex = lexer->beginIndex();
    std::unique_ptr<ExprAST> lhs;
    // TODO: move to language.hpp
    if (lexer->tokenType() == TokenType::IDENT || lexer->top() == "[") {
        // ComplexLiteral | Ident
        // TODO: lambda, closure
        auto typeInfo = (*lexer) | TypeParser();
        if (lexer->top() != "{") {
            // Ident
            if (!typeInfo.isBaseType()) {
                return setError("type can not act as Expression along: " + typeInfo.toString());
            }
            lhs = std::make_unique<IdentifierExprAST>(typeInfo.getBaseTypeString());
        } else {
            // ComplexLiteral
            lexer->pop(IGNORE_BREAK);
            std::vector<std::tuple<std::unique_ptr<ExprAST>, std::unique_ptr<ExprAST>>> members;
            bool designated = lexer->top() == ".";
            while (lexer->top() != "}") {
                std::unique_ptr<rulejit::ExprAST> key;
                if (lexer->top() == ".") {
                    lexer->pop(IGNORE_BREAK);
                    if (lexer->tokenType() != TokenType::IDENT) {
                        if (lexer->tokenType() == TokenType::INT || lexer->tokenType() == TokenType::REAL ||
                            lexer->tokenType() == TokenType::STRING) {
                            return setError("expect indentifer as key of designated initializer, found: " +
                                            lexer->topCopy() + "; literal as key do not need '.' before it");
                        }
                        return setError("expect indentifer as key of designated initializer, found: " +
                                        lexer->topCopy());
                    }
                    auto ident = lexer->popCopy(IGNORE_BREAK);
                    key = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(StringType), std::move(ident));
                } else {
                    if (designated) {
                        return setError("designated initializer must have key");
                    }
                    key = parseExpr(true);
                }
                if (lexer->top() == ":" || lexer->top() == "=") {
                    lexer->pop(IGNORE_BREAK);
                    auto value = parseExpr(true);
                    // eatBreak();
                    if (!designated) {
                        return setError("non-designated initializer must not have key");
                    }
                    members.push_back(std::make_tuple(std::move(key), std::move(value)));
                    if (lexer->top() == ",") {
                        lexer->pop(IGNORE_BREAK);
                    }
                } else if (lexer->top() == ",") {
                    if (designated) {
                        return setError("designated initializer must have value");
                    }
                    lexer->pop(IGNORE_BREAK);
                    members.push_back(std::make_tuple(nullptr, std::move(key)));
                } else if (lexer->top() != "}") {
                    return setError("invalid symbol found in complex literal: " + lexer->topCopy());
                } else {
                    if (designated) {
                        return setError("designated initializer must have value");
                    }
                    members.push_back(std::make_tuple(nullptr, std::move(key)));
                }
            }
            lexer->pop();
            // TODO: debug
            lhs = std::make_unique<ComplexLiteralExprAST>(std::make_unique<TypeInfo>(typeInfo), std::move(members));
        }
    } else if (lexer->tokenType() == TokenType::INT || lexer->tokenType() == TokenType::REAL) {
        // Literal
        lhs = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(RealType), lexer->popCopy());
    } else if (lexer->tokenType() == TokenType::STRING) {
        // Literal
        std::string tmp;
        (*lexer) >> tmp;
        lhs = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(StringType), std::move(tmp));
    } else if (lexer->top() == "(") {
        // Parent | tokenized symbol(not support cause may caused complex problem)
        lexer->pop(IGNORE_BREAK);
        lhs = parseExpr(true);
        eatBreak();
        if (lexer->pop() != ")") {
            return setError("mismatch \")\"");
        }
    } else if (lexer->top() == "{") {
        // Block
        lhs = parseBlock();
    } else if (lexer->top() == "if") {
        // branch
        lexer->pop(IGNORE_BREAK);
        if (lexer->pop(IGNORE_BREAK) != "(") {
            return setError("expected \"(\" after \"if\", found: " + lexer->topCopy());
        }
        auto cond = parseExpr(true);
        eatBreak();
        if (lexer->pop(IGNORE_BREAK) != ")") {
            return setError("expected \")\" after \"if\", found: " + lexer->topCopy());
        }
        auto trueExpr = parseExpr();
        auto state = lexer->getState();
        eatBreak();
        std::unique_ptr<rulejit::ExprAST> falseExpr;
        if (lexer->top() == "else") {
            lexer->pop(IGNORE_BREAK);
            falseExpr = parseExpr();
        } else {
            lexer->loadState(state);
            falseExpr = nop();
        }
        lhs = std::make_unique<BranchExprAST>(std::move(cond), std::move(trueExpr), std::move(falseExpr));
    } else if (lexer->top() == "while") {
        // while
        lexer->pop(IGNORE_BREAK);
        if (lexer->pop(IGNORE_BREAK) != "(") {
            return setError("expected \"(\" after \"while\", found: " + lexer->topCopy());
        }
        auto cond = parseExpr(true);
        eatBreak();
        if (lexer->pop(IGNORE_BREAK) != ")") {
            return setError("expected \")\" after \"while\", found: " + lexer->topCopy());
        }
        std::string label;
        if (lexer->top() == "@") {
            // labeled
            lexer->pop(IGNORE_BREAK);
            if (lexer->tokenType() != TokenType::IDENT) {
                return setError("expected ident after \"@\", found: " + lexer->topCopy());
            }
            label = lexer->pop(IGNORE_BREAK);
        }
        auto expr = parseExpr();
        lhs = std::make_unique<LoopAST>(std::move(label), nop(), std::move(cond), std::move(expr));
    } else {
        return setError("unexcepted token: \"" + lexer->topCopy() + "\" in expression");
    }
    while (lexer->tokenType() == TokenType::SYM) {
        // FuncCall | MemberAccess
        if (lexer->top() == ".") {
            // member access
            lexer->pop(IGNORE_BREAK);
            if (lexer->tokenType() != TokenType::IDENT) {
                return setError("expected ident after \".\", found: " + lexer->topCopy());
            }
            std::string ident = lexer->popCopy();
            lhs = std::make_unique<MemberAccessExprAST>(
                std::move(lhs), std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(StringType), ident));
        } else if (lexer->top() == "(") {
            // function call
            lexer->pop(IGNORE_BREAK);
            std::vector<std::unique_ptr<ExprAST>> args;
            while (lexer->top() != ")") {
                auto arg = parseExpr(true);
                eatBreak();
                args.push_back(std::move(arg));
                if (lexer->top() != ",") {
                    break;
                } else {
                    lexer->pop(IGNORE_BREAK);
                }
            }
            if (lexer->pop() != ")") {
                return setError("expected \")\" in function call expression, found: " + lexer->topCopy());
            }
            lhs = std::make_unique<FunctionCallExprAST>(std::move(lhs), std::move(args));
        } else if (lexer->top() == "[") {
            // array access
            lexer->pop(IGNORE_BREAK);
            auto index = parseExpr(true);
            if (lexer->pop() != "]") {
                return setError("expected \"]\" in array access expression, found: " + lexer->topCopy());
            }
            lhs = std::make_unique<MemberAccessExprAST>(std::move(lhs), std::move(index));
        } else {
            break;
        }
    }
    AST2place.emplace(lhs.get(), startIndex);
    return lhs;
}

std::unique_ptr<ExprAST> ExpressionParser::parseBlock() {
    // Block
    lexer->pop(IGNORE_BREAK);
    std::vector<std::unique_ptr<ExprAST>> exprs;
    while (lexer->top() != "}") {
        exprs.push_back(parseExpr());

        if (lexer->tokenType() == TokenType::ENDLINE) {
            lexer->pop(IGNORE_BREAK);
        } else if (lexer->top() != "}") {
            return setError("mismatched \"}\", found: " + lexer->topCopy());
        }
    }
    if (lexer->pop() != "}") {
        return setError("expected \"}\" in block expression, found: " + lexer->topCopy());
    }
    if (exprs.empty()) {
        return nop();
    }
    return std::make_unique<BlockExprAST>(std::move(exprs));
}

std::unique_ptr<ExprAST> ExpressionParser::parseDef() {
    if (lexer->top() == "var") {
        // var def
        lexer->pop(IGNORE_BREAK);
        if (lexer->tokenType() != TokenType::IDENT) {
            return setError("expected ident as var name, found: " + lexer->topCopy());
        }
        auto indent = lexer->popCopy(IGNORE_BREAK);
        std::unique_ptr<TypeInfo> type;
        if (lexer->top() == ":=") {
            lexer->pop(IGNORE_BREAK);
            type = std::make_unique<TypeInfo>(AutoType);
        } else {
            if (lexer->top() == "auto") {
                return setError("donot support user-defined auto type, use \":=\" to define variables instead.");
            }
            type = std::make_unique<TypeInfo>((*lexer) | TypeParser());
            eatBreak();
            auto tmp = lexer->pop(IGNORE_BREAK);
            if (tmp != "=") {
                return setError("expected \"=\" in var definition, found: " + std::string(tmp));
            }
        }
        return std::make_unique<VarDefAST>(indent, std::move(type), parseExpr());
    } else if (lexer->top() == "func") {
        // TODO: operator.
        // func def
        lexer->pop(IGNORE_BREAK);

        FunctionDefAST::FuncDefType funcDefType = FunctionDefAST::FuncDefType::NORMAL;
        std::vector<std::unique_ptr<rulejit::IdentifierExprAST>> params;
        TypeInfo returnType;
        std::unique_ptr<ExprAST> returnValue;
        std::string funcName;

        // parse function definition
        if (lexer->top() == "(") {
            // lambda | member
            params = parseParamList();
            eatBreak();
            if (lexer->tokenType() == TokenType::IDENT) {
                // member
                funcName = lexer->popCopy(IGNORE_BREAK);
                if (lexer->top() != "(") {
                    return setError("expected \"(\" after member function name in member function def, found: " +
                                    lexer->topCopy());
                }
                if (params.size() != 1) {
                    return setError("member function must contain 1 receiver param, given: " +
                                    std::to_string(params.size()));
                }
                auto tmp = parseParamList();
                eatBreak();
                for (auto &x : tmp) {
                    params.push_back(std::move(x));
                }
                funcDefType = FunctionDefAST::FuncDefType::MEMBER;
            } else {
                // lambda
                funcDefType = FunctionDefAST::FuncDefType::LAMBDA;
            }
        } else {
            // symbolic | normal
            if (lexer->tokenType() == TokenType::IDENT) {
                funcName = lexer->popCopy(IGNORE_BREAK);
                if (lexer->top() == "infix") {
                    // symbolic (infix)
                    lexer->pop(IGNORE_BREAK);
                    funcDefType = FunctionDefAST::FuncDefType::SYMBOLIC;
                }
                // normal
                // do nothing
            } else if (lexer->tokenType() == TokenType::SYM) {
                // symbolic (operator overload)
                funcName = lexer->popCopy(IGNORE_BREAK);
                funcDefType = FunctionDefAST::FuncDefType::SYMBOLIC;
                if (!reloadableBuildInInfix.contains(funcName) && !reloadableBuildInUnary.contains(funcName)) {
                    return setError("unknown operator overload: " + funcName);
                }
            } else {
                return setError("expected ident or symbol as function name, found: " + lexer->topCopy());
            }
            params = parseParamList();
            eatBreak();
        }

        // parse returned type
        if (lexer->top() == ":") {
            // returned function
            // TODO: remove ":"? donot, 
            //                   1. hard to parse user-defined infix operator
            //                   2. hard to distinguish lambda from member function
            lexer->pop(IGNORE_BREAK);
            returnType = *lexer | TypeParser();
            eatBreak();
        } else {
            // no return function
            returnType = NoInstanceType;
        }

        // parse returned value
        // TODO: named constructor 'func getStr():(ret string := ""){}'? maybe need default construct var def
        if (lexer->top() == "->") {
            lexer->pop(IGNORE_BREAK);
        } else if (lexer->top() == "{") {
            // do nothing
        } else {
            return setError("expect \"->\" or \"{\" to indicate return value, found: " + lexer->topCopy());
        }

        returnValue = parseExpr();

        TypeInfo funcType("func");
        for(auto&& p : params){
            funcType.addParamType(*(p->type));
        }
        funcType.addParamType(returnType);

        return std::make_unique<FunctionDefAST>(std::move(funcName), std::make_unique<TypeInfo>(funcType), std::move(params),
                                                std::move(returnValue), funcDefType);
    } else if (lexer->top() == "type") {
        // type def
        lexer->pop(IGNORE_BREAK);
        if (lexer->tokenType() != TokenType::IDENT) {
            return setError("expected ident as type name, found: " + lexer->topCopy());
        }
        if (lexer->tokenType() != TokenType::IDENT) {
            return setError("expected ident as type name, found: " + lexer->topCopy());
        }
        auto name = lexer->popCopy(IGNORE_BREAK);
        TypeDefAST::TypeDefType typeDefType = TypeDefAST::TypeDefType::NORMAL;
        if (lexer->topChar() == '=') {
            lexer->pop(IGNORE_BREAK);
            typeDefType = TypeDefAST::TypeDefType::ALIAS;
        }
        auto type = (*lexer) | TypeParser();
        return std::make_unique<TypeDefAST>(std::move(name), std::make_unique<TypeInfo>(type), typeDefType);
    } else {
        return setError("expect definition keywords like \"var\", \"func\" or \"type\", found: " + lexer->topCopy());
    }
}

std::vector<std::unique_ptr<IdentifierExprAST>> ExpressionParser::parseParamList() {
    std::vector<std::unique_ptr<IdentifierExprAST>> ret;
    lexer->pop(IGNORE_BREAK);
    while (lexer->top() != ")") {
        // TODO: unnamed param
        if (lexer->tokenType() != TokenType::IDENT) {
            setError("except identifier in param list, found: " + lexer->topCopy());
        }
        auto ident = lexer->pop(IGNORE_BREAK);
        ret.push_back(
            std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>((*lexer) | TypeParser()), std::move(ident)));
        eatBreak();
        if (lexer->top() != "," && lexer->top() != ")") {
            setError("expect \",\" after single param, found: " + lexer->topCopy());
        }
        if (lexer->top() == ",") {
            lexer->pop(IGNORE_BREAK);
            if (lexer->top() == ")") {
                setError("param list should not end with \",\"");
            }
        }
    }
    lexer->pop();
    return ret;
}

std::unique_ptr<ExprAST> ExpressionParser::parseCommand() {
    if (lexer->top() == "extern") {
        lexer->pop(IGNORE_BREAK);
        if (lexer->top() == "func") {
            lexer->pop(IGNORE_BREAK);
            auto nameType = lexer->tokenType();
            if (nameType != TokenType::IDENT) {
                return setError("expected ident as extern function name, found: " + lexer->topCopy());
            }
            std::string name = lexer->popCopy(IGNORE_BREAK);
            TypeInfo type{"func"};
            auto params = parseParamList();
            for (auto &param : params) {
                type.addParamType(*(param->type));
            }
            if (lexer->top() == ":") {
                // return type
                // TODO: return value func(i i32):(i i32){}
                lexer->pop(IGNORE_BREAK);
                auto returnType = (*lexer) | TypeParser();
                if (!returnType.isValid()) {
                    return setError("expected return type in function definition after \":\", found: " +
                                    lexer->topCopy());
                }
                type.addParamType(returnType);
            } else {
                type.addParamType(NoInstanceType);
            }
            return std::make_unique<SymbolDefAST>(name, SymbolDefAST::SymbolCommandType::EXTERN,
                                                  std::make_unique<TypeInfo>(type));
        } else {
            return setError("only support extern func command");
        }
    }
    return setError("only support extern func command");
}

std::unique_ptr<ExprAST> ExpressionParser::parseTopLevel() { return setError("top level not supported now"); }

} // namespace rulejit