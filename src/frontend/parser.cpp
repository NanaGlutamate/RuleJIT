#include "frontend/parser.h"
#include "ast/type.hpp"

namespace rulejit {

// EXPR := UNARYEXPR (op UNARYEXPR)*
std::unique_ptr<ExprAST> ExpressionParser::parseExpr(bool ignoreBreak) {
    auto lhs = parseUnary();
    if (!lhs) {
        return nullptr;
    }
    return parseBinOpRHS(0, std::move(lhs), ignoreBreak);
}

std::unique_ptr<ExprAST> ExpressionParser::parseBinOpRHS(Priority priority, std::unique_ptr<ExprAST> lhs,
                                                         bool ignoreBreak) {
    while (true) {
        Priority prec;
        auto op = lexer->topCopy();
        if (auto it = reloadableBuildInInfix.find(op); it != reloadableBuildInInfix.end() && prec >= priority) {
            prec = it->second;
        } else {
            return lhs;
        }
        lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        auto rhs = parseUnary();
        if (!rhs) {
            return nullptr;
        }
        if (ignoreBreak && lexer->top() == "\n") {
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        }
        std::string nextOp = lexer->topCopy();
        if (auto it = reloadableBuildInInfix.find(nextOp); it != reloadableBuildInInfix.end()) {
            if (prec < it->second) {
                rhs = parseBinOpRHS(prec + 1, std::move(rhs));
                if (!rhs) {
                    return nullptr;
                }
            }
        }
        std::vector<std::unique_ptr<ExprAST>> args;
        args.push_back(std::move(lhs));
        args.push_back(std::move(rhs));
        lhs = std::make_unique<FunctionCallExprAST>(std::make_unique<IdentifierExprAST>(op), std::move(args));
    }
}
// cannot return unary function; returned unary function act as normal function
// UNARYEXPR := unary UNARYEXPR | MEMBERACCESS
std::unique_ptr<ExprAST> ExpressionParser::parseUnary() {
    if (reloadableBuildInUnary.contains(lexer->topCopy())) {
        auto op = lexer->popCopy(ExpressionLexer::Guidence::IGNORE_BREAK);
        auto rhs = parseUnary();
        if (!rhs) {
            return nullptr;
        }
        std::vector<std::unique_ptr<ExprAST>> args;
        args.push_back(std::move(rhs));
        return std::make_unique<FunctionCallExprAST>(std::make_unique<IdentifierExprAST>(op), std::move(args));
    } else {
        return parsePrimary();
    }
}
// PRIMARYEXPR :=
//     IDENT                                                                                          |
//     literal                                                                                        |
//     '(' EXPR ')'                                                                                   |
//     '{' (VARDEF | TYPEDEF | ASSIGNMENT | EXPR ENDLINE)* EXPR? '}'                                  |
//     (IDENT | COMPLEXTYPE | FUNCTYPE | SLICETYPE | ARRAYTYPE) '{' ((IDENT ':')? EXPR ENDLINE)* '}'  |
//     'if' '(' EXPR ')' EXPR ('else' EXPR)?                                                          |
//     'while' '(' EXPR ')' EXPR                                                                      | // return last
//     PRIMARYEXPR '.' IDENT | PRIMARYEXPR '(' EXPR ')'
std::unique_ptr<ExprAST> ExpressionParser::parsePrimary() {
    std::unique_ptr<ExprAST> lhs;
    if (lexer->top() == "(") {
        // Parent
        lhs = parseExpr(true);
        if (lexer->pop() != ")") {
            return setError("mismatch \")\"");
        }
    } else if (lexer->top() == "{") {
        // Block
        lhs = parseBlock();
    } else if (lexer->tokenType() == TokenType::INT || lexer->tokenType() == TokenType::REAL) {
        // Literal
        lhs = std::make_unique<LiteralExprAST>(
            std::make_unique<TypeInfo>(std::vector<std::string>{std::string(typeident::RealType)}), lexer->popCopy());
    } else if (lexer->tokenType() == TokenType::STRING) {
        // Literal
        std::string tmp;
        (*lexer) >> tmp;
        lhs = std::make_unique<LiteralExprAST>(
            std::make_unique<TypeInfo>(std::vector<std::string>{std::string(typeident::StringType)}), std::move(tmp));
    } else if (lexer->top() == "if") {
        // TODO: branch
    } else if (lexer->top() == "while") {
        // TODO: while
    } else if (lexer->tokenType() == TokenType::IDENT || lexer->top() == "[") {
        // ComplexLiteral | Ident
        // TODO: lambda
        auto typeInfo = (*lexer) | TypeParser();
        if (lexer->top() != "{") {
            // Ident
            if (!typeInfo.isSingleToken()) {
                return setError("type can not act as Expression along: "+typeInfo.toString());
            }
            lhs = std::make_unique<IdentifierExprAST>(typeInfo.idents[0]);
        } else {
            // ComplexLiteral
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            std::vector<std::tuple<std::string, std::unique_ptr<ExprAST>>> members;
            while (lexer->top() != "}") {
                if (lexer->tokenType() == TokenType::IDENT) {
                    auto state = lexer->getState();
                    std::string key;
                    key = lexer->popCopy(ExpressionLexer::Guidence::IGNORE_BREAK);
                    if (lexer->top() == ":") {
                        auto var = parseExpr(true);
                        if (!var) {
                            return nullptr;
                        }
                        eatBreak();
                        members.push_back({std::move(key), std::move(var)});
                        continue;
                    }
                    lexer->loadState(state);
                }
                auto var = parseExpr(true);
                if (!var) {
                    return nullptr;
                }
                members.push_back({"", std::move(var)});
                if (lexer->top() != ",") {
                    break;
                } else {
                    lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
                }
            }
            if (lexer->pop() != "}") {
                return setError("expected \"}\" in complex type literal expression, found: "+lexer->topCopy());
            }
            lexer->pop();
            lhs = std::make_unique<ComplexLiteralExprAST>(std::make_unique<TypeInfo>(typeInfo), std::move(members));
        }
    } else {
        return setError("unexcepted token: \"" + lexer->topCopy() + "\" in expression");
    }
    while (true) {
        // FuncCall | MemberAccess
        if (lexer->top() == ".") {
            // member access
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            if (lexer->tokenType() != TokenType::IDENT) {
                return setError("expected ident after \".\", found: "+lexer->topCopy());
            }
            std::string ident = lexer->popCopy();
            lhs = std::make_unique<MemberAccessExprAST>(std::move(lhs), ident);
        } else if (lexer->top() == "(") {
            // function call
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            std::vector<std::unique_ptr<ExprAST>> args;
            while (lexer->top() != ")") {
                auto arg = parseExpr(true);
                if (!arg) {
                    return nullptr;
                }
                eatBreak();
                args.push_back(std::move(arg));
                if (lexer->top() != ",") {
                    break;
                } else {
                    lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
                }
            }
            if (lexer->pop() != ")") {
                return setError("expected \")\" in function call expression, found: "+lexer->topCopy());
            }
            lhs = std::make_unique<FunctionCallExprAST>(std::move(lhs), std::move(args));
        } else if (lexer->top() == "[") {
            // array access
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            auto index = parseExpr(true);
            if (!index) {
                return nullptr;
            }
            if (lexer->pop() != "]") {
                return setError("expected \"]\" in array access expression, found: "+lexer->topCopy());
            }
            lhs = std::make_unique<ArrayIndexExprAST>(std::move(lhs), std::move(index));
        } else {
            break;
        }
    }
    return lhs;
}

std::unique_ptr<BlockExprAST> ExpressionParser::parseBlock() {
    // Block
    lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
    std::vector<std::unique_ptr<AST>> statements;
    std::unique_ptr<AST> last;
    while (lexer->top() != "}") {
        if (!last) {
            statements.push_back(std::move(last));
        }
        if (defKeyWords.contains(lexer->top())) {
            // defs
            auto def = parseDef();
            if (!def) {
                return nullptr;
            }
            last = std::move(def);
        } else {
            // expr | assign
            auto lhs = parseUnary();
            if (!lhs) {
                return nullptr;
            }
            if (lexer->top() != "=") {
                // expr
                last = parseBinOpRHS(0, std::move(lhs));
            } else {
                // assign
                lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
                auto rhs = parseExpr();
                if (!rhs) {
                    return nullptr;
                }
                // TODO: <= assign to pointer
                if (dynamic_cast<AssignableExprAST *>(lhs.get()) == nullptr) {
                    return setError("can not assign to non assignable expression");
                }
                last = std::make_unique<AssignmentAST>(
                    std::unique_ptr<AssignableExprAST>(dynamic_cast<AssignableExprAST *>(lhs.release())),
                    std::move(rhs));
            }
        }
    }
    if (dynamic_cast<ExprAST *>(last.get()) == nullptr) {
        statements.push_back(std::move(last));
        last = std::make_unique<LiteralExprAST>(std::make_unique<TypeInfo>(voidType), "");
    }
    lexer->pop();
    return std::make_unique<BlockExprAST>(std::move(statements), std::unique_ptr<ExprAST>(dynamic_cast<ExprAST *>(last.release())));
}

std::unique_ptr<DefAST> ExpressionParser::parseDef() {
    if (lexer->top() == "func") {
        // func def
        lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        // TODO: operator[]
        std::string name = lexer->popCopy(ExpressionLexer::Guidence::IGNORE_BREAK);
        TypeInfo type{{"func", "("}};
        auto params = parseParamList();
        eatBreak();
        FunctionDefAST::FuncDefType funcType;
        if (lexer->top() == "(") {
            // member func
            if (params->size() != 1) {
                return setError("member function must have only one accepter");
            }
            auto param1 = parseParamList();
            eatBreak();
            funcType = FunctionDefAST::FuncDefType::MEMBER;
            for (auto &param : (*param1)) {
                params->push_back(std::move(param));
            }
        } else {
            // normal func
            funcType = FunctionDefAST::FuncDefType::NORMAL;
        }
        for (auto &param : (*params)) {
            type.idents.push_back(param->type->toString());
            type.idents.push_back(",");
        }
        if (type.idents.back() == ",") {
            type.idents.pop_back();
        }
        type.idents.push_back(")");
        type.idents.push_back(":");

        if (lexer->top() == ":") {
            // return type
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            auto returnType = (*lexer) | TypeParser();
            if (!returnType) {
                return setError("expected return type in function definition after \":\", found: "+lexer->topCopy());
            }
            eatBreak();
            type.idents.push_back(returnType.toString());
        } else {
            // no return type
            type.idents.push_back("");
        }
        if (lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK) != "->") {
            return setError("expected \"->\" in function definition, found: "+lexer->topCopy());
        }
        auto expr = parseExpr();
        if (!expr) {
            return nullptr;
        }
        return std::make_unique<FunctionDefAST>(std::move(name), std::make_unique<TypeInfo>(type), std::move(*params),
                                                std::move(expr), funcType);
    } else if (lexer->top() == "type") {
        // type def
        lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        auto name = lexer->popCopy(ExpressionLexer::Guidence::IGNORE_BREAK);
        // TODO: type alias
        auto type = (*lexer) | TypeParser();
        return std::make_unique<TypeDefAST>(std::move(name), std::make_unique<TypeInfo>(type));
    } else if (lexer->top() == "var") {
        // var def
        lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        auto indent = lexer->popCopy();
        eatBreak();
        // TODO: ":=" def
        auto type = (*lexer) | TypeParser();
        eatBreak();
        if (lexer->popCopy(ExpressionLexer::Guidence::IGNORE_BREAK) != "=") {
            return setError("expected \"=\" in var definition, found: "+lexer->topCopy());
        }
        auto var = parseExpr();
        return std::make_unique<VarDefAST>(indent, std::make_unique<TypeInfo>(type), std::move(var));
    } else {
        return setError("expect definition keywords like \"var\", \"func\" or \"type\", found: "+lexer->topCopy());
    }
}

std::unique_ptr<std::vector<std::unique_ptr<IdentifierExprAST>>> ExpressionParser::parseParamList() {
    std::unique_ptr<std::vector<std::unique_ptr<IdentifierExprAST>>> ret =
        std::make_unique<std::vector<std::unique_ptr<IdentifierExprAST>>>();
    lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
    while (lexer->top() != ")") {
        if(lexer->tokenType()!=TokenType::IDENT) {
            return setError("except identifier in param list, found: "+lexer->topCopy());
        }
        auto ident = lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        auto type = (*lexer) | TypeParser();
        ret->push_back(
            std::make_unique<IdentifierExprAST>(std::make_unique<TypeInfo>(type), std::move(ident)));
        eatBreak();
        if (lexer->top() != "," && lexer->top() != ")") {
            return setError("expect \",\" after single param, found: "+lexer->topCopy());
        }
        if (lexer->top() == ",") {
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            if (lexer->top() == ")") {
                return setError("param list should not end with \",\"");
            }
        }
    }
    lexer->pop();
    return ret;
}

// std::unique_ptr<ExprAST> ExpressionParser::parseAssignment() {/*prim->check '='*/}

// std::unique_ptr<ExprAST> ExpressionParser::parse() { return parseExpr(); }

// std::unique_ptr<ExprAST> ExpressionParser::parseExpr() {}

// std::unique_ptr<DefAST> ExpressionParser::parseDef() {}

// std::unique_ptr<BlockExprAST> ExpressionParser::parseBlock() {
//     // Block
//     if (lexer->pop() != "{") {
//         return nullptr;
//     }
//     std::vector<std::unique_ptr<AST>> preStatement;
//     std::unique_ptr<ExprAST> value;
//     if (lexer->top() == "}") {
//         return setError("empty Block not supported");
//     }
//     while (lexer->top() != "}") {
//         if (value) {
//             preStatement.push_back(std::move(value));
//         }
//         if (lexer->tokenType() == TokenType::KEYWORD && defKeyWords.find(lexer->top()) != defKeyWords.end()) {
//             // Def
//             auto tmp = parseDef();
//             if (!tmp) {
//                 return nullptr;
//             }
//             preStatement.push_back(std::move(tmp));
//         } else {
//             // Expr | Assign
//             auto tmp = parseExpr();
//             if (!tmp) {
//                 return nullptr;
//             }
//             if (lexer->top() == "=") {
//                 // Assign
//                 auto lvalue_tmp = tmp.get();
//                 auto lvalue_test = dynamic_cast<AssignableExprAST *>(lvalue_tmp);
//                 if (!lvalue_test) {
//                     return setError("try assign to un-assignable value");
//                 }
//                 lexer->pop();
//                 auto rvalue = parseExpr();
//                 if (!rvalue) {
//                     return nullptr;
//                 }
//                 preStatement.push_back(std::make_unique<AssignmentAST>(
//                     std::unique_ptr<AssignableExprAST>(dynamic_cast<AssignableExprAST *>(tmp.release())),
//                     std::move(rvalue)));
//             } else if (lexer->tokenType() == TokenType::ENDLINE) {
//                 // TODO: Expr
//                 lexer->pop();
//                 value = std::move(tmp);
//             } else {
//                 return setError("unknown token '" + lexer->topCopy() + "'");
//             }
//         }
//         if (lexer->tokenType() != TokenType::ENDLINE && lexer->top() != "}") {
//             return setError("expect ';' or '\\n', found " + lexer->topCopy());
//         }
//         if (lexer->tokenType() == TokenType::ENDLINE) {
//             lexer->pop();
//         }
//     }
//     if (!value) {
//         return setError("block must end with expression");
//     }
//     lexer->pop(ExpressionLexer::Guidence::SEEK_ENDLINE);
//     return std::make_unique<BlockExprAST>(
//         // TODO:
//         std::make_unique<TypeInfo>(),
//         std::move(preStatement),
//         std::move(value)
//     );
// }

// std::unique_ptr<ExprAST> ExpressionParser::parseParent() {
//     lexer->pop();
//     auto p = parseExpr();
//     if (auto tmp = lexer->popCopy(ExpressionLexer::Guidence::SEEK_ENDLINE); tmp != ")") {
//         return setError("expect ')', found '" + tmp + "'");
//     }
//     return std::move(p);
// }

// std::unique_ptr<ExprAST> ExpressionParser::parseCall(){}

// std::string ExpressionParser::parseTypeName(){}

// // TODO: call on returned func like "add(1)(2)"
// std::unique_ptr<ExprAST> ExpressionParser::parsePrimary() {
//     if (lexer->tokenType() == TokenType::SYM) {
//         // Parentheses | Block | Symbol(PreOp) | ComplexLiteral
//         if (lexer->top() == "{") {
//             // Block
//             return parseBlock();
//         } else if (lexer->top() == "(") {
//             // Parentheses
//             return parseParent();
//         } else if (lexer->top() == "["){
//             // ComplexLiteral
//         } else if (auto it = unaryOp.find(lexer->top()); it != unaryOp.end()) {
//             // TODO: PreOp
//             auto funcIdent = std::make_unique<IdentifierExprAST>(
//                 // TODO:
//                 std::make_unique<TypeInfo>(),
//                 lexer->popCopy()
//             );
//             auto param = parsePrimary();

//         } else {
//             return setError("unknown symbol: '" + lexer->topCopy() + "'");
//         }
//     } else if (lexer->tokenType() == TokenType::KEYWORD) {
//         // Branch | Loop
//         if (lexer->top() == "if") {
//             // TODO: Branch
//         } else if (lexer->top() == "while") {
//             // TODO: Loop
//         }
//     } else if (lexer->tokenType() == TokenType::REAL || lexer->tokenType() == TokenType::INT ||
//                lexer->tokenType() == TokenType::STRING) {
//         // Int | Real | String
//         // TODO: i64 | u64
//         std::string tmp;
//         auto type = std::make_unique<TypeIdent>((lexer->tokenType() == TokenType::STRING) ? typeident::StringType
//                                                                                           : typeident::ValueType);
//         lexer->fill(tmp, ExpressionLexer::Guidence::SEEK_ENDLINE);
//         return std::make_unique<LiteralExprAST>(std::move(type), std::move(tmp));
//     } else if (lexer->tokenType() == TokenType::IDENT) {
//         // TODO: Ident | Funccall | ComplexLiteral
//     }
// }

} // namespace rulejit