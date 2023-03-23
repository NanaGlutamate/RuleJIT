#include <format>
#include <string>

#define CarPhyModel_EXPORTS

#include "RuleEngine.h"

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
    try {
        engine.buildFromFile(filePath);
    } catch (std::exception &e) {
        WriteLog(std::string("Init RuleEngine error: ") + e.what(), 1);
        return false;
    }
    if(!log_){
        SetLogFun([](const std::string &msg, int level) {});
    }
    engine.init();
    return true;
}

bool RuleEngine::Tick(double time) {
    engine.tick();
    WriteLog("RuleEngine model Tick", 1);
    return true;
}

bool RuleEngine::SetInput(const std::unordered_map<std::string, std::any> &value) {
    engine.setInput(value);
    WriteLog("RuleEngine model SetInput", 1);
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
    WriteLog("RuleEngine model GetOutput", 1);
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