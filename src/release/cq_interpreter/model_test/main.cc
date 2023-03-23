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
        if (v.type() == typeid(int8_t)) {
            cout << std::any_cast<int8_t>(v);
        } else if (v.type() == typeid(uint8_t)) {
            cout << std::any_cast<uint8_t>(v);
        } else if (v.type() == typeid(int16_t)) {
            cout << std::any_cast<int16_t>(v);
        } else if (v.type() == typeid(uint16_t)) {
            cout << std::any_cast<uint16_t>(v);
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
            cout << "{";
            bool start_ = false;
            for (auto &&item : tmp) {
                if (!start_) {
                    start_ = true;
                } else {
                    cout << ", ";
                }
                printCSValueMap(std::any_cast<std::unordered_map<std::string, std::any>>(item));
            }
            cout << "}";
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
    engine->Init(CSValueMap{{"filePath", std::string(__PROJECT_ROOT_PATH"/doc/xml_design/example1.0.xml")}});

    for (int i = 0; i < 20; i++) {
        printCSValueMap(*(engine->GetOutput()));
        std::cout << std::endl;
        engine->SetInput(CSValueMap{{"Input1", double(1)}});
        engine->Tick(0.02);
    }

    return 0;
}