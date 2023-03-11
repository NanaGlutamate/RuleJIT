#pragma once

#include <any>
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "tools/myassert.hpp"

namespace rulejit::cq {

inline static std::set<std::string> baseData{
    "bool",  "int8",   "uint8",   "int16",   "uint16",   "int32",  "uint32",
    "int64", "uint64", "float32", "float64", "float128", "string",
};

struct ResourceHandler {
    using CSValueMap = std::unordered_map<std::string, std::any>;
    CSValueMap input;
    CSValueMap output;
    CSValueMap cache;
    std::vector<std::string> inputVar, outputVar, cacheVar;
    // name -> type
    std::unordered_map<std::string, std::string> varType;
    // type -> {member name, member type}
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> typeDefines;
    void Init() {
        // fill input, output and cache
        for (auto &&s : inputVar) {
            input[s] = makeTypeEmptyInstance(varType[s]);
        }
        for(auto&& s : outputVar){
            output[s] = makeTypeEmptyInstance(varType[s]);
        }
        for (auto &&s : cacheVar) {
            cache[s] = makeTypeEmptyInstance(varType[s]);
        }
    }
    void SetInput(const CSValueMap &v) {
        for (auto &&[key, value] : v) {
            input[key] = value;
        }
    }
    CSValueMap *GetOutput() { return &output; }
    void Tick() {
        writeBack();
        buffer.clear();
        relation.clear();
    }

    size_t take(const std::string &s) {
        if (managedString.contains(s)) {
            return managedString[s];
        }
        buffer.emplace_back(s, "string");
        managedString[s] = buffer.size() - 1;
        return buffer.size() - 1;
    }
    bool isString(size_t index) { return std::get<0>(buffer[index]).type() == typeid(std::string); }
    size_t readIn(const std::string &s) {
        if (auto it = bufferMap.find(s); it != bufferMap.end()) {
            return it->second;
        }
        if (auto it1 = input.find(s); it1 != input.end()) {
            buffer.emplace_back(it1->second, varType[s]);
        } else if (auto it2 = cache.find(s); it2 != cache.end()) {
            buffer.emplace_back(it2->second, varType[s]);
        } else if (auto it3 = output.find(s); it2 != output.end()) {
            buffer.emplace_back(it3->second, varType[s]);
        } else {
            throw std::logic_error(std::string("unknown token: ") + s);
        }
        bufferMap[s] = buffer.size() - 1;
        return buffer.size() - 1;
    }
    void writeBack() {
        for (auto &&[name, ind] : bufferMap) {
            if (auto it = output.find(name); it != output.end()) {
                it->second = assemble(ind);
            } else if (auto it = cache.find(name); it != cache.end()) {
                it->second = assemble(ind);
            }
        }
    }
    void assign(size_t tar, size_t dst) {
        my_assert(std::get<1>(buffer[tar]) == std::get<1>(buffer[dst]),
                  "assignment of different types are not allowed");
        std::get<0>(buffer[tar]) = std::get<0>(buffer[dst]);
    }
    size_t arrayAccess(size_t base, size_t index) {
        if (auto it = relation[base].find(std::to_string(index)); it != relation[base].end()) {
            return it->second;
        }
        if (!isArray(std::get<1>(buffer[base]))) {
            throw std::logic_error(std::format("type \"{}\" is not an array", std::get<1>(buffer[base])));
        }
        auto tmp = std::any_cast<std::vector<std::any>>(std::get<0>(buffer[base]))[index];
        auto baseType = std::get<1>(buffer[base]);
        buffer.emplace_back(tmp, arrayElementType(baseType));
        relation[base].emplace(std::to_string(index), buffer.size() - 1);
        return buffer.size() - 1;
    }
    size_t memberAccess(size_t base, const std::string &token) {
        if (auto it = relation[base].find(token); it != relation[base].end()) {
            return it->second;
        }
        if (isArray(std::get<1>(buffer[base]))) {
            throw std::logic_error(std::format("type \"{}\" is an array", std::get<1>(buffer[base])));
        }
        auto tmp = std::any_cast<CSValueMap>(std::get<0>(buffer[base]))[token];
        auto baseType = std::get<1>(buffer[base]);
        auto it = typeDefines[baseType].find(token);
        if (it == typeDefines[baseType].end()) {
            throw std::logic_error(std::format("type \"{}\" has no member {}", std::get<1>(buffer[base]), token));
        }
        auto newType = it->second;
        buffer.emplace_back(tmp, newType);
        relation[base].emplace(token, buffer.size() - 1);
        return buffer.size() - 1;
    }
    size_t arrayLength(size_t index) {
        auto tmp = std::any_cast<std::vector<std::any>>(std::get<0>(buffer[index]));
        return tmp.size();
    }
    bool isBaseType(size_t index) {
        try {
            readValue(index);
            return true;
        } catch (...) {
            return false;
        }
    }
    double readValue(size_t index) {
        auto &v = std::get<0>(buffer[index]);
        if (v.type() == typeid(int8_t)) {
            return std::any_cast<int8_t>(v);
        } else if (v.type() == typeid(uint8_t)) {
            return std::any_cast<uint8_t>(v);
        } else if (v.type() == typeid(int16_t)) {
            return std::any_cast<int16_t>(v);
        } else if (v.type() == typeid(uint16_t)) {
            return std::any_cast<uint16_t>(v);
        } else if (v.type() == typeid(int32_t)) {
            return std::any_cast<int32_t>(v);
        } else if (v.type() == typeid(uint32_t)) {
            return std::any_cast<uint32_t>(v);
        } else if (v.type() == typeid(int64_t)) {
            return std::any_cast<int64_t>(v);
        } else if (v.type() == typeid(uint64_t)) {
            return std::any_cast<uint64_t>(v);
        } else if (v.type() == typeid(float)) {
            return std::any_cast<float>(v);
        } else if (v.type() == typeid(double)) {
            return std::any_cast<double>(v);
        } else if (v.type() == typeid(std::string)) {
            // TODO: redesign
            union Value{
                double value;
                size_t token;
            } tmp;
            tmp.token = index;
            return tmp.value;
        } else {
            throw std::logic_error(std::string("unknown type: ") + v.type().name());
        }
    }
    void writeValue(size_t index, double tar) {
        // TODO: string
        auto &v = std::get<0>(buffer[index]);
        if (v.type() == typeid(int8_t)) {
            v = (int8_t)(tar);
        } else if (v.type() == typeid(uint8_t)) {
            v = (uint8_t)(tar);
        } else if (v.type() == typeid(int16_t)) {
            v = (int16_t)(tar);
        } else if (v.type() == typeid(uint16_t)) {
            v = (uint16_t)(tar);
        } else if (v.type() == typeid(int32_t)) {
            v = (int32_t)(tar);
        } else if (v.type() == typeid(uint32_t)) {
            v = (uint32_t)(tar);
        } else if (v.type() == typeid(int64_t)) {
            v = (int64_t)(tar);
        } else if (v.type() == typeid(uint64_t)) {
            v = (uint64_t)(tar);
        } else if (v.type() == typeid(float)) {
            v = (float)(tar);
        } else if (v.type() == typeid(double)) {
            v = (double)(tar);
        }else if (v.type() == typeid(std::string)) {
            // TODO: redesign
            union Value{
                double value;
                size_t token;
            } tmp;
            tmp.value = tar;
            auto& real_tar = std::get<0>(buffer[reinterpret_cast<size_t>(tmp.token)]);
            if(real_tar.type() != typeid(std::string)){
                throw std::logic_error(std::string("only allow assign string to string"));
            }
            v = real_tar;
        } else {
            throw std::logic_error(std::string("unknown type: ") + v.type().name());
        }
    }
    bool isArray(const std::string &s) { return s.back() == ']'; }
    std::string arrayElementType(const std::string &s) {
        my_assert(isArray(s));
        std::string tmp = s;
        tmp.pop_back();
        tmp.pop_back();
        return tmp;
    }

  private:
    std::any makeTypeEmptyInstance(const std::string &type) {
        if (isArray(type)) {
            return std::vector<std::any>();
        }
        if (baseData.contains(type)) {
            if (type == "bool")
                return bool();
            if (type == "int8")
                return int8_t();
            if (type == "uint8")
                return uint8_t();
            if (type == "int16")
                return int16_t();
            if (type == "uint16")
                return uint16_t();
            if (type == "int32")
                return int32_t();
            if (type == "uint32")
                return uint32_t();
            if (type == "int64")
                return int64_t();
            if (type == "uint64")
                return uint64_t();
            if (type == "float32")
                return float();
            if (type == "float64")
                return double();
                if (type == "string")
                return std::string();
        }
        CSValueMap tmp;
        for (auto &&[name, type] : typeDefines[type]) {
            tmp[name] = makeTypeEmptyInstance(type);
        }
    }
    std::any assemble(size_t index) {
        auto [v, type] = buffer[index];
        if (baseData.contains(type)) {
            return v;
        }
        if (isArray(type)) {
            auto tmp = std::any_cast<std::vector<std::any>>(v);
            for (auto &&[name, ind] : relation[index]) {
                tmp[std::stoi(name)] = assemble(ind);
            }
            return tmp;
        } else {
            auto tmp = std::any_cast<CSValueMap>(v);
            for (auto &&[name, ind] : relation[index]) {
                tmp[name] = assemble(ind);
            }
            return tmp;
        }
    }
    std::map<std::string, size_t> managedString;
    // {value, type}
    std::vector<std::tuple<std::any, std::string>> buffer;
    // index -> {memberName, index}
    std::map<size_t, std::map<std::string, size_t>> relation;
    // name -> index
    std::map<std::string, size_t> bufferMap;
};

} // namespace rulejit::cq