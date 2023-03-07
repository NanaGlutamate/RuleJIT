#pragma once

#include <map>
#include <set>
#include <string>
#include <tuple>

#include "ast/type.hpp"

namespace rulejit {

inline const std::map<std::string, std::map<TypeInfo, size_t>> buildInFunc{
    {"+", 
        {
            {0, make_type("func(f64,f64):f64")},
        }
    },
    {"-", 
        {
            {0, make_type("func(f64,f64):f64")},
        }
    },
    {"*", 
        {
            {0, make_type("func(f64,f64):f64")},
        }
    },
    {"/", 
        {
            {0, make_type("func(f64,f64):f64")},
        }
    },
};

}