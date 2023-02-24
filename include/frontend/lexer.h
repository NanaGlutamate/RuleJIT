#pragma once

#include <string>

namespace rulejit {

template <typename BufferType> class ExpressionLexer {
  public:
    enum class Token{
        NUMBER,
        IDENTIFIER,
        UNKNOW,
    };
    ExpressionLexer() = default;
    void loadExpression(const BufferType &expression) { buffer = expression; }
    void loadExpression(BufferType &&expression) { buffer = std::move(expression); }
    ExpressionLexer& operator>>(){}
    std::string_view top(){}
  private:
    BufferType buffer;
};

class RuleSetLexer {};

} // namespace rulejit