#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <atlbase.h>
#include <atlwin.h>
#else
#include <dlfcn.h>
#endif
#include <string>

#include "../csmodel_base/csmodel_base.h"
#include "backend/cq/cqrulesetengine.h"

extern "C" __declspec(dllexport) CSModelObject* __stdcall CreateModelObject();
extern "C" __declspec(dllexport) void __stdcall DestroyMemory(void *mem, bool is_array);

class RuleEngine : public CSModelObject {
  public:
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) override;

    virtual bool Tick(double time) override;

    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) override;

    virtual std::unordered_map<std::string, std::any> *GetOutput() override;

  private:
    rulejit::cq::RuleSetEngine engine;
};
