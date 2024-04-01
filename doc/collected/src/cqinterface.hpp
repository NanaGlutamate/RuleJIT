#pragma once

#include <any>
#include <exception>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

enum class CSInstanceState {
    IS_UNSPECIFIED = 0,
    IS_CREATED = 1,
    IS_INITIALIZED = 2,
    IS_RUNNING = 3,
    IS_STOPPED = 4,
    IS_DESTROYED = 5,
    IS_ERROR = 6
};

class CSModelObject {
  public:
    virtual bool Init(const std::unordered_map<std::string, std::any> &value) = 0;
    virtual bool Tick(double time) = 0;
    virtual bool SetInput(const std::unordered_map<std::string, std::any> &value) = 0;
    virtual std::unordered_map<std::string, std::any> *GetOutput() = 0;
  public:
    void SetState(CSInstanceState state) { state_ = state; }
    CSInstanceState GetState(void) { return state_; }
    void SetID(uint64_t id) { id_ = id; }
    uint64_t GetID() const { return id_; }
    void SetModelID(const std::string &model_id) { model_id_ = model_id; }
    const std::string &GetModelID() const { return model_id_; }
    void SetInstanceName(const std::string &instance_name) { instance_name_ = instance_name; }
    const std::string &GetInstanceName() const { return instance_name_; }
    void SetForceSideID(uint16_t force_side_id) { force_side_id_ = force_side_id; }
    uint16_t GetForceSideID() const { return force_side_id_; }
  protected:
    CSInstanceState state_ = CSInstanceState::IS_UNSPECIFIED;
    uint64_t id_;
    std::string model_id_;
    std::string instance_name_;
    uint16_t force_side_id_;
  protected:
    void WriteLog(const std::string &msg, uint32_t level = 0) { log_(msg, level); }
    void WriteKeyMessage(const std::vector<std::any> &msgs) { params_.emplace("KeyMessages", msgs); }
    void WriteTopic(const std::string &topic_name, const std::unordered_map<std::string, std::any> &topic_data) {
        std::unordered_map<std::string, std::any> topic;
        topic["TopicName"] = topic_name;
        topic["Params"] = topic_data;
        com_cb_("DirectWriteTopic", topic);
    }
    void CreateEntity(const std::string &model_id, const std::string &instance_name, uint64_t instance_id,
                      uint16_t forceside_id, std::unordered_map<std::string, std::any> &params) {
        std::unordered_map<std::string, std::any> basic_params = {
            {"InstanceName", instance_name}, {"ID", instance_id}, {"ModelID", model_id}, {"ForceSideID", forceside_id}};
        basic_params.merge(params);
        com_cb_("CreateEntity", basic_params);
    }
  protected:
    std::string CommonCallBack(const std::string &fun_name, const std::unordered_map<std::string, std::any> &params) {
        return com_cb_(fun_name, params);
    }
  public:
    void SetLogFun(std::function<void(const std::string &, uint32_t)> log) { log_ = log; }
    void SetCommonCallBack(
        std::function<std::string(const std::string &, const std::unordered_map<std::string, std::any> &)> com_cb) {
        com_cb_ = com_cb;
    }
  protected:
    std::function<void(const std::string &, uint32_t)> log_;
    std::function<std::string(const std::string &, const std::unordered_map<std::string, std::any> &)> com_cb_;
    std::unordered_map<std::string, std::any> params_;
};
