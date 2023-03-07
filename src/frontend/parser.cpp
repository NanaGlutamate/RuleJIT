#include "frontend/parser.h"
#include "ast/type.hpp"

namespace rulejit {

// EXPR := UNARYEXPR (op UNARYEXPR)*
std::unique_ptr<ExprAST> ExpressionParser::parseExpr() {
    auto lhs = parseUnary();
    if (!lhs) {
        return nullptr;
    }
    return parseBinOpRHS(0, std::move(lhs));
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
        lhs = parseUnary();
        if (!lhs) {
            return nullptr;
        }
        lhs = parseBinOpRHS(0, std::move(lhs), true);
        if (!lhs) {
            return nullptr;
        }
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
    } else if (typeIndicator.contains(lexer->topCopy())) {
        // Type
        auto typeInfo = (*lexer) | TypeParser();
        eatBreak();
        if (lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK) != "{") {
            return setError("expected \"{\" in complex type literal expression");
        }
        std::vector<std::tuple<std::string, std::unique_ptr<ExprAST>>> members;
        while (lexer->top() != "}") {
            if (lexer->tokenType() == TokenType::IDENT) {
                auto state = lexer->getState();
                std::string key;
                key = lexer->popCopy(ExpressionLexer::Guidence::IGNORE_BREAK);
                if (lexer->top() == ":") {
                    auto var = parseExpr();
                    if (!var) {
                        return nullptr;
                    }
                    eatBreak();
                    members.push_back({std::move(key), std::move(var)});
                    continue;
                }
                lexer->loadState(state);
            }
            auto var = parseExpr();
            if (!var) {
                return nullptr;
            }
            eatBreak();
            members.push_back({"", std::move(var)});
            if (lexer->top() != ",") {
                break;
            } else {
                lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
            }
        }
        // TODO:
        if (lexer->pop() != "}") {
            return setError("expected \"}\" in complex type literal expression");
        }
        lexer->pop();
        lhs = std::make_unique<ComplexLiteralExprAST>(std::move(typeInfo), std::move(members));
    } else {
        // Type | Identifier | FuncCall
        std::string token = lexer->popCopy();
    }
}

std::unique_ptr<ExprAST> parsePrimBinOp(Priority priority, std::unique_ptr<ExprAST> lhs) {}

std::unique_ptr<BlockExprAST> ExpressionParser::parseBlock() {}

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