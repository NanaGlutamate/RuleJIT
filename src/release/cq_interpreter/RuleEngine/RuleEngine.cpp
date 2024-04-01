/**
 * @file RuleEngine.cpp
 * @author djw
 * @brief CQ/Interpreter/Rule engine
 * @date 2023-03-27
 * 
 * @details Includes platform interface function defines and member function defines of RuleEngine
 * 
 * @see RuleEngine
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#include <ranges>
#ifdef _WIN32
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>
#else
#include <dlfcn.h>
#endif
#include <format>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "RuleEngine.h"
#include "defines/marco.hpp"
#include "tools/seterror.hpp"
#include "tools/showmsg.hpp"
#include "tools/parseany.hpp"

namespace {

using namespace std;

string getLibDir() {
    string library_dir_;
#ifdef _WIN32
    HMODULE module_instance = _AtlBaseModule.GetModuleInstance();
    char dll_path[MAX_PATH] = {0};
    GetModuleFileNameA(module_instance, dll_path, _countof(dll_path));
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath_s(dll_path, drive, dir, fname, ext);
    library_dir_ = drive + std::string(dir) + "/";
#else
    Dl_info dl_info;
    CSModelObject *(*p)() = &CreateModelObject;
    if (0 != dladdr((void *)(p), &dl_info)) {
        library_dir_ = std::string(dl_info.dli_fname);
        library_dir_ = library_dir_.substr(0, library_dir_.find_last_of('/'));
        library_dir_ += "/";
    }
#endif
    return library_dir_;
}

} // namespace

bool RuleEngine::Init(const std::unordered_map<std::string, std::any> &value) {
    std::string filePath;
    if (auto it = value.find("filePath"); it != value.end()) {
        filePath = std::any_cast<std::string>(it->second);
    } else {
        auto library_dir_ = getLibDir();
        filePath = library_dir_ + "rule.xml";
    }
    if (auto it = value.find("enableLog"); it != value.end()) {
        enableLog = std::any_cast<bool>(it->second);
    } else {
        enableLog = true;
    }
    if (!log_) {
        SetLogFun([](const std::string& msg, int level) {
            std::cout << msg;
        });
    }
    if(!std::filesystem::exists(filePath)){
        WriteLog(std::format("Init RuleEngine error: file {} not exists", filePath), 4);
        return false;
    }
    try {
        engine.buildFromFile(filePath);
    } catch (std::exception &e) {
        WriteLog(std::string("Init RuleEngine Error: \n") + e.what(), 5);
        return false;
    }
    for (auto& msg : rulejit::debugMessages) {
        WriteLog(std::move(msg), 1);
    }
    rulejit::debugMessages.clear();
    engine.init();
    engine.setInput(value);
    return true;
}

bool RuleEngine::Tick(double time) {
    try {
        if (autoCollectedArray.size()) {
            std::unordered_map<std::string, std::any> tmp;
            for(auto& [k, v] : autoCollectedArray){
                tmp.emplace(std::move(k), std::move(v));
            }
            autoCollectedArray.clear();
            engine.setInput(tmp);
        }
        engine.tick();
    }
    catch (std::exception& e) {
        WriteLog(std::string("RuleEngine Tick Error: \n") + e.what(), 4);
        return false;
    }

// #define __RULEENGINE_RECORD

#ifdef __RULEENGINE_RECORD
    static RuleEngine& logger = *this;
    static size_t cnt = 0;
    static std::vector<std::any> input, output;
    if(&logger == this){
        cnt++;
        input.push_back(engine.getInput());
        if(cnt%100==0){
            // ofstream f{ __PROJECT_ROOT_PATH"/bin/collected/data.xml" };
            ofstream f{ "D:/data.xml" };
            f << "<data><input>";
            f << tools::myany::printAnyToString<tools::myany::XMLFormat>(input);
            f << "</input></data>";
        }
    }
#endif // __RULEENGINE_RECORD

    std::string info = "RuleEngine model Hit rules: " + (engine.hitRules() | std::views::transform([](int x) {return std::to_string(x); }) | tools::mystr::join(", ")) + "\n\n";
    info += std::format("RuleEngine model Cache: {}\n\n", tools::myany::printCSValueMapToString(engine.getCache()));
    info += std::format("RuleEngine model Input: {}\n\n",
        tools::myany::printCSValueMapToString(engine.getInput()));
    info += std::format("RuleEngine model Output: {}\n\n", tools::myany::printCSValueMapToString(*engine.getOutput()));
    WriteLog(info, 1);
    return true;
}

bool RuleEngine::SetInput(const std::unordered_map<std::string, std::any> &value) {
    engine.setInput(value);
    for(auto&& [k, v] : value) {
        auto it = engine.dataStorage.metaInfo.varType.find(k);
        if (it == engine.dataStorage.metaInfo.varType.end()){
            continue;
        }
        if (it->second.ends_with("[]") && v.type() != typeid(std::vector<std::any>)) {
            autoCollectedArray[k].push_back(v);
        }
    }
    return true;
}

std::unordered_map<std::string, std::any> *RuleEngine::GetOutput() {
    state_ = CSInstanceState::IS_RUNNING;
    auto &params_ = *(engine.getOutput());
    params_.emplace("ForceSideID", GetForceSideID());
    params_.emplace("ModelID", GetModelID());
    params_.emplace("InstanceName", GetInstanceName());
    params_.emplace("ID", GetID());
    params_.emplace("State", uint16_t(GetState()));
    return &params_;
}

extern "C" CSModelObject *__stdcall CreateModelObject() {
    CSModelObject *model = new RuleEngine();
    return model;
}

extern "C" void __stdcall DestroyMemory(void *mem, bool is_array) {
    if (is_array) {
        delete[] ((RuleEngine *)mem);
    } else {
        delete ((RuleEngine *)mem);
    }
}