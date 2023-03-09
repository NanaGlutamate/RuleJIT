#pragma once

#include <map>
#include <set>
#include <string>
#include <tuple>

#include "ast/type.hpp"

namespace rulejit {

inline const TypeInfo NoInstanceType{std::vector<std::string>{std::string(typeident::NoInstanceTypeIdent)}};
inline const TypeInfo StringType{std::vector<std::string>{std::string(typeident::StringTypeIdent)}};
inline const TypeInfo IntType{std::vector<std::string>{std::string(typeident::IntTypeIdent)}};
inline const TypeInfo RealType{std::vector<std::string>{std::string(typeident::RealTypeIdent)}};

inline const std::map<std::string, std::map<TypeInfo, size_t>> buildInFunc{
    {"+", 
        {
            {make_type("func(f64,f64):f64"), 0},
        }
    },
    {"-", 
        {
            {make_type("func(f64,f64):f64"), 0},
        }
    },
    {"*", 
        {
            {make_type("func(f64,f64):f64"), 0},
        }
    },
    {"/", 
        {
            {make_type("func(f64,f64):f64"), 0},
        }
    },
};

}