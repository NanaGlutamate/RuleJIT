#pragma once

namespace rulejit::templates{

// namespace, prefix, defs, i, o, cache
inline constexpr auto typeDefHpp = R"(
#pragma once

#include <unordered_map>
#include <string>

namespace {0}{{

using CSValueMap = std::unordered_map<std::string, std::string>;

// template <typename T>
// struct CStyleVector{{
//     T* data;
//     size_t length;
// }};
{2}
struct input{{
    {3}
}};

struct output{{
    {4}
}};

struct cache{{
    {5}
}};

// struct vectors{{
//     {6}
// }};

}}

)";

// name, member, deserialize, serialize
inline constexpr auto typeDef = R"(
struct {0} {{
    {1}
    void FromValueMap(const CSValueMap& map) const {{
        {2}
    }}
    CSValueMap ToValueMap() const {{
        CSValueMap tmp;
        {3}
        return tmp;
    }}
}};
)";

// namespace, prefix, defs
inline constexpr auto funcDefHpp = R"(
#pragma once

#include <cmath>

#include "{1}typedef.hpp"

namespace {0}{{
{2}

}}

)";

// returntype, funcname, params
inline constexpr auto externFuncDef = R"(
extern "C" {0} {1}({2});
)";

// returntype, funcname, params, body
inline constexpr auto funcDef = R"(
inline {0} {1}({2}) {{
    {3}
}}
)";

inline constexpr auto loaderHpp = R"(

)";

// namespace, prefix, subrulesetcall, subrulesdef
inline constexpr auto rulesetHpp = R"(
#pragma once

#include <vector>

#include "{1}typedef.hpp"
#include "{1}funcdef.hpp"
#include "{1}loader.hpp"

namespace {0}{{

struct RuleSet{{
    input in;
    output out;
    cache cache;
    CSValueMap in_map, out_map;
    RuleSet() = default;
    CSValueMap* GetOutput(){{
        return &out_map;
    }}
    void SetInput(const CSValueMap& map){{
        in_map = map;
    }}
    void Tick(){{
        {2}
    }}
    {3}
}};

}}

)";

}