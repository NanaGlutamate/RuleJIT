#pragma once

#include <list>
#include <memory>
#include <stack>
#include <vector>
#include <unordered_map>
#include <any>
#include <string>

namespace rulejit {

namespace cq {

struct ResourceHandler{
    using CSValueMap = std::unordered_map<std::string, std::any>;
    CSValueMap input;
    CSValueMap output;
    CSValueMap cache;
};

}

}