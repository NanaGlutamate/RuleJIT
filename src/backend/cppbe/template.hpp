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
inline constexpr auto typeDefHpp = R"(#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <vector>
#include <type_traits>

namespace {0}{{

using f64 = double;
using float64 = double;
using float32 = float;
using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

template <typename T> constexpr inline bool is_vector_v = false;

template <typename T> constexpr inline bool is_vector_v<std::vector<T>> = true;

template <typename T>
constexpr inline bool is_base_v =
    std::is_same_v<T, bool> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, int16_t> ||
    std::is_same_v<T, uint16_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, float> ||
    std::is_same_v<T, double>;// || std::is_same_v<T, std::string>;

template <typename T>
struct typedReal{{
    double v;
    using type = T;
    operator double() const{{ return v; }}
    typedReal(double v) : v(v){{}}
    typedReal() : v(0.){{}}
    typedReal(const typedReal&) = default;
    typedReal& operator=(const typedReal&) = default;
}};

template <typename T> constexpr inline bool is_typedReal_v = false;

template <typename T> constexpr inline bool is_typedReal_v<typedReal<T>> = true;

using CSValueMap = std::unordered_map<std::string, std::any>;

template <typename T>
void emplaceFromAny(T& t, const std::any& v){{
    using Ty = std::remove_cv_t<typename std::remove_reference<T>::type>;
    if constexpr(is_base_v<Ty>){{
        t = std::any_cast<Ty>(v);
    }}else if constexpr(is_typedReal_v<Ty>){{
        t = double(std::any_cast<typename Ty::type>(v));
    }}else if constexpr(is_vector_v<Ty>){{
        auto tmp = std::any_cast<std::vector<std::any>>(&(v));
        t.resize(tmp->size());
        for(size_t i = 0; i < tmp->size(); ++i){{
            emplaceFromAny(t[i], (*tmp)[i]);
        }}
    }}else{{
        t.FromValueMap(std::any_cast<CSValueMap>(v));
    }}
}}

template <typename T>
std::any toAny(const T& t){{
    using Ty = std::remove_cv_t<typename std::remove_reference<T>::type>;
    if constexpr(is_base_v<Ty>){{
        return t;
    }}else if constexpr(is_typedReal_v<Ty>){{
        return (typename Ty::type)(t);
    }}else if constexpr(is_vector_v<Ty>){{
        std::vector<std::any> tmp;
        tmp.reserve(t.size());
        for(auto&& i : t){{
            tmp.emplace_back(toAny(i));
        }}
        return tmp;
    }}else{{
        return t.ToValueMap();
    }}
}}

struct NoInstanceType{{}};

{2}

}}

)";

// name, member, deserialize, serialize
inline constexpr auto typeDef = R"(
struct {0} {{
{1}    void FromValueMap(const CSValueMap& v){{{2}
    }}
    CSValueMap ToValueMap() const {{
        CSValueMap tmp;
{3}        return tmp;
    }}
}};
)";

// type token
inline constexpr auto typeMember = "    {0} {1};\n";
inline constexpr auto typeDeserialize = R"(
        if(auto it = v.find("{1}"); it != v.end()){{
            emplaceFromAny({1}, it->second);
        }})";
inline constexpr auto typeSerialize = "        tmp.emplace(\"{1}\", toAny({1}));\n";

// namespace, prefix, defs, externs
inline constexpr auto funcDefHpp = R"(#pragma once

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

// namespace, prefix, subrulesetcall, subrulesetwrite, subrulesetdef, inits
inline constexpr auto rulesetHpp = R"(#pragma once

#include <vector>
#include <functional>

#include "{1}typedef.hpp"
#include "{1}funcdef.hpp"

namespace {0}{{

struct RuleSet{{
    Input in;
    Output out;
    Cache cache;
    CSValueMap out_map;
    RuleSet() = default;
    void Init(){{
        {5}
    }}
    CSValueMap* GetOutput(){{
        out_map = out.ToValueMap();
        return &out_map;
    }}
    void SetInput(const CSValueMap& map){{
        in.FromValueMap(map);
    }}
    void Tick(){{
{2}
{3}    }}
    {4}
}};

}}
)";

// id
inline constexpr auto subRulesetCall = "        subRuleSet{0}.Tick(*this);\n";
inline constexpr auto subRulesetWrite = "        subRuleSet{0}.writeBack(*this);\n";

// id, func
inline constexpr auto subRulesetDef = R"(
    struct {{
        Cache cache;
        std::unordered_map<std::string, std::function<void(Cache*, Cache*)>> modified;
        template <typename T>
        void loadCache(RuleSet& base, T p, const std::string& name){{
            if(auto it = modified.find(name); it == modified.end()){{
                auto origin = base.cache.*p;
                cache.*p = base.cache.*p;
                modified.emplace(name, [origin, p](Cache* src, Cache* dst){{
                    // only 1 write back to a same cached value valid through each subruleset,
                    // so if src->*p == tmp, there must no write back by this subruleset,
                    // or may write back same value.
                    // in the second case, assume no other write back requires to this value,
                    // so no need to write back.
                    if(src->*p == origin)return;
                    dst->*p = src->*p;
                }});
            }}
        }}
        void Tick(RuleSet& base){{
            {1}
        }}
        void writeBack(RuleSet& base){{
            for(auto&& [_, f] : modified){{
                f(&cache, &base.cache);
            }}
            modified.clear();
        }}
    }}subRuleSet{0};
)";

} // namespace rulejit::templates