/**
 * @file parser.h
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
 * </table>
 */
#pragma once

#include <format>
#include <map>
#include <type_traits>

#include "ast/ast.hpp"
#include "defines/language.hpp"
#include "frontend/lexer.h"
#include "tools/seterror.hpp"

namespace rulejit {

/**
 * @brief main class of parser
 *
 */
struct ExpressionParser {
    ExpressionParser() = default;
    ExpressionParser(const ExpressionParser &) = delete;
    ExpressionParser(ExpressionParser &&) = delete;
    ExpressionParser &operator=(const ExpressionParser &) = delete;
    ExpressionParser &operator=(ExpressionParser &&) = delete;

    /// @brief error handler to handle error information and state when error
    struct Error {
        bool err;
        TokenType type;
        std::string_view token;
        std::string info;
        operator bool() { return err; }
        void clear() { err = false; }
    } errorHandler;

    /**
     * @brief call before parse, used to escape empty lines
     *
     */
    void selectNextExpr() {
        my_assert(lexer);
        while (lexer->tokenType() == TokenType::ENDLINE) {
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        }
    }

    /**
     * @brief pick next expression in ExpressionParser
     *
     * TODO: donot use type conversion operator here
     *
     * @return std::unique_ptr<ExprAST>
     */
    operator std::unique_ptr<ExprAST>() {
        selectNextExpr();
        if (lexer->tokenType() == TokenType::END) {
            return nullptr;
        }
        return (parse());
    }

    /**
     * @brief pipe operator| for ExpressionParser to load ExpressionLexer as input stream
     *
     * @param src source ExpressionLexer
     * @param e receiver ExpressionParser
     * @return ExpressionParser& reference to receiver
     */
    friend ExpressionParser &operator|(ExpressionLexer &src, ExpressionParser &e) {
        e.bind(src);
        return e;
    }

    /**
     * @brief map from AST to place in string, used for better error message, not used for now
     *
     */
    std::map<IAST *, size_t> AST2place;

  private:
    [[noreturn]] std::nullptr_t setError(const std::string &info) {
        std::string modified;
        for (auto c : info) {
            if (c == '\n') {
                modified += "\\n";
            } else {
                modified += c;
            }
        }
        errorHandler = {
            true,
            lexer->tokenType(),
            lexer->top(),
            modified,
        };
        error(std::format("Parse Error: {}", modified));
    }

    std::unique_ptr<ExprAST> parse() {
        AST2place.clear();
        return parseExpr();
    };
    // add tuple after generics support
    std::unique_ptr<ExprAST> parseExpr(bool ignoreBreak = false, bool allowTuple = false);
    std::unique_ptr<ExprAST> parseBinOpRHS(Priority priority, std::unique_ptr<ExprAST> lhs, bool ignoreBreak = false,
                                           bool allowTuple = false);
    std::unique_ptr<ExprAST> parseUnary();
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST> parseBlock();
    std::unique_ptr<ExprAST> parseDef();

    void parseFuncDef(std::vector<std::unique_ptr<rulejit::IdentifierExprAST>> &params, TypeInfo &returnType,
                      std::string &funcName, FunctionDefAST::FuncDefType &funcDefType);
    std::vector<std::unique_ptr<IdentifierExprAST>> parseParamList(const std::string& end = ")");

    std::unique_ptr<ExprAST> parseCommand();
    std::unique_ptr<ExprAST> parseTopLevel();

    void eatBreak() {
        if (lexer->top().back() == '\n') {
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        }
    }

    ExpressionParser &bind(ExpressionLexer &elexer) { return changeSource(elexer).restart(); }
    ExpressionParser &changeSource(ExpressionLexer &elexer) {
        lexer = std::addressof(elexer);
        return *this;
    }
    ExpressionParser &restart() {
        errorHandler.clear();
        return *this;
    };
    // ContextStack *context;
    ExpressionLexer *lexer;
    // TokenStream* stream;
};

} // namespace rulejit