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
 * <tr><td>djw</td><td>2023-04-22</td><td>Add template support.</td></tr>
 * <tr><td>djw</td><td>2023-04-23</td><td>Add pure lambda support.</td></tr>
 * </table>
 */
#include "frontend/parser.h"
#include "ast/type.hpp"
#include "parser.h"

// DONE: check if poped token is as expected
// DONE: check assign to assignment
// TODO: lambda // meet fun() -> generate {func unnamedxxx(); unnamedxxx}
// TODO: directly-called lambda
// TODO: type guide to overload return type

namespace {

constexpr inline auto IGNORE_BREAK = rulejit::ExpressionLexer::Guidence::IGNORE_BREAK;

}

namespace rulejit {

// EXPR := UNARYEXPR (op UNARYEXPR)*
std::unique_ptr<ExprAST> ExpressionParser::parseExpr(bool ignoreBreak) {
    std::unique_ptr<ExprAST> ret;
    auto start = lexer->beginPointer();
    if (lexer->tokenType() == TokenType::SYM) {
        if (DEF_KEYWORDS.contains(lexer->top())) {
            // move function def to primary(may have overload `func not (fun func():bool):bool->!func()`)
            ret = parseDef();
        } else if (COMMAND_KEYWORDS.contains(lexer->top())) {
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
        // TODO: op == ":="
        ret = parseBinOpRHS(0, std::move(tmp), ignoreBreak);
    }
    AST2place[ret.get()] = {start, lexer->beginPointer()};
    return ret;
}

std::unique_ptr<ExprAST> ExpressionParser::parseBinOpRHS(Priority priority, std::unique_ptr<ExprAST> lhs,
                                                         bool ignoreBreak) {
    // todo: tuple definition, only aviliable when 'accept tuple'
    while (true) {
        Priority prec;
        auto op = lexer->topCopy();
        if (auto it = BUILDIN_INFIX.find(op); it == BUILDIN_INFIX.end()) {
            if (lexer->tokenType() == TokenType::IDENT) {
                // regard as user defined infix
                prec = USER_DEFINED_PRIORITY;
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
        if (auto it = BUILDIN_INFIX.find(nextOp);
            (it != BUILDIN_INFIX.end() && prec < it->second) ||
            (lexer->tokenType() == TokenType::IDENT && prec < USER_DEFINED_PRIORITY)) {
            rhs = parseBinOpRHS(prec + 1, std::move(rhs));
        } else if (priority == ASSIGN_PRIORITY + 1) {
            if (it != BUILDIN_INFIX.end() && it->second == ASSIGN_PRIORITY) {
                return setError("assign to a assignment is not allowed");
            }
        }
        lhs = std::make_unique<BinOpExprAST>(op, std::move(lhs), std::move(rhs));
    }
}

// cannot return unary function; returned unary function act as normal function
// UNARYEXPR := unary UNARYEXPR | MEMBERACCESS
std::unique_ptr<ExprAST> ExpressionParser::parseUnary() {
    auto start = lexer->beginPointer();
    if (!BUILDIN_UNARY.contains(lexer->topCopy())) {
        return parsePrimary();
    }
    auto op = lexer->popCopy(IGNORE_BREAK);
    auto arg = parseUnary();
    AST2place[arg.get()] = {start, lexer->beginPointer()};
    return std::make_unique<UnaryOpExprAST>(op, std::move(arg));
}

// PRIMARYEXPR :=
//     IDENT                                                                                                  |
//     literal                                                                                                |
//     '(' EXPR ')'                                                                                           |
//     '{' (VARDEF | TYPEDEF | ASSIGNMENT | EXPR ENDLINE)* EXPR? '}'                                          |
//     (IDENT | COMPLEXTYPE | FUNCTYPE | SLICETYPE | ARRAYTYPE) '{' ((IDENT (':' | '='))? EXPR ENDLINE)* '}'  |
//     'if' '(' EXPR ')' EXPR ('else' EXPR)?                                                                  |
//     'while' '(' EXPR ')' EXPR                                                                              |
//     PRIMARYEXPR '.' IDENT | PRIMARYEXPR '(' EXPR ')'
std::unique_ptr<ExprAST> ExpressionParser::parsePrimary() {
    auto startIndex = lexer->beginPointer();
    std::unique_ptr<ExprAST> lhs;
    if (lexer->tokenType() == TokenType::IDENT || lexer->top() == "[") {
        // ComplexLiteral | Ident
        // TODO: lambda, closure
        auto typeInfo = (*lexer) | TypeParser();
        if (lexer->top() != "{") {
            // Ident
            if (!typeInfo.isBaseType()) {
                return setError("type can not act as Expression along: " + typeInfo.toString());
            }
            // TODO: template function
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
                if (lexer->top() == "=") {
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
        if (lexer->top() == ",") {
            // TODO: tuple
        }
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
    } else if (lexer->top() == "|" || lexer->top() == "||") {
        // closure
        std::vector<std::unique_ptr<rulejit::IdentifierExprAST>> params;
        if (lexer->top() == "||") {
            // no args
            lexer->pop(IGNORE_BREAK);
        } else {
            params = parseParamList("|");
            eatBreak();
        }
        bool explicitCapture = false;
        std::vector<std::unique_ptr<IdentifierExprAST>> captures;
        TypeInfo type;
        if (lexer->top() == "->") {
            lexer->pop();
            auto retType = *lexer | TypeParser{};
            type = TypeInfo("func");
            for (auto &&param : params) {
                type.addParamType(*(param->type));
            }
            type.addParamType(retType);
        } else {
            type = AutoType;
        }
        auto ret = parseExpr();
        lhs = std::make_unique<ClosureExprAST>(std::make_unique<TypeInfo>(std::move(type)), explicitCapture,
                                               std::move(captures), std::move(params), std::move(ret));
    } else {
        return setError("unexcepted token: \"" + lexer->topCopy() + "\" in expression");
    }

    while (lexer->tokenType() == TokenType::SYM) {
        // FuncCall | MemberAccess
        if (lexer->top() == ".") {
            // member access
            lexer->pop(IGNORE_BREAK);
            if (lexer->tokenType() != TokenType::IDENT && !KEYWORDS.contains(lexer->top())) {
                return setError("expected member name after \".\", found: " + lexer->topCopy());
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
        AST2place.emplace(lhs.get(), std::string_view{startIndex, lexer->beginPointer()});
    }
    AST2place.emplace(lhs.get(), std::string_view{startIndex, lexer->beginPointer()});
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
    if (lexer->top() == "var" || lexer->top() == "const") {
        // var def
        VarDefAST::VarDefType varDefType =
            lexer->top() == "var" ? VarDefAST::VarDefType::NORMAL : VarDefAST::VarDefType::CONSTANT;
        lexer->pop(IGNORE_BREAK);
        if (lexer->tokenType() != TokenType::IDENT) {
            return setError("expected ident as var name, found: " + lexer->topCopy());
        }
        auto indent = lexer->popCopy(IGNORE_BREAK);
        std::unique_ptr<TypeInfo> type;
        if (lexer->top() == "=") {
            lexer->pop(IGNORE_BREAK);
            type = std::make_unique<TypeInfo>(AutoType);
        } else {
            if (lexer->top() == "auto") {
                return setError(
                    "donot support user-defined auto type, use \"=\" directly to define variables instead.");
            }
            type = std::make_unique<TypeInfo>((*lexer) | TypeParser());
            // un-initialized var
            if (lexer->tokenType() == TokenType::ENDLINE) {
                auto defined = std::make_unique<ComplexLiteralExprAST>(
                    std::make_unique<TypeInfo>(*type),
                    std::vector<std::tuple<std::unique_ptr<ExprAST>, std::unique_ptr<ExprAST>>>{});
                if (varDefType == VarDefAST::VarDefType::CONSTANT) {
                    return setError("const var must be explicitly initialized");
                }
                return std::make_unique<VarDefAST>(indent, std::move(type), std::move(defined));
            }
            auto tmp = lexer->pop(IGNORE_BREAK);
            if (tmp != "=") {
                return setError("expected \"=\" in var definition, found: " + std::string(tmp));
            }
        }
        return std::make_unique<VarDefAST>(indent, std::move(type), parseExpr(), varDefType);
    } else if (lexer->top() == "func") {
        // func def
        // TODO: operator. operator() operator[]
        lexer->pop(IGNORE_BREAK);

        std::vector<std::unique_ptr<rulejit::IdentifierExprAST>> params;
        std::string funcName;
        TypeInfo funcType;
        FunctionDefAST::FuncDefType funcDefType;

        std::vector<std::string> tparams;
        bool isTemplate = false;
        if (lexer->top() == "<" && std::get<1>(lexer->foresee(1)) != "(") {
            // extinguish reload of "<" and template function
            isTemplate = true;
            while (lexer->top() != ">") {
                lexer->pop(IGNORE_BREAK);
                if (lexer->tokenType() != TokenType::IDENT) {
                    return setError("expected ident in template parameter list, found: " + lexer->topCopy());
                }
                tparams.push_back(lexer->popCopy());
                if (lexer->top() != "," && lexer->top() != ">") {
                    return setError("expected \",\" in template parameter list, found: " + lexer->topCopy());
                }
            }
            lexer->pop(IGNORE_BREAK);
        }

        parseFuncDef(params, funcType, funcName, funcDefType);

        eatBreak();

        // parse returned value

        // // TODO: named constructor 'func getStr():(ret string){}'? maybe need default construct var def
        // if (lexer->top() == "->") {
        //     lexer->pop(IGNORE_BREAK);
        // } else if (lexer->top() == "{") {
        //     // do nothing
        // } else {
        //     return setError("expect \"->\" or \"{\" to indicate return value, found: " + lexer->topCopy());
        // }

        std::unique_ptr<ExprAST> returnValue = parseExpr();

        auto funcDefAST = std::make_unique<FunctionDefAST>(std::move(funcName), std::make_unique<TypeInfo>(funcType),
                                                           std::move(params), std::move(returnValue), funcDefType);

        if (isTemplate) {
            return std::make_unique<TemplateDefAST>(std::move(tparams), std::move(funcDefAST));
        } else {
            return std::move(funcDefAST);
        }
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
        if (lexer->top() == "=") {
            lexer->pop(IGNORE_BREAK);
            setError("donot support type alias");
            typeDefType = TypeDefAST::TypeDefType::ALIAS;
        } else if (lexer->top() == "struct") {
            lexer->pop(IGNORE_BREAK);
            typeDefType = TypeDefAST::TypeDefType::NORMAL;
        } else {
            return setError("expected \"=\" or \"struct\" or \"class\" after type name, found: " + lexer->topCopy());
        }
        std::vector<std::tuple<std::string, TypeInfo>> members;
        if (lexer->pop(IGNORE_BREAK) != "{") {
            return setError("expect \"{\", found: " + lexer->topCopy());
        }
        while (lexer->top() != "}") {
            if (lexer->tokenType() != TokenType::IDENT && !KEYWORDS.contains(lexer->top())) {
                return setError("expect member name, found: " + lexer->topCopy());
            }
            auto memberName = lexer->popCopy(IGNORE_BREAK);
            auto memberType = *lexer | TypeParser();
            members.emplace_back(memberName, memberType);
            if (lexer->tokenType() != TokenType::ENDLINE && lexer->top() != "}") {
                return setError("expect ENDLINE or \"}\", found: " + lexer->topCopy());
            }
            if (lexer->tokenType() == TokenType::ENDLINE) {
                lexer->pop(IGNORE_BREAK);
            }
        }
        lexer->pop();
        return std::make_unique<TypeDefAST>(std::move(name), std::move(members), typeDefType);
    } else {
        return setError("expect definition keywords like \"var\", \"func\" or \"type\", found: " + lexer->topCopy());
    }
}

void rulejit::ExpressionParser::parseFuncDef(std::vector<std::unique_ptr<rulejit::IdentifierExprAST>> &params,
                                             TypeInfo &funcType, std::string &funcName,
                                             FunctionDefAST::FuncDefType &funcDefType) {
    funcDefType = FunctionDefAST::FuncDefType::NORMAL;
    TypeInfo returnType;

    // parse function definition
    if (lexer->top() == "(") {
        // member
        params = parseParamList();
        eatBreak();
        if (lexer->tokenType() == TokenType::IDENT) {
            funcName = lexer->popCopy(IGNORE_BREAK);
            if (lexer->top() != "(") {
                setError("expected \"(\" after member function name in member function def, found: " +
                         lexer->topCopy());
            }
            if (params.size() != 1) {
                setError("member function must contain 1 receiver param, given: " + std::to_string(params.size()));
            }
            auto tmp = parseParamList();
            eatBreak();
            for (auto &x : tmp) {
                params.push_back(std::move(x));
            }
            funcDefType = FunctionDefAST::FuncDefType::MEMBER;
        } else {
            setError("expected ident as function name in member function def, found: " + lexer->topCopy());
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
            if (RESERVED_NOT_RELOADABLE_SYMBOL.contains(funcName)) {
                setError("unsupport operator overload: " + funcName);
            }
            if (lexer->top() == "infix") {
                setError("operator overload is automatically infix or unary, donot use \"infix\" keyword");
            }
        } else {
            setError("expected ident or symbol as function name, found: " + lexer->topCopy());
        }
        params = parseParamList();
        eatBreak();
    }

    // TODO: auto infer return type
    // parse returned type
    if (lexer->top() == "->") {
        // returned function
        // TODO: remove "->"? donot,
        //                   1. hard to parse user-defined infix operator
        //                   2. hard to distinguish lambda from member function
        lexer->pop(IGNORE_BREAK);
        returnType = *lexer | TypeParser();
        eatBreak();
    } else {
        // no return function
        returnType = NoInstanceType;
    }

    if (funcDefType == FunctionDefAST::FuncDefType::SYMBOLIC) {
        if (params.size() == 2) {
            // infix
        } else if (params.size() == 1) {
            if (!BUILDIN_UNARY.contains(funcName)) {
                setError("unsupport unary operator overload: " + funcName);
            }
        } else {
            setError("only allow unary or infix operator overload");
        }
    }

    funcType = TypeInfo("func");
    for (auto &&param : params) {
        funcType.addParamType(*(param->type));
    }
    funcType.addParamType(returnType);
}

std::vector<std::unique_ptr<IdentifierExprAST>> ExpressionParser::parseParamList(const std::string &end) {
    std::vector<std::unique_ptr<IdentifierExprAST>> ret;
    lexer->pop(IGNORE_BREAK);
    while (lexer->top() != end) {
        // TODO: unnamed param
        if (lexer->tokenType() != TokenType::IDENT) {
            setError("except identifier in param list, found: " + lexer->topCopy());
        }
        auto ident = lexer->pop(IGNORE_BREAK);
        ret.push_back(
            std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>((*lexer) | TypeParser()), std::move(ident)));
        eatBreak();
        if (lexer->top() != "," && lexer->top() != end) {
            setError("expect \",\" after single param, found: " + lexer->topCopy());
        }
        if (lexer->top() == ",") {
            lexer->pop(IGNORE_BREAK);
            if (lexer->top() == end) {
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

            std::vector<std::unique_ptr<rulejit::IdentifierExprAST>> params;
            std::string funcName;
            TypeInfo funcType;
            FunctionDefAST::FuncDefType funcDefType;

            parseFuncDef(params, funcType, funcName, funcDefType);

            if (funcDefType != FunctionDefAST::FuncDefType::NORMAL) {
                return setError("only support extern normal func command");
            }

            return std::make_unique<SymbolDefAST>(funcName, SymbolDefAST::SymbolCommandType::EXTERN,
                                                  std::make_unique<TypeInfo>(funcType));
        } else {
            return setError("only support extern func command");
        }
    }
    return setError("only support extern func command");
}

std::unique_ptr<ExprAST> ExpressionParser::parseTopLevel() { return setError("top level not supported now"); }

} // namespace rulejit