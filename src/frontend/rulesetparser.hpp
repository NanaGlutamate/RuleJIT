#pragma once

#include <map>
#include <source_location>
#include <type_traits>

#include "ast/ast.hpp"
#include "defines/language.hpp"
#include "frontend/lexer.h"

#include "rapidxml-1.13/rapidxml.hpp"

namespace rulejit {

struct RuleSetParser {
    RuleSetParser() = default;
};

struct SubRuleSetParser {
    SubRuleSetParser() = default;
};

}