#pragma once

#include <map>
#include <type_traits>

#include "ast/ast.hpp"
#include "ast/type.hpp"
#include "frontend/language.hpp"
#include "frontend/lexer.h"
#include "rapidxml-1.13/rapidxml.hpp"

namespace rulejit {

// template <class T>
// struct Parser{
// };

struct Context {
    std::map<std::string, std::unique_ptr<TypeInfo>> typeDef;
    std::map<std::string, std::unique_ptr<TypeInfo>> varDef;
    std::map<std::string, std::unique_ptr<FuncType>> funcDef;
    std::map<std::string, int> infixOp;
    void clear() {
        typeDef.clear();
        varDef.clear();
        funcDef.clear();
        infixOp.clear();
    }
};

struct RuleSetParser {
    RuleSetParser() = default;
};

struct SubRuleSetParser {
    SubRuleSetParser() = default;
};

struct ExpressionParser {
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
    ExpressionParser() = default;
    friend std::unique_ptr<ExprAST> operator|(ExpressionLexer &src, ExpressionParser &e) {
        e.bind(src);
        auto tmp = e.parse();
        e.lexer = nullptr;
        return std::move(tmp);
    }
    ExpressionParser &loadContext(Context &c) {
        context = std::addressof(c);
        return *this;
    }

  private:
    bool err() { return errorHandler.err; }
    std::nullptr_t setError(const std::string &info) {
        errorHandler = {
            true,
            lexer->tokenType(),
            lexer->top(),
            info,
        };
        return nullptr;
    }

    std::unique_ptr<ExprAST> parse();
    std::unique_ptr<ExprAST> parseExpr();
    std::unique_ptr<DefAST> parseDef();
    std::unique_ptr<BlockExprAST> parseBlock();
    std::unique_ptr<ExprAST> parseParent();
    std::unique_ptr<ExprAST> parseCall();
    std::string parseTypeName();
    std::unique_ptr<ExprAST> parsePrimary();



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
    Context *context;
    ExpressionLexer *lexer;
    // TokenStream* stream;
};

// auto tmp = s | lexer | parser | compiler;
// TODO: auto tmp = cin | lexer | parser | interpreter;

} // namespace rulejit