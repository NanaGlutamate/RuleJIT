/**
 * @file RuleEngine.h
 * @author djw
 * @brief CQ/Interpreter/Rule engine
 * @date 2023-03-27
 * 
 * @details Header of rule engine
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <string>

#include "../csmodel_base/csmodel_base.h"
#include "backend/cq/cqrulesetengine.h"

extern "C" __declspec(dllexport) CSModelObject* __stdcall CreateModelObject();
extern "C" __declspec(dllexport) void __stdcall DestroyMemory(void *mem, bool is_array);

/**
 * @ingroup interpreter
 * @brief main class to interact with CQ platform
 * 
 */
class RuleEngine : public CSModelObject {
  public:
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) override;

    virtual bool Tick(double time) override;

    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) override;

    virtual std::unordered_map<std::string, std::any> *GetOutput() override;

  private:
    rulejit::cq::RuleSetEngine engine;
};
