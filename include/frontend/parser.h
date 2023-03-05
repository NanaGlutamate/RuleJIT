#pragma once

#include <type_traits>

#include "rapidxml-1.13/rapidxml.hpp"
#include "ast/ast.hpp"
#include "frontend/lexer.h"

namespace rulejit{

// template <class T>
// struct Parser{
// };

struct RuleSetParser{
    RuleSetParser() = default;
};

struct SubRuleSetParser{
    SubRuleSetParser() = default;
};

struct ExpressionParser{
    ExpressionParser() = default;
    template<typename Ty, typename This> requires std::is_same_v<ExpressionParser, std::remove_all_extents_t<This>>
    friend std::unique_ptr<ExprAST> operator|(Ty&&src, This&& e){
        static ExpressionParser real_e;
        real_e = std::forward<This>(e);
        real_e.load(std::forward<Ty>(src));
        return real_e;
    }
};

auto tmp = std::string("12") | ExpressionLexer{} | ExpressionParser{};

}