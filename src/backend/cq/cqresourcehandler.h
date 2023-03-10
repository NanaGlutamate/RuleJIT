#pragma once

#include <any>
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace rulejit {

namespace cq {

struct ResourceHandler {
    using CSValueMap = std::unordered_map<std::string, std::any>;
    CSValueMap input;
    CSValueMap output;
    CSValueMap cache;
    std::vector<std::any> resource;
    void SetInput(const CSValueMap &v) { input = v; }
    CSValueMap *GetOutput() { resource.clear(); return &output; }
};

} // namespace cq

} // namespace rulejit