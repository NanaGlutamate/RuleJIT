#pragma once

#include <map>
#include <string>
#include <memory>

#include "ast/ast.hpp"
#include "ast/astvisitor.hpp"

namespace rulejit {

struct TypeInferer : public ASTVisitor {}

}