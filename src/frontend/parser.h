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
#include <source_location>
#include <type_traits>

#include "ast/ast.hpp"
#include "defines/language.hpp"
#include "frontend/lexer.h"

namespace rulejit {

struct ExpressionParser {
    ExpressionParser() = default;
    ExpressionParser(const ExpressionParser &) = delete;
    ExpressionParser(ExpressionParser &&) = delete;
    ExpressionParser &operator=(const ExpressionParser &) = delete;
    ExpressionParser &operator=(ExpressionParser &&) = delete;

    struct Error {
        bool err;
        TokenType type;
        std::string_view token;
        std::string info;
        operator bool() { return err; }
        void clear() { err = false; }
    } errorHandler;
    enum class State {

        END,
    };
    void selectNextExpr() {
        my_assert(lexer);
        while (lexer->tokenType() == TokenType::ENDLINE) {
            lexer->pop(ExpressionLexer::Guidence::IGNORE_BREAK);
        }
    }
    operator std::unique_ptr<ExprAST>() {
        selectNextExpr();
        if (lexer->tokenType() == TokenType::END) {
            return nullptr;
        }
        return (parse());
    }
    friend ExpressionParser &operator|(ExpressionLexer &src, ExpressionParser &e) {
        e.bind(src);
        return e;
    }
    // ExpressionParser &loadContext(ContextStack &c) {
    //     context = std::addressof(c);
    //     return *this;
    // }
    std::map<AST *, size_t> AST2place;

  private:
    bool err() { return errorHandler.err; }
    [[noreturn]] std::nullptr_t setError(const std::string &info,
                                         const std::source_location location = std::source_location::current()) {
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
        throw std::logic_error(std::format("Parse Error{}: {}", location.line(), modified));
        // return nullptr;
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
    std::unique_ptr<std::vector<std::unique_ptr<IdentifierExprAST>>> parseParamList();

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
    State state;
    // ContextStack *context;
    ExpressionLexer *lexer;
    // TokenStream* stream;
};

} // namespace rulejit