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
#include <chrono>
#include <filesystem>
#include <ranges>
#include <fstream>

#include "../csmodel_base/csmodel_base.h"
#include "testcase.hpp"
#include "tools/printcsvaluemap.hpp"
#include "tools/parseany.hpp"
#include "tools/anyprocess.hpp"

CSModelObject *loadModel(const std::string &lib_path_) {
#ifdef _WIN32
    auto hmodule = LoadLibraryExA(lib_path_.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else  // _WIN32
    void *hmodule = dlopen(lib_path_.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif // _WIN32
    if (!hmodule) {
        std::cout << "load "
                  << "cq_interpreter"
                  << " failed" << std::endl;
        return nullptr;
    }

#ifdef _WIN32
    auto create_obj_ = (auto (*)()->CSModelObject *)GetProcAddress(hmodule, "CreateModelObject");
    auto destroy_obj_ = (auto (*)(void *, bool)->void)GetProcAddress(hmodule, "DestroyMemory");
#else  // _WIN32
    auto create_obj_ = (auto (*)()->CSModelObject *)dlsym(hmodule, "CreateModelObject");
    auto destroy_obj_ = (auto (*)(void *, bool)->void)dlsym(hmodule, "DestroyMemory");
#endif // _WIN32
    if (!create_obj_ || !destroy_obj_) {
#ifdef _WIN32
        if (!FreeLibrary(hmodule))
#else  // _WIN32
        if (!dlclose(hmodule))
#endif // _WIN32
            std::cout << "release dll error" << std::endl;
        return nullptr;
    }
    return create_obj_();
}

// air2v2 / car1v1
#define TARGET "car1v1"

int main() {
    using namespace std;
    using namespace views;
    using namespace rapidxml;
    using namespace chrono;
    using namespace tools::myany;
    using CSValueMap = std::unordered_map<std::string, std::any>;

    CSModelObject *interpreter = loadModel("cq_interpreter.dll");
    CSModelObject *cppengine = loadModel(__PROJECT_ROOT_PATH "/doc/collected/" TARGET ".dll");

    interpreter->SetLogFun([](const auto &, auto) {});
    cppengine->SetLogFun([](const auto &, auto) {});
    interpreter->Init(CSValueMap{{"filePath", std::string(__PROJECT_ROOT_PATH "/doc/collected/" TARGET "rule.xml")}});

    std::ifstream file(__PROJECT_ROOT_PATH "/doc/collected/" TARGET ".xml");
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    buffer.push_back(0);
    xml_document<> doc;
    doc.parse<parse_default>(buffer.data());
    vector<any> inputdata = any_cast<vector<any>>(parseXML(doc.first_node("data")->first_node("input")->first_node()));

    std::vector<double> time_int, time_cpp;
    constexpr int test_time = 10;
    constexpr int slice = 5 + 1;

    //for (auto&& i : inputdata) {
    //    auto p = std::any_cast<CSValueMap>(&i);
    //    interpreter->SetInput(*p);
    //    interpreter->Tick(0.05);
    //    auto p1 = interpreter->GetOutput();
    //    cppengine->SetInput(*p); 
    //    cppengine->Tick(0.05);
    //    auto p2 = cppengine->GetOutput();
    //    printCSValueMap(*p);
    //    std::cout << "\noutput\n";
    //    printCSValueMap(*p1);
    //    std::cout << "\n\n";
    //    printCSValueMap(*p2);
    //    if (!anyEqual(*p1, *p2)) {
    //        return 0;
    //    }
    //}

    for (auto cnt : iota(1, slice)) {
        auto ticks = test_time * inputdata.size() * cnt / slice;
        auto start = high_resolution_clock::now();
        for (auto _ : iota(0, test_time)) {
            for (auto&& i : inputdata | take(inputdata.size() * cnt / slice)) {
                auto p = std::any_cast<CSValueMap>(&i);
                interpreter->SetInput(*p);
                interpreter->Tick(0.05);
                interpreter->GetOutput();
            }
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        time_int.push_back(double(duration.count()) / ticks);
    }

    for (auto cnt : iota(1, slice)) {
        auto ticks = test_time * inputdata.size() * cnt / slice;
        auto start = high_resolution_clock::now();
        for (auto _ : iota(0, test_time)) {
            for (auto &&i : inputdata | take(inputdata.size() * cnt / slice)) {
                auto p = std::any_cast<CSValueMap>(&i);
                cppengine->SetInput(*p);
                cppengine->Tick(0.05);
                cppengine->GetOutput();
            }
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        time_cpp.push_back(double(duration.count()) / ticks);
    }

    // cout << "interpreter:\t";
    cout << "data1 = [";
    for (auto i : time_int) {
        cout << i << ",";
    }
    cout << "]" << endl;

    // cout << "generated cpp:\t";
    cout << "data2 = [";
    for (auto i : time_cpp) {
        cout << i << ",";
    }
    cout << "]" << endl;

    // auto tmp = CSValueMap{
    //     {"A_output",
    //      CSValueMap{
    //          {"Longitude", (double)106},
    //          {"Latitude", (double)36},
    //          {"Altitude", (double)500},
    //          {"vx", (double)300},
    //          {"vy", (double)0},
    //          {"vz", (double)0},
    //          {"Speed", (double)300},
    //          {"Roll", (double)0},
    //          {"Pitch", (double)0},
    //          {"Yaw", (double)0},
    //      }},
    //     {"T_output",
    //      CSValueMap{
    //          {"Longitude", (double)106.1},
    //          {"Latitude", (double)36.2},
    //          {"Altitude", (double)500},
    //          {"vx", (double)300},
    //          {"vy", (double)0},
    //          {"vz", (double)0},
    //          {"Speed", (double)300},
    //          {"Roll", (double)0},
    //          {"Pitch", (double)0},
    //          {"Yaw", (double)0},
    //      }},
    // };
    // model_obj_ = create_obj_();
    // if (nullptr == model_obj_) {
    //     std::cerr << "create model error" << std::endl;
    //     return -1;
    // }
    // engine = model_obj_;
    // // engine->SetLogFun([](const std::string &msg, uint32_t type) { std::cout << msg << std::endl;});
    // engine->SetLogFun([](const std::string &msg, uint32_t type) {});
    // try {
    //     // engine->Init(CSValueMap{ {"filePath", std::string(__PROJECT_ROOT_PATH "/doc/test_xml/rule_err.xml")} });
    //     engine->Init(CSValueMap{ {"filePath", std::string(__PROJECT_ROOT_PATH "/doc/test_xml/car_rule.xml")} });
    //     engine->SetInput(CSValueMap{});
    //     engine->Tick(0.02);
    // } catch (std::logic_error& e) {
    //     std::cout << e.what();
    //     return 0;
    // }
    // tools::myany::printCSValueMap(*(engine->GetOutput()));

    return 0;
}