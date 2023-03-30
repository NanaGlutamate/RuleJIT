/**
 * @file main.cc
 * @author djw
 * @brief CQ/Interpreter/Test main
 * @date 2023-03-28
 *
 * @details test for interpreter dll
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
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
#include "testcase.hpp"
#include "tools/printcsvaluemap.hpp"

typedef CSModelObject *(*CreateModelObjectFun)();
typedef void (*DestroyMemoryFun)(void *mem, bool is_array);

int main() {
    using namespace rulejit;
    using CSValueMap = std::unordered_map<std::string, std::any>;
    std::string lib_path_ = "cq_interpreter.dll";

#ifdef _WIN32
    auto hmodule = LoadLibraryExA(lib_path_.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    void *hmodule = dlopen(lib_path_.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
    if (!hmodule) {
        std::cout << "load "
                  << "cq_interpreter"
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

    CSModelObject *model_obj_, *engine;

    // model_obj_ = create_obj_();
    // if (nullptr == model_obj_) {
    //     std::cerr << "create model error" << std::endl;
    //     return -1;
    // }
    // engine = model_obj_;
    // // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    // engine->SetLogFun([](const std::string &msg, uint32_t type) {});
    // engine->Init(CSValueMap{{"filePath", std::string(__PROJECT_ROOT_PATH "/doc/test_xml/BVR1.0.xml")}});
    // for (auto &&input : inputs) {
    //     std::cout << std::endl;
    //     engine->SetInput(input);
    //     engine->Tick(0.02);
    //     printCSValueMap(*(engine->GetOutput()));
    // }

    // model_obj_ = create_obj_();
    // if (nullptr == model_obj_) {
    //     std::cerr << "create model error" << std::endl;
    //     return -1;
    // }
    // engine = model_obj_;
    // // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    // engine->SetLogFun([](const std::string &msg, uint32_t type) {});
    // engine->Init(CSValueMap{{"filePath", std::string(__PROJECT_ROOT_PATH "/doc/test_xml/WVR1.0(1).xml")}});
    // for (auto &&input : inputs2) {
    //     std::cout << std::endl;
    //     input.emplace("flag", (double)1);
    //     engine->SetInput(input);
    //     engine->Tick(0.02);
    //     printCSValueMap(*(engine->GetOutput()));
    // }

    // model_obj_ = create_obj_();
    // if (nullptr == model_obj_) {
    //     std::cerr << "create model error" << std::endl;
    //     return -1;
    // }
    // engine = model_obj_;
    // // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    // engine->SetLogFun([](const std::string &msg, uint32_t type) {});
    // engine->Init(CSValueMap{});
    // for (auto &&input : inputs3) {
    //     std::cout << std::endl;
    //     input.emplace("flag", (double)1);
    //     engine->SetInput(input);
    //     engine->Tick(0.02);
    //     printCSValueMap(*(engine->GetOutput()));
    // }

    auto tmp = CSValueMap{
        {"A_output",
         CSValueMap{
             {"Longitude", double(0)},
             {"Latitude", double(10)},
             {"Altitude", double(500)},
             {"Speed", double(300)},
             {"Roll", double(0)},
             {"Pitch", double(0)},
             {"Yaw", double(0)},
         }},
        {"T_output",
         CSValueMap{
             {"Longitude", double(0)},
             {"Latitude", double(12)},
             {"Altitude", double(450)},
             {"Speed", double(350)},
             {"Roll", double(0)},
             {"Pitch", double(0)},
             {"Yaw", double(0)},
         }},
        {"radar", bool(false)},
        {"missile", bool(false)},
        {"DT",
         CSValueMap{
             {"DRmax", double(50000)},
             {"DMmax", double(40000)},
             {"DMmin", double(10000)},
             {"DMKmax", double(30000)},
             {"DMKmin", double(20000)},
             {"phiRmax", double(70)},
             {"phiMmax", double(50)},
             {"phiMK", double(30)},
             {"DNmax", double(10000)},
             {"DNmin", double(2000)},
             {"DNok", double(5000)},
             {"phiNmax", double(40)},
         }},
        {"DA",
         CSValueMap{
             {"DRmax", double(50000)},
             {"DMmax", double(40000)},
             {"DMmin", double(10000)},
             {"DMKmax", double(30000)},
             {"DMKmin", double(20000)},
             {"phiRmax", double(70)},
             {"phiMmax", double(50)},
             {"phiMK", double(30)},
             {"DNmax", double(10000)},
             {"DNmin", double(2000)},
             {"DNok", double(5000)},
             {"phiNmax", double(40)},
         }},
        {"flag", bool(true)},
    };
    model_obj_ = create_obj_();
    if (nullptr == model_obj_) {
        std::cerr << "create model error" << std::endl;
        return -1;
    }
    engine = model_obj_;
    // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    engine->SetLogFun([](const std::string &msg, uint32_t type) {});
    engine->Init(CSValueMap{{"filePath", std::string(__PROJECT_ROOT_PATH "/doc/test_xml/rule(2).xml")}});
    engine->SetInput(tmp);
    engine->Tick(0.02);
    printCSValueMap(*(engine->GetOutput()));

    return 0;
}