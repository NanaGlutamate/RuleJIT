#pragma once

// TODO: vector<bool> -> vector<char>

namespace rulejit::cppgen::templates {

// template <typename T>
// struct CStyleVector{{
//     T* data;
//     size_t length;
//     size_t capacity;
//     size_t elementSize;
// }};
// namespace, prefix, defs
inline constexpr auto typeDefHpp = R"(
#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <vector>
#include <type_traits>

namespace {0}{{

template <typename T> constexpr inline bool is_vector_v = false;

template <typename T> constexpr inline bool is_vector_v<std::vector<T>> = true;

template <typename T>
constexpr inline bool is_base_v =
    std::is_same_v<T, bool> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, int16_t> ||
    std::is_same_v<T, uint16_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, float> ||
    std::is_same_v<T, double>;// || std::is_same_v<T, std::string>;

template <typename T>
struct value{{
    double v;
    using type = T;
    operator double() const{{ return v; }}
    value(double v) : v(v){{}}
}};

using CSValueMap = std::unordered_map<std::string, std::any>;

template <typename T>
void emplaceFromAny(T& t, const std::any& v){{
    if constexpr(is_base_v<T::type>){{
        t = std::any_cast<T::type>(it->second);
    }}else if constexpr(is_vector_v<T::type>){{
        auto tmp = std::any_cast<std::vector<std::any>*>(&(it->second));
        t.resize(tmp->size());
        for(size_t i = 0; i < tmp->size(); ++i){{
            emplaceFromAny(t[i], (*tmp)[i]);
        }}
    }}else{{
        t.FromValueMap(std::any_cast<CSValueMap>(it->second));
    }}
}}

template <typename T>
std::any toAny(const T& t){{
    if constexpr(is_base_v<T>){{
        return t;
    }}else if constexpr(is_vector_v<T>){{
        std::vector<std::any>> tmp;
        tmp.reserve(t.size());
        for(auto&& i : t){{
            tmp.emplace_back(toAny(i));
        }}
        return tmp;
    }}else{{
        return t.ToValueMap();
    }}
}}

{2}

}}

)";

// name, member, deserialize, serialize
inline constexpr auto typeDef = R"(
struct {0} {{
    {1}
    void FromValueMap(const CSValueMap& v) const {{
        {2}
    }}
    CSValueMap ToValueMap() const {{
        CSValueMap tmp;
        {3}
        return tmp;
    }}
}};
)";

// type token
inline constexpr auto typeMember = "{0} {1};\n";
inline constexpr auto typeDeserialize = R"(
    if(auto it = v.find(\"{1}\"); it != v.end()){{
        emplaceFromAny({1}, it->second);
    }}
)";
inline constexpr auto typeSerialize = "tmp.emplace(\"{1}\", toAny({1}));\n";

// namespace, prefix, defs, externs
inline constexpr auto funcDefHpp = R"(
#pragma once

#include <cmath>

#include "{1}typedef.hpp"

extern "C" {{
{3}
}}

namespace {0}{{
{2}

}}

)";

// returntype, funcname, params
inline constexpr auto externFuncDef = R"(
{0} {1}({2});
)";
// returntype, funcname, params, body
inline constexpr auto funcDef = R"(
inline {0} {1}({2}) {{
    {3}
}}
)";

// namespace, prefix, subrulesetcall, subrulesetdef
inline constexpr auto rulesetHpp = R"(
#pragma once

#include <vector>

#include "{1}typedef.hpp"
#include "{1}funcdef.hpp"

namespace {0}{{

struct RuleSet{{
    input in;
    output out;
    cache cache;
    CSValueMap out_map;
    RuleSet() = default;
    CSValueMap* GetOutput(){{
        out_map = out.ToValueMap();
        return &out_map;
    }}
    void SetInput(const CSValueMap& map){{
        in.FromValueMap(map);
    }}
    void Tick(){{
        {2}
    }}
    {3}
}};

}}
)";

// id, 
inline constexpr auto subRulesetDef = R"(
    struct SubRuleSet{0}{{

    }}subRuleSet{0};
)";

} // namespace rulejit::templates