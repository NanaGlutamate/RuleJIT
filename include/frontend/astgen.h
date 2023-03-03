#pragma once

#include "lexer.h"

namespace rulejit {

struct RuleSetASTGenerator{

};

struct ExprASTGenerator{
    ExprASTGenerator():lexer(ExpressionLexer::getLexer()){};
private:
    ExpressionLexer& lexer;
};

}