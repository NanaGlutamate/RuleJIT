#pragma once

#include "lexer.h"
#include "ast/ast.hpp"

namespace rulejit {

struct RuleSetASTGenerator{

};

struct ExprASTGenerator{
    ExprASTGenerator():lexer(){};
private:
    ExpressionLexer lexer;
};

}