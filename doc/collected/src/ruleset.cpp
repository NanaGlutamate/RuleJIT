#include <functional>
#include <map>
#include <exception>
#include <any>
#include <string>
#include <unordered_map>
#include <vector>

#include "typedef.hpp"
#include "funcdef.hpp"
#include "ruleset.hpp"
#include "cqinterface.hpp"

extern "C" __declspec(dllexport) CSModelObject* __stdcall CreateModelObject();
extern "C" __declspec(dllexport) void __stdcall DestroyMemory(void *mem, bool is_array);

class RuleEngine : public CSModelObject {
  public:
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) override{
        engine.Init();
        if(!log_)SetLogFun([](const std::string &msg, uint32_t type) {});
        return true;
    };
    virtual bool Tick(double time) override{
        engine.Tick();
        WriteLog("RuleEngine model Tick", 1);
        return true;
    };
    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) override{
        engine.SetInput(value);
        WriteLog("RuleEngine model SetInput", 1);
        return true;
    };
    virtual std::unordered_map<std::string, std::any> *GetOutput() override{
        state_ = CSInstanceState::IS_RUNNING;
        auto &params_ = *(engine.GetOutput());
        params_.emplace("ForceSideID", GetForceSideID());
        params_.emplace("ModelID", GetModelID());
        params_.emplace("InstanceName", GetInstanceName());
        params_.emplace("ID", GetID());
        params_.emplace("State", uint16_t(GetState()));
        WriteLog("RuleEngine model GetOutput", 1);
        return &params_;
    };
  private:
    ruleset::RuleSet engine;
};

CSModelObject* __stdcall CreateModelObject() {
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

