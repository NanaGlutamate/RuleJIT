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


/**
 * @brief interface function regulated by CQ platform
 * 
 */
extern "C" __declspec(dllexport) CSModelObject *__stdcall CreateModelObject();

/**
 * @brief interface function regulated by CQ platform
 * 
 */
extern "C" __declspec(dllexport) void __stdcall DestroyMemory(void *mem, bool is_array);

/**
 * @brief main class to interact with CQ platform
 *
 */
class RuleEngine : public CSModelObject {
  public:

    /**
     * @brief init the rule engin model, if "filePath" is set in value, load the rule file;
     * else load the rule file from the same directory of the dll named "rule.xml"
     *
     * @param value the init value
     * @return bool true if success
     */
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) override;

    /**
     * @brief calculate the rule model
     * 
     * @param time time between two ticks
     * @return bool, true if no errors
     */
    virtual bool Tick(double time) override;

    /**
     * @brief set input value to rule set model
     * 
     * @param value input value
     * @return bool, true if no errors
     */
    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) override;

    /**
     * @brief get output of rule set model
     * 
     * @return std::unordered_map<std::string, std::any>* output value
     */
    virtual std::unordered_map<std::string, std::any> *GetOutput() override;
  
  protected:
    void WriteLog(const std::string &msg, uint32_t level = 0) {
        if(!enableLog){
            return;
        }
        CSModelObject::WriteLog(msg, level);
    }

  private:
    rulejit::cq::RuleSetEngine engine;
    bool enableLog;
};
