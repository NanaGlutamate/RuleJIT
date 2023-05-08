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
#include <iostream>

#include "RuleEngine.h"
#include "tools/seterror.hpp"
#include "tools/showmsg.hpp"

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

std::vector<std::unordered_map<std::string, std::any>> input, output;

} // namespace

bool RuleEngine::Init(const std::unordered_map<std::string, std::any> &value) {
    std::string filePath;
    if (auto it = value.find("filePath"); it != value.end()) {
        filePath = std::any_cast<std::string>(it->second);
    } else {
        auto library_dir_ = getLibDir();
        filePath = library_dir_ + "rule.xml";
    }
    if (!log_) {
        SetLogFun([](const std::string& msg, int level) {
            std::cout << msg;
        });
    }
    if(!std::filesystem::exists(filePath)){
        WriteLog(std::format("Init RuleEngine error: file {} not exists", filePath), 4);
        error(std::format("Init RuleEngine error: file {} not exists", filePath));
        return false;
    }
    try {
        engine.buildFromFile(filePath);
    } catch (std::exception &e) {
         WriteLog(std::string("Init RuleEngine Error: \n") + e.what(), 4);
         return false;
    }
    for (auto& msg : rulejit::debugMessages) {
        WriteLog(std::move(msg), 1);
    }
    rulejit::debugMessages.clear();
    engine.init();
    return true;
}

bool RuleEngine::Tick(double time) {
    try {
        engine.tick();
    }
    catch (std::exception& e) {
        WriteLog(std::string("RuleEngine Tick Error: \n") + e.what(), 4);
        return false;
    }
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

CSModelObject *__stdcall CreateModelObject() {
    CSModelObject *model = new RuleEngine();
    return model;
}

void __stdcall DestroyMemory(void *mem, bool is_array) {
    if (is_array) {
        delete[] ((RuleEngine *)mem);
    } else {
        delete ((RuleEngine *)mem);
    }
}