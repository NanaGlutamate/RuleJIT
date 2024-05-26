/**
 * @file template.hpp
 * @author djw
 * @brief CQ/CPPBE/Code templates
 * @date 2023-03-27
 * 
 * @details Includes template strings used in std::format for code generation.
 * specifically, {prefix}funcdef.hpp, {prefix}typedef.hpp
 * {prefix}ruleset.hpp, {prefix}ruleset.cpp, testmain.cpp, cqinterface.hpp and CMakeLists.txt
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

// TODO: vector<bool> -> vector<char>

namespace rulejit::cppgen::templates {

// namespace, prefix, defs, autocollector
inline constexpr auto typeDefHpp = R"(#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <vector>
#include <type_traits>

namespace {0}{{
using std::string;

using f32 = float;
using f64 = double;
using i32 = int32_t;
using i64 = int64_t;
using u32 = uint32_t;
using u64 = uint64_t;

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
    std::is_same_v<T, double> || std::is_same_v<T, std::string>;

template <typename T>
struct typedReal{{
    double v;
    using type = T;
    operator double() const{{ return v; }}
    typedReal(double v) : v(static_cast<T>(v)){{}}
    typedReal() : v(0.){{}}
    typedReal(const typedReal&) = default;
    template<typename Other>
    void operator=(const typedReal<Other>& o){{
        // type trans?
        v = static_cast<T>(o.v);
    }}
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
        auto tmp = std::any_cast<std::vector<std::any>>(&v);
        if(tmp==nullptr){{
            t.clear();
            return;
        }}
        t.resize(tmp->size());
        for(size_t i = 0; i < tmp->size(); ++i){{
            emplaceFromAny(t[i], (*tmp)[i]);
        }}
    }}else{{
        t.FromValueMap(*std::any_cast<CSValueMap>(&v));
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

struct NoInstanceType{{
    NoInstanceType() = default;
    NoInstanceType(const NoInstanceType&) = delete;
    NoInstanceType& operator=(const NoInstanceType&) = delete;
    NoInstanceType(NoInstanceType&&) = delete;
    NoInstanceType& operator=(NoInstanceType&&) = delete;
}};

{3}

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
    void ToValueMap(CSValueMap& tmp) const {{
{3}    }}
}};
)";

// input vector auto-collector
// readers
inline constexpr auto autoCollecter = R"(
struct __AutoCollector{{
    std::unordered_map<std::string, std::vector<std::any>> buffer;
    CSValueMap assemble(){{
        CSValueMap ret;
        for(auto&&[k, v] : buffer){{
            ret[k] = std::move(v);
        }}
        buffer.clear();
        return ret;
    }}
    void read(const CSValueMap& value){{
{0}    }}
}};
)";
// value member names
inline constexpr auto autoCollecterReader = R"(        if(auto it = value.find("{0}"); it != value.end() && it->second.type() != typeid(std::vector<std::any>)){{
            buffer["{0}"].emplace_back(it->second);
        }}
)";

// type token
inline constexpr auto typeMember = "    {0} {1};\n";
inline constexpr auto typeDeserialize = R"(
        if(auto it = v.find("{1}"); it != v.end()){{
            emplaceFromAny({1}, it->second);
        }})";
inline constexpr auto typeSerialize = "        tmp[\"{1}\"] = toAny({1});\n";

// namespace, prefix, predefs, defs, externs
inline constexpr auto funcDefHpp = R"(#pragma once

#include <cmath>

#include "{1}typedef.hpp"

extern "C" {{
using namespace {0};
{4}
}}

namespace {0}{{

template <typename V>
size_t length(V&& v){{ return v.size(); }}

template <typename V, typename Op>
void push(V& v, Op&& op){{ v.push_back(std::forward<Op>(op)); }}

template <typename V>
void resize(V& v, size_t size){{ v.resize(size); }}

bool strEqual(const string& lhs, const string& rhs){{ return lhs == rhs; }}
{2}
{3}

}}

)";

// returntype, funcname, params
inline constexpr auto externFuncDef = R"(
{0} {1}({2});
)";
// returntype, funcname, params, body
inline constexpr auto funcDef = R"(
inline {0} {1}({2}) {{
    {3};
}}
)";
// returntype, funcname, params
inline constexpr auto funcPreDef = R"(
inline {0} {1}({2});
)";

// namespace, prefix, subrulesetcall, subrulesetwrite, subrulesetdef, inits, precall, prewrite
inline constexpr auto rulesetHpp = R"(#pragma once

#include <set>

#include "{1}typedef.hpp"
#include "{1}funcdef.hpp"

namespace {0}{{

struct RuleSet{{
    _Input in;
    _Output out;
    _Cache cache;
    __AutoCollector ac;
    CSValueMap out_map;
    RuleSet() = default;
    void Init(){{
        {5}
    }}
    CSValueMap* GetOutput(){{
        return &out_map;
    }}
    void SetInput(const CSValueMap& map){{
        ac.read(map);
        in.FromValueMap(map);
    }}
    void Tick(){{
        if(ac.buffer.size()){{
            in.FromValueMap(ac.assemble());
        }}
        // auto &_in = in;
        // auto &_out = out;
        // auto _base = 0;
        // auto loadCache = [](auto x, auto y, auto z){{}};
{6}
{7}
{2}
{3}        out.ToValueMap(out_map);
    }}
{4}
}};

}}
)";

// id
inline constexpr auto subRulesetCall = "        subRuleSet{0}.Tick(*this);\n";
inline constexpr auto subRulesetWrite = "        subRuleSet{0}.writeBack(*this);\n";

// TODO: use double-buffer / copy-on-write to replace unconditional buffer copy
// TODO: record modified output to avoid meaningless serialization
// id, func, writeback
inline constexpr auto subRulesetDef = R"(    struct {{
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){{
            if(auto it = loaded.find(name); it == loaded.end()){{
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }}
        }}
        void Tick(RuleSet& _base){{
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = {1};
        }}
        void writeBack(RuleSet& base){{
            switch(actived){{
                case -1:
                    break;{2}
            }}
            loaded.clear();
        }}
    }}subRuleSet{0};
)";

// id, member
inline constexpr auto subRulesetWriteCase = R"(
                case {0}:{1}
                    break;
)";

// member name
inline constexpr auto subRulesetWriteCaseMember = R"(
                    base.cache.{0} = std::move(cache.{0});)";

// namespace, prefix
inline constexpr auto CMakeListsTxt = R"(cmake_minimum_required(VERSION 3.6)
set(PROJ_NAME ruleset)
project(${{PROJ_NAME}})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${{PROJECT_SOURCE_DIR}}/bin)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /permissive- /Zc:__cplusplus")
endif()

add_library(${{PROJ_NAME}} SHARED {1}ruleset.cpp)
add_executable(${{PROJ_NAME}}_test testmain.cpp)
add_dependencies(${{PROJ_NAME}}_test ${{PROJ_NAME}})
if(UNIX)
target_link_libraries(${{PROJ_NAME}} dl)
else(UNIX)
endif(UNIX))";

// namespace, prefix
inline constexpr auto rulesetCpp = R"(#include <functional>
#include <map>
#include <exception>
#include <any>
#include <string>
#include <unordered_map>
#include <vector>

#include "{1}typedef.hpp"
#include "{1}funcdef.hpp"
#include "{1}ruleset.hpp"
#include "cqinterface.hpp"

extern "C" __declspec(dllexport) CSModelObject* __stdcall CreateModelObject();
extern "C" __declspec(dllexport) void __stdcall DestroyMemory(void *mem, bool is_array);

class RuleEngine : public CSModelObject {{
  public:
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) override{{
        engine.Init();
        auto params_ = engine.GetOutput();
        engine.SetInput(value);
        params_->emplace("ForceSideID", GetForceSideID());
        params_->emplace("ModelID", GetModelID());
        params_->emplace("InstanceName", GetInstanceName());
        params_->emplace("ID", GetID());
        params_->emplace("State", uint16_t(CSInstanceState::IS_RUNNING));
        state_ = CSInstanceState::IS_RUNNING;
        return true;
    }};
    virtual bool Tick(double time) override{{
        engine.Tick();
        return true;
    }};
    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) override{{
        engine.SetInput(value);
        return true;
    }};
    virtual std::unordered_map<std::string, std::any> *GetOutput() override{{
        return engine.GetOutput();
    }};
  private:
    {0}::RuleSet engine;
}};

CSModelObject* __stdcall CreateModelObject() {{
    CSModelObject *model = new RuleEngine();
    return model;
}}

void __stdcall DestroyMemory(void *mem, bool is_array) {{
    if (is_array) {{
        delete[] ((RuleEngine *)mem);
    }} else {{
        delete ((RuleEngine *)mem);
    }}
}}

)";

inline constexpr auto cqinterfaceHpp = R"(#pragma once

#include <any>
#include <exception>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

enum class CSInstanceState {
    IS_UNSPECIFIED = 0,
    IS_CREATED = 1,
    IS_INITIALIZED = 2,
    IS_RUNNING = 3,
    IS_STOPPED = 4,
    IS_DESTROYED = 5,
    IS_ERROR = 6
};

class CSModelObject {
  public:
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) = 0;
    virtual bool Tick(double time) = 0;
    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) = 0;
    virtual std::unordered_map<std::string, std::any> *GetOutput() = 0;
  public:
    void SetState(CSInstanceState state) { state_ = state; }
    CSInstanceState GetState(void) { return state_; }
    void SetID(uint64_t id) { id_ = id; }
    uint64_t GetID() const { return id_; }
    void SetModelID(const std::string &model_id) { model_id_ = model_id; }
    const std::string &GetModelID() const { return model_id_; }
    void SetInstanceName(const std::string &instance_name) { instance_name_ = instance_name; }
    const std::string &GetInstanceName() const { return instance_name_; }
    void SetForceSideID(uint16_t force_side_id) { force_side_id_ = force_side_id; }
    uint16_t GetForceSideID() const { return force_side_id_; }
  protected:
    CSInstanceState state_ = CSInstanceState::IS_UNSPECIFIED;
    uint64_t id_;
    std::string model_id_;
    std::string instance_name_;
    uint16_t force_side_id_;
  protected:
    void WriteLog(const std::string &msg, uint32_t level = 0) { log_(msg, level); }
    void WriteKeyMessage(const std::vector<std::any> &msgs) { params_.emplace("KeyMessages", msgs); }
    void WriteTopic(const std::string &topic_name, const std::unordered_map<std::string, std::any> &topic_data) {
        std::unordered_map<std::string, std::any> topic;
        topic["TopicName"] = topic_name;
        topic["Params"] = topic_data;
        com_cb_("DirectWriteTopic", topic);
    }
    void CreateEntity(const std::string &model_id, const std::string &instance_name, uint64_t instance_id,
                      uint16_t forceside_id, std::unordered_map<std::string, std::any> &params) {
        std::unordered_map<std::string, std::any> basic_params = {
            {"InstanceName", instance_name}, {"ID", instance_id}, {"ModelID", model_id}, {"ForceSideID", forceside_id}};
        basic_params.merge(params);
        com_cb_("CreateEntity", basic_params);
    }
  protected:
    std::string CommonCallBack(const std::string &fun_name, const std::unordered_map<std::string, std::any> &params) {
        return com_cb_(fun_name, params);
    }
  public:
    void SetLogFun(std::function<void(const std::string &, uint32_t)> log) { log_ = log; }
    void SetCommonCallBack(
        std::function<std::string(const std::string &, const std::unordered_map<std::string, std::any> &)> com_cb) {
        com_cb_ = com_cb;
    }
  protected:
    std::function<void(const std::string &, uint32_t)> log_;
    std::function<std::string(const std::string &, const std::unordered_map<std::string, std::any> &)> com_cb_;
    std::unordered_map<std::string, std::any> params_;
};
)";

inline constexpr auto testmainCpp = R"(#include <any>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>
#else
#include <dlfcn.h>
#endif
#include <filesystem>

#include "cqinterface.hpp"

void printCSValueMap(const std::unordered_map<std::string, std::any> &v) {
    using namespace std;
    cout << "{";
    bool start = false;
    for (auto &[k, v] : v) {
        if (!start) {
            start = true;
        } else {
            cout << ", ";
        }
        cout << k << ": ";
        if (v.type() == typeid(bool)) {
            cout << (bool)std::any_cast<bool>(v);
        } else if (v.type() == typeid(int8_t)) {
            cout << (int64_t)std::any_cast<int8_t>(v);
        } else if (v.type() == typeid(uint8_t)) {
            cout << (uint64_t)std::any_cast<uint8_t>(v);
        } else if (v.type() == typeid(int16_t)) {
            cout << (int64_t)std::any_cast<int16_t>(v);
        } else if (v.type() == typeid(uint16_t)) {
            cout << (uint64_t)std::any_cast<uint16_t>(v);
        } else if (v.type() == typeid(int32_t)) {
            cout << std::any_cast<int32_t>(v);
        } else if (v.type() == typeid(uint32_t)) {
            cout << std::any_cast<uint32_t>(v);
        } else if (v.type() == typeid(int64_t)) {
            cout << std::any_cast<int64_t>(v);
        } else if (v.type() == typeid(uint64_t)) {
            cout << std::any_cast<uint64_t>(v);
        } else if (v.type() == typeid(float)) {
            cout << std::any_cast<float>(v);
        } else if (v.type() == typeid(double)) {
            cout << std::any_cast<double>(v);
        } else if (v.type() == typeid(std::string)) {
            cout << std::any_cast<std::string>(v);
        } else if (v.type() == typeid(std::unordered_map<std::string, std::any>)) {
            printCSValueMap(std::any_cast<std::unordered_map<std::string, std::any>>(v));
        } else if (v.type() == typeid(std::vector<std::any>)) {
            auto tmp = std::any_cast<std::vector<std::any>>(v);
            cout << "[";
            bool start_ = false;
            size_t cnt = 0;
            for (auto &&item : tmp) {
                if (!start_) {
                    start_ = true;
                } else {
                    cout << ", ";
                }
                printCSValueMap({{std::to_string(cnt), item}});
                cnt++;
            }
            cout << "]";
        } else {
            cout << "unknown";
        }
    }
    cout << "}";
}

typedef CSModelObject *(*CreateModelObjectFun)();
typedef void (*DestroyMemoryFun)(void *mem, bool is_array);

int main() {

    using CSValueMap = std::unordered_map<std::string, std::any>;
    std::string lib_path_ = "ruleset.dll";

#ifdef _WIN32
    auto hmodule = LoadLibraryExA(lib_path_.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    void *hmodule = dlopen(lib_path_.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
    if (!hmodule) {
        std::cout << "load ruleset failed" << std::endl;
        return -1;
    }

#ifdef _WIN32
    auto create_obj_ = (CreateModelObjectFun)GetProcAddress(hmodule, "CreateModelObject");
    auto destroy_obj_ = (DestroyMemoryFun)GetProcAddress(hmodule, "DestroyMemory");
#else
    auto create_obj_ = (CreateModelObjectFun)dlsym(hmodule, "CreateModelObject");
    auto destroy_obj_ = (DestroyMemoryFun)dlsym(hmodule, "DestroyMemory");
#endif
    if (!create_obj_ || !destroy_obj_) {
#ifdef _WIN32
        if (!FreeLibrary(hmodule))
#else
        if (!dlclose(hmodule))
#endif
            std::cout << "release dll error" << std::endl;
        return -1;
    }
    CSModelObject *model_obj_ = create_obj_();
    if (nullptr == model_obj_) {
        std::cerr << "create model error" << std::endl;
        return -1;
    }
    auto engine = model_obj_;
    // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    engine->Init(CSValueMap{});
    for (int i = 0; i < 20; i++) {
        printCSValueMap(*(engine->GetOutput()));
        std::cout << std::endl;
        engine->SetInput(CSValueMap{});
        engine->Tick(0.02);
    }
    return 0;
})";

} // namespace rulejit::templates