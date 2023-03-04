#pragma once

#include "lexer.h"

namespace rulejit {

struct RuleSetASTGenerator{

};

struct ExprASTGenerator{
    ExprASTGenerator():lexer(){};
private:
    ExpressionLexer lexer;
};

}