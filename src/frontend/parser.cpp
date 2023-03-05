#include "frontend/parser.h"

namespace {}

namespace rulejit {

std::unique_ptr<ExprAST> ExpressionParser::parse() { return parseExpr(); }

std::unique_ptr<ExprAST> ExpressionParser::parseExpr() {}

std::unique_ptr<DefAST> ExpressionParser::parseDef() {}

std::unique_ptr<BlockExprAST> ExpressionParser::parseBlock() {
    // Block
    if (lexer->pop() != "{") {
        return nullptr;
    }
    std::vector<std::unique_ptr<AST>> preStatement;
    std::unique_ptr<ExprAST> value;
    if (lexer->top() == "}") {
        return setError("empty Block not supported");
    }
    while (lexer->top() != "}") {
        if (value) {
            preStatement.push_back(std::move(value));
        }
        if (lexer->tokenType() == TokenType::KEYWORD && defKeyWords.find(lexer->top()) != defKeyWords.end()) {
            // Def
            auto tmp = parseDef();
            if (!tmp) {
                return nullptr;
            }
            preStatement.push_back(std::move(tmp));
        } else {
            // Expr | Assign
            auto tmp = parseExpr();
            if (!tmp) {
                return nullptr;
            }
            if (lexer->top() == "=") {
                // Assign
                auto lvalue_tmp = tmp.get();
                auto lvalue_test = dynamic_cast<AssignableExprAST *>(lvalue_tmp);
                if (!lvalue_test) {
                    return setError("try assign to un-assignable value");
                }
                lexer->pop();
                auto rvalue = parseExpr();
                if (!rvalue) {
                    return nullptr;
                }
                preStatement.push_back(std::make_unique<AssignmentAST>(
                    std::unique_ptr<AssignableExprAST>(dynamic_cast<AssignableExprAST *>(tmp.release())),
                    std::move(rvalue)));
            } else if (lexer->tokenType() == TokenType::ENDLINE) {
                // TODO: Expr
                lexer->pop();
                value = std::move(tmp);
            } else {
                return setError("unknown token '" + lexer->topCopy() + "'");
            }
        }
        if (lexer->tokenType() != TokenType::ENDLINE && lexer->top() != "}") {
            return setError("expect ';' or '\\n', found " + lexer->topCopy());
        }
        if (lexer->tokenType() == TokenType::ENDLINE) {
            lexer->pop();
        }
    }
    if (!value) {
        return setError("block must end with expression");
    }
    lexer->pop(ExpressionLexer::Guidence::SEEK_ENDLINE);
    return std::make_unique<BlockExprAST>(
        // TODO:
        std::make_unique<TypeInfo>(),
        std::move(preStatement), 
        std::move(value)
    );
}

std::unique_ptr<ExprAST> ExpressionParser::parseParent() {
    lexer->pop();
    auto p = parseExpr();
    if (auto tmp = lexer->popCopy(ExpressionLexer::Guidence::SEEK_ENDLINE); tmp != ")") {
        return setError("expect ')', found '" + tmp + "'");
    }
    return std::move(p);
}

std::unique_ptr<ExprAST> ExpressionParser::parseCall(){}

std::string ExpressionParser::parseTypeName(){}

// TODO: call on returned func like "add(1)(2)"
std::unique_ptr<ExprAST> ExpressionParser::parsePrimary() {
    if (lexer->tokenType() == TokenType::SYM) {
        // Parentheses | Block | Symbol(PreOp) | ComplexLiteral
        if (lexer->top() == "{") {
            return parseBlock();
        } else if (lexer->top() == "(") {
            // Parentheses TODO: lambda?
            return parseParent();
        } else if (lexer->top() == "["){
            // ComplexLiteral
        } else if (auto it = preOp.find(lexer->top()); it != preOp.end()) {
            // TODO: PreOp
            auto funcIdent = std::make_unique<IdentifierExprAST>(
                // TODO:
                std::make_unique<TypeInfo>(),
                lexer->popCopy()
            );
            auto param = parsePrimary();

        } else {
            return setError("unknown symbol: '" + lexer->topCopy() + "'");
        }
    } else if (lexer->tokenType() == TokenType::KEYWORD) {
        // Branch | Loop
        if (lexer->top() == "if") {
            // TODO: Branch
        } else if (lexer->top() == "while") {
            // TODO: Loop
        }
    } else if (lexer->tokenType() == TokenType::REAL || lexer->tokenType() == TokenType::INT ||
               lexer->tokenType() == TokenType::STRING) {
        // Int | Real | String
        // TODO: i64 | u64
        std::string tmp;
        auto type = std::make_unique<TypeIdent>((lexer->tokenType() == TokenType::STRING) ? typeident::StringType
                                                                                          : typeident::ValueType);
        lexer->fill(tmp, ExpressionLexer::Guidence::SEEK_ENDLINE);
        return std::make_unique<LiteralExprAST>(std::move(type), std::move(tmp));
    } else if (lexer->tokenType() == TokenType::IDENT) {
        // TODO: Ident | Funccall | ComplexLiteral
    }
}

} // namespace rulejit