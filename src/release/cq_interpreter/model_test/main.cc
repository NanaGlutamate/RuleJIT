#include <any>
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

#include "../csmodel_base/csmodel_base.h"
#include "tools/printcsvaluemap.hpp"

typedef CSModelObject *(*CreateModelObjectFun)();
typedef void (*DestroyMemoryFun)(void *mem, bool is_array);

int main() {
    using namespace rulejit;
    using CSValueMap = std::unordered_map<std::string, std::any>;
    std::string lib_path_ = "D:/Desktop/FinalProj/Code/RuleJIT/bin/Debug/"
                            "RuleJIT_interpreter.dll";

#ifdef _WIN32
    auto hmodule = LoadLibraryExA(lib_path_.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    void *hmodule = dlopen(lib_path_.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
    if (!hmodule) {
        std::cout << "load "
                  << "RuleJIT_interpreter"
                  << " failed" << std::endl;
        return false;
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
        return false;
    }

    // 模型实例
    CSModelObject *model_obj_ = create_obj_();
    if (nullptr == model_obj_) {
        std::cerr << "create model error" << std::endl;
        return -1;
    }
    auto engine = model_obj_;
    // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    engine->SetLogFun([](const std::string &msg, uint32_t type) {});
    engine->Init(CSValueMap{{"filePath", std::string(__PROJECT_ROOT_PATH "/doc/test_xml/BVR1.0.xml")}});

    std::vector<CSValueMap> inputs{
        CSValueMap{
            {"d", double(55000)},       {"phiA", double(50)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(45000)},       {"phiA", double(80)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(35000)},       {"phiA", double(80)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(80)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(80)},
            {"radar", double(1)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(80)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(20)},
            {"radar", double(1)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(35000)},       {"phiA", double(60)},       {"phiT", double(20)},
            {"radar", double(1)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(35000)},       {"phiA", double(60)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(60)},       {"phiT", double(20)},
            {"radar", double(1)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(60)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(80)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(80)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(60)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(60)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(40)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(40)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(80)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(80)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(60)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(60)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(40)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(40)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(1)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },

        CSValueMap{
            {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(20)},
            {"radar", double(0)},       {"missile", double(0)},     {"DTRmax", double(50000)},
            {"DTMmax", double(40000)},  {"DTMmin", double(10000)},  {"DTMKmax", double(30000)},
            {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},   {"phiTMmax", double(50)},
            {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)},
            {"DAMmin", double(10000)},  {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)},
            {"phiARmax", double(70)},   {"phiAMmax", double(50)},   {"phiAMK", double(30)},
        },
    };

    for (auto &&input : inputs) {
        printCSValueMap(*(engine->GetOutput()));
        std::cout << std::endl;
        engine->SetInput(input);
        engine->Tick(0.02);
    }
    printCSValueMap(*(engine->GetOutput()));

    return 0;
}