#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace parsergen {

struct TransCondition {
    // s=="" means not eat input
    // if symbol, s=symbol; else s=TokenType
    std::string s;
};

struct FSMNode {
    inline static std::vector<size_t> empty{};
    std::map<std::string, std::vector<size_t>> succ;
    std::vector<size_t>& shift(){
        return accept("");
    }
    std::vector<size_t> accept(const std::string& s){
        if(auto it = succ.find(s); it!=succ.end()){
            return it->second;
        }
        return empty;
    }
};



// bool canAccept(const std::string& s)override{
// }
// int accept(const std::string& s, int currState)override{
// }

} // namespace parsergen