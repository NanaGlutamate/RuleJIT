#pragma once

#include <type_traits>
#include <string>

namespace rulejit {

class ExpressionLexer {
public:
    using BufferType = std::string;
    enum class Token{
        LITERAL = -1,
        IDENTIFIER = -2,
        SYM = -3,
        KEYWORD = -4,
        END = -5,
    };
    ExpressionLexer& getLexer(){static ExpressionLexer e; return e;}
    void loadExpression(const BufferType &expression){buffer = expression; restartLexer();}
    void loadExpression(BufferType &&expression){buffer = std::move(expression); restartLexer();}
    ExpressionLexer& operator>>(std::string_view& dst){dst = next(); return *this;}
    ExpressionLexer& operator>>(std::string& dst){dst = next(); return *this;}
    Token tokenType(){return type;}
    std::string_view top();
    std::string_view next();
private:
    void restartLexer();
    ExpressionLexer() = default;
    Token type;
    BufferType buffer;
};

class RuleSetLexer {};

} // namespace rulejit