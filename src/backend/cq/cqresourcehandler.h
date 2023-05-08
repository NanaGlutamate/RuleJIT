/**
 * @file cqresourcehandler.h
 * @author djw
 * @brief CQ/Interpreter/CQResourceHandler
 * @date 2023-03-27
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-27</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <any>
#include <format>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "frontend/ruleset/rulesetparser.h"
#include "tools/anyprocess.hpp"
#include "tools/myassert.hpp"
#include "tools/printcsvaluemap.hpp"
#include "tools/seterror.hpp"
#include "tools/stringprocess.hpp"

namespace rulejit::cq {

/**
 * @brief data structure for storing real data
 *
 */
struct DataStore {
    using CSValueMap = std::unordered_map<std::string, std::any>;
    DataStore() = default;
    DataStore(const DataStore &) = delete;
    CSValueMap input;
    CSValueMap output;
    CSValueMap cache;
    rulesetxml::RuleSetMetaInfo metaInfo;

    /**
     * @brief generate core dump
     *
     * @return std::string core dump
     */
    std::string dump() {
        return std::format("Input:\n{}\n\nOutput:\n{}\n\nCache:\n{}\n",
                           tools::mystr::autoIdent(tools::myany::printCSValueMapToString(input), 1),
                           tools::mystr::autoIdent(tools::myany::printCSValueMapToString(output), 1),
                           tools::mystr::autoIdent(tools::myany::printCSValueMapToString(cache), 1));
    }

    /**
     * @brief convert type name in XML to inner type name
     *
     * @attention will lose infomation about specific numerical type
     *
     * @param s xml type
     * @return std::string
     */
    static std::string XMLType2InnerType(const std::string &s) {
        if (s.ends_with("[]")) {
            return "[]" + XMLType2InnerType(s.substr(0, s.size() - 2));
        }
        if (rulesetxml::baseNumericalData.contains(s)) {
            return "f64";
        } else {
            return s;
        }
    }

    /**
     * @brief convert inner type name to XML type name
     *
     * @param s inner type
     * @return std::string xml type
     */
    static std::string innerType2XMLType(const std::string &s) {
        if (s.starts_with("[]")) {
            return innerType2XMLType(s.substr(2)) + "[]";
        }
        if (s == "f64") {
            return "float64";
        } else {
            return s;
        }
    }

    /**
     * @brief check all stored data to see if their type are as defined;
     *
     * @return std::string type check info, empty if no error
     */
    std::string genTypeCheckInfo() {
        std::string ret;
        // use inner class act as a recursible lambda to avoid private function used only once / y-combinator
        struct TypeChecker {
            rulesetxml::RuleSetMetaInfo &metaInfo;
            DataStore &dataStore;
            std::string check(const std::string &name, const std::string &type, std::any &real) {
                if (rulesetxml::baseData.contains(type)) {
                    // base type
                    if (dataStore.makeTypeEmptyInstance(type).type() != real.type()) {
                        return std::format("    variable \"{}\" is not of type \"{}\"\n", name, type);
                    }
                    return "";
                }
                if (dataStore.isArray(type)) {
                    // array
                    if (real.type() != typeid(std::vector<std::any>)) {
                        return std::format("    variable \"{}\" is not of type \"{}\"\n", name, type);
                    }
                    auto eleType = type.substr(0, type.size() - 2);
                    auto &realArray = std::any_cast<std::vector<std::any> &>(real);
                    std::string ret;
                    for (size_t i = 0; i < realArray.size(); ++i) {
                        ret += check(name + "[" + std::to_string(i) + "]", eleType, realArray[i]);
                    }
                    return std::move(ret);
                }
                // struct
                auto definedIt = metaInfo.typeDefines.find(type);
                if (definedIt == metaInfo.typeDefines.end()) {
                    return std::format("    unknown type \"{}\" found in variable \"{}\"\n", type, name);
                }
                if (real.type() != typeid(CSValueMap)) {
                    return std::format("    variable \"{}\" is not of type \"{}\"\n", name, type);
                }
                auto &realValue = std::any_cast<CSValueMap &>(real);
                std::string ret;
                for (auto &[eleName, eleType] : definedIt->second) {
                    auto eleIt = realValue.find(eleName);
                    if (eleIt == realValue.end()) {
                        return std::format("    variable \"{}\" is missing element \"{}\"\n", name, eleName);
                    }
                    ret += check(name + "." + eleName, eleType, eleIt->second);
                }
                return std::move(ret);
            }
        } typeChecker{metaInfo, *this};
        std::vector<std::tuple<std::string, std::reference_wrapper<CSValueMap>>> v{
            {"Input", input}, {"Output", output}, {"Cache", cache}};
        for (auto &[name, varTable] : v) {
            std::string detail;
            for (auto &[varName, varValue] : varTable.get()) {
                auto varTypeIt = metaInfo.varType.find(varName);
                if (varTypeIt == metaInfo.varType.end()) {
                    detail += std::format("    undefined variable \"{}\" found in {}\n", varName, name);
                } else {
                    detail += typeChecker.check(varName, varTypeIt->second, varValue);
                }
            }
            if (!detail.empty()) {
                ret += "Runtime Error in " + name + " Vars:\n" + detail;
            }
        }
        return ret;
    }

    /**
     * @brief init function, will fill input, output and cache
     * with legal empty instance.
     *
     */
    void Init() {
        // fill input, output and cache
        for (auto &&s : metaInfo.inputVar) {
            input[s] = makeTypeEmptyInstance(metaInfo.varType[s]);
        }
        for (auto &&s : metaInfo.outputVar) {
            output[s] = makeTypeEmptyInstance(metaInfo.varType[s]);
        }
        for (auto &&s : metaInfo.cacheVar) {
            cache[s] = makeTypeEmptyInstance(metaInfo.varType[s]);
        }
    }

    /**
     * @brief Set input value
     *
     * @param v input value
     */
    void SetInput(const CSValueMap &v) {
        for (auto &&[k, v] : v) {
            input[k] = v;
        }
    }

    /**
     * @brief Get the Outputs
     *
     * @return CSValueMap* pointer to output data
     */
    CSValueMap *GetOutput() { return &output; }

    /**
     * @brief check if a type is array type
     *
     * @param s type string need to be checked
     * @return bool
     */
    bool isArray(const std::string &s) { return s.back() == ']'; }

    /**
     * @brief get element type of an array type
     *
     * @param s array type string
     * @return std::string
     */
    std::string arrayElementType(const std::string &s) {
        my_assert(isArray(s));
        return s.substr(0, s.size() - 2);
    }

    /**
     * @brief generate an empty instance of a type
     *
     * @param type type string
     * @return std::any
     */
    std::any makeTypeEmptyInstance(const std::string &type) {
        if (isArray(type)) {
            return std::vector<std::any>();
        }
        if (rulesetxml::baseData.contains(type)) {
            if (type == "bool")
                return bool(false);
            if (type == "int8")
                return int8_t(0);
            if (type == "uint8")
                return uint8_t(0);
            if (type == "int16")
                return int16_t(0);
            if (type == "uint16")
                return uint16_t(0);
            if (type == "int32")
                return int32_t(0);
            if (type == "uint32")
                return uint32_t(0);
            if (type == "int64")
                return int64_t(0);
            if (type == "uint64")
                return uint64_t(0);
            if (type == "float32")
                return float(0);
            if (type == "float64")
                return double(0);
            if (type == "string")
                return std::string{};
        }
        CSValueMap tmp;
        auto it = metaInfo.typeDefines.find(type);
        if (it == metaInfo.typeDefines.end()) {
            error("undefined type: " + type);
        }
        for (auto &&[name, type] : it->second) {
            tmp[name] = makeTypeEmptyInstance(type);
        }
        return tmp;
    }
};

/**
 * @brief data structure for buffering data in DataStore,
 * and provide functions for CQInterpreter to access and modify data.
 *
 */
struct ResourceHandler {
    using CSValueMap = std::unordered_map<std::string, std::any>;
    DataStore &data;
    ResourceHandler(DataStore &data)
        : data(data), managedString(), buffer(), bufferMap(), originalValue(), relation(){};
    ResourceHandler(const ResourceHandler &) = delete;
    ResourceHandler(ResourceHandler &&) = delete;
    ResourceHandler &operator=(const ResourceHandler &) = delete;
    ResourceHandler &operator=(ResourceHandler &&) = delete;

    /**
     * @brief manage a string, if the string is already in the buffer, return the index of the string,
     *
     * @param s string need to be managed
     * @return size_t
     */
    size_t takeString(const std::string &s) {
        if (managedString.contains(s)) {
            return managedString[s];
        }
        buffer.emplace_back(s, "string");
        managedString[s] = buffer.size() - 1;
        return buffer.size() - 1;
    }

    /**
     * @brief get managed string through index
     *
     * @param index index of string
     * @return const std::string&
     */
    std::string getString(size_t index) {
        my_assert(isString(index), "not a string");
        return std::any_cast<std::string>(std::get<1>(buffer[index]));
    }

    /**
     * @brief check if a value is string
     *
     * @param index token which referring to the value
     * @return bool
     */
    bool isString(size_t index) { return std::get<1>(buffer[index]) == "string"; }

    /**
     * @brief read string from buffer
     *
     * @param index token which referring to the string
     * @return std::string
     */
    std::string readString(size_t index) { return std::any_cast<std::string>(std::get<0>(buffer[index])); }

    /**
     * @brief compare managed string
     *
     * @param v1 token which referring to the first string
     * @param v2 token which referring to the second string
     * @return true
     * @return false
     */
    bool stringComp(size_t v1, size_t v2) {
        my_assert(isString(v1) && isString(v2), "string comparison only allowed on string");
        return std::any_cast<std::string>(std::get<0>(buffer[v1])) ==
               std::any_cast<std::string>(std::get<0>(buffer[v2]));
    }

    /**
     * @brief read input, cache or output value into the buffer and return index of the value;
     * if the value already in the buffer, directly return the index;
     * if the value not a input, cache or output value, throw an exception
     *
     * @exception "unknown token: " + [name of var]
     *
     * @param s variable name
     * @return size_t token which referring to the value
     */
    size_t readIn(const std::string &s) {
        if (auto it = bufferMap.find(s); it != bufferMap.end()) {
            return it->second;
        }
        if (auto it1 = data.input.find(s); it1 != data.input.end()) {
            buffer.emplace_back(it1->second, data.metaInfo.varType[s]);
        } else if (auto it2 = data.cache.find(s); it2 != data.cache.end()) {
            buffer.emplace_back(it2->second, data.metaInfo.varType[s]);
        } else if (auto it3 = data.output.find(s); it3 != data.output.end()) {
            buffer.emplace_back(it3->second, data.metaInfo.varType[s]);
        } else {
            error(std::string("unknown token: ") + s);
        }
        bufferMap[s] = buffer.size() - 1;
        originalValue[s] = std::get<0>(buffer[buffer.size() - 1]);
        return buffer.size() - 1;
    }

    /**
     * @brief write all managed value back to cache and output
     * @attention it will not write back input value
     *
     */
    void writeBack() {
        for (auto &&[name, ind] : bufferMap) {
            if (auto it = data.output.find(name); it != data.output.end()) {
                // should not access output unless assign to it
                auto &now = assemble(ind);
                it->second = now;
            } else if (auto it = data.cache.find(name); it != data.cache.end()) {
                // may access cache without access to it, so use the same method in cpp-backend to
                // determine whether to write back
                auto &now = assemble(ind);
                if (!tools::myany::anyEqual(now, originalValue[name])) {
                    it->second = now;
                }
            }
        }
        buffer.clear();
        bufferMap.clear();
        originalValue.clear();
        relation.clear();
        managedString.clear();
    }

    /**
     * @brief create a new instance of given type
     *
     * @param s type name
     * @return size_t
     */
    size_t makeInstance(const std::string &s) {
        auto xmlType = data.innerType2XMLType(s);
        buffer.emplace_back(data.makeTypeEmptyInstance(xmlType), xmlType);
        return buffer.size() - 1;
    }

    /**
     * @brief create a new instance whose type is same as the given one
     *
     * @param token token which referring to the value
     * @return size_t
     */
    size_t makeInstanceAs(size_t token) {
        // TODO: make array, array push back
        return makeInstance(std::get<1>(buffer[token]));
    }

    /**
     * @brief define a new type
     *
     * @param s name of new type
     * @param t member of new type({name : type})
     */
    void defineType(const std::string &s, const std::unordered_map<std::string, std::string> &t) {
        if (data.metaInfo.typeDefines.contains(s))
            error(std::string("type \"") + s + "\" already defined");
        // due to data stored in CSValueMap, order of member does not make sense.
        data.metaInfo.typeDefines[s] = {t.begin(), t.end()};
    }

    /**
     * @brief make assignment between two managed value
     *
     * @param dst token which referring to the assign destination
     * @param src token which referring to the assign source
     */
    void assign(size_t dst, size_t src) {
        // TODO: allow assign base type to base type
        if (rulesetxml::baseNumericalData.contains(std::get<1>(buffer[dst])) && rulesetxml::baseNumericalData.contains(std::get<1>(buffer[src]))) {
            auto v = readValue(src);
            writeValue(dst, v);
            return;
        }
        my_assert(std::get<1>(buffer[dst]) == std::get<1>(buffer[src]),
                  "assignment of different types are not allowed");
        auto &tmp = assemble(src);
        std::get<0>(buffer[dst]) = tmp;
        relation[dst].clear();
    }

    /**
     * @brief get the array member in index, store it in buffer and return the token
     * reffering to it
     *
     * @param base token which referring to the array
     * @param index array index, must in range [0, base.length)
     * @return size_t token which referring to the returned value
     */
    size_t arrayAccess(size_t base, size_t index) {
        if (auto it = relation[base].find(std::to_string(index)); it != relation[base].end()) {
            return it->second;
        }
        if (!data.isArray(std::get<1>(buffer[base]))) {
            error(std::format("type \"{}\" is not an array", std::get<1>(buffer[base])));
        }
        auto array = std::any_cast<std::vector<std::any>>(std::get<0>(buffer[base]));
        if (index >= array.size()) {
            error(std::format("array out of range, index: {}, size: {}", index, array.size()));
        }
        auto tmp = array[index];
        auto baseType = std::get<1>(buffer[base]);
        buffer.emplace_back(tmp, data.arrayElementType(baseType));
        relation[base].emplace(std::to_string(index), buffer.size() - 1);
        return buffer.size() - 1;
    }

    /**
     * @brief get designated member of the given value, store it in buffer and return the token
     * reffering to it
     * @attention specially, if the base is an array, the only member it has is "length"
     * which is the length of the array
     *
     * @param base token which referring to the value
     * @param name member name
     * @return size_t token which referring to the returned value
     */
    size_t memberAccess(size_t base, const std::string &name) {
        if (auto it = relation[base].find(name); it != relation[base].end()) {
            return it->second;
        }
        if (data.isArray(std::get<1>(buffer[base]))) {
            error(std::format("type \"{}\" is an array", std::get<1>(buffer[base])));
        }
        auto tmp = std::any_cast<CSValueMap>(std::get<0>(buffer[base]))[name];
        auto baseType = std::get<1>(buffer[base]);
        auto it = std::find_if(data.metaInfo.typeDefines[baseType].begin(), data.metaInfo.typeDefines[baseType].end(),
                               [&](auto &x) { return std::get<0>(x) == name; });
        if (it == data.metaInfo.typeDefines[baseType].end()) {
            error(std::format("type \"{}\" has no member {}", std::get<1>(buffer[base]), name));
        }
        auto newType = std::get<1>(*it);
        buffer.emplace_back(tmp, newType);
        relation[base].emplace(name, buffer.size() - 1);
        return buffer.size() - 1;
    }

    /**
     * @brief get the length of the given array
     *
     * @param index token referring to the array
     * @return size_t
     */
    size_t arrayLength(size_t index) {
        auto &tmp = std::any_cast<std::vector<std::any> &>(std::get<0>(buffer[index]));
        return tmp.size();
    }

    /**
     * @brief resize given array to new size
     *
     * @param index token referring to the array
     * @param size new size of the array
     */
    void arrayResize(size_t index, size_t size) {
        auto &origin = assemble(index);
        auto &tmp = std::any_cast<std::vector<std::any> &>(origin);
        tmp.resize(size, data.makeTypeEmptyInstance(data.arrayElementType(std::get<1>(buffer[index]))));
    }

    /**
     * @brief extend the given array by one element
     *
     * @param index token referring to the array
     * @param newElementIndex token referring to the new element append to array
     */
    void arrayExtend(size_t index, size_t newElementIndex) {
        auto &origin = assemble(index);
        auto &tmp = std::any_cast<std::vector<std::any> &>(origin);
        auto &newElement = assemble(newElementIndex);
        tmp.emplace_back(newElement);
    }

    /**
     * @brief extend the given array by one element
     *
     * @param index token referring to the array
     * @param newElement new element append to array
     */
    void arrayExtend(size_t index, double newElement) {
        arrayResize(index, arrayLength(index) + 1);
        auto p = arrayAccess(index, arrayLength(index) - 1);
        writeValue(p, newElement);
    }

    /**
     * @brief check if the given value is a base type
     * (which means it is not an array or a struct)
     *
     * @param index token referring to the value
     * 
     * @return bool
     */
    bool isBaseType(size_t index) {
        return rulesetxml::baseData.contains(std::get<1>(buffer[index]));
    }

    /**
     * @brief get the double value of the variable that given token reffered
     * @attention the given token must reffering to a variable
     * with base type, that means isBaseType(index) must be true
     *
     * @param index token referring to the value
     * @return double
     */
    double readValue(size_t index) {
        auto &v = std::get<0>(buffer[index]);
        if (v.type() == typeid(bool)) {
            return std::any_cast<bool>(v);
        } else if (v.type() == typeid(int8_t)) {
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
            // TODO: check if lose precision
            auto tmp = std::any_cast<int64_t>(v);
            auto tmpv = static_cast<double>(tmp);
            // my_assert(tmp == static_cast<int64_t>(tmpv),
            //     "Ruleset engine use float64 as inner type to execute, data provided overflow");
            return tmpv;
        } else if (v.type() == typeid(uint64_t)) {
            // TODO: check if lose precision
            auto tmp = std::any_cast<uint64_t>(v);
            auto tmpv = static_cast<double>(tmp);
            // my_assert(tmp == static_cast<int64_t>(tmpv),
            //     "Ruleset engine use float64 as inner type to execute, data provided overflow");
            return tmpv;
        } else if (v.type() == typeid(float)) {
            return std::any_cast<float>(v);
        } else if (v.type() == typeid(double)) {
            return std::any_cast<double>(v);
        } else {
            error(std::string("unknown type: ") + v.type().name());
        }
    }

    /**
     * @brief assign to managed variable
     * @attention the given token must reffering to a variable
     * with base type, that means isBaseType(index) must be true
     *
     * @param index token referring to the value
     * @param tar assigned value
     */
    void writeValue(size_t index, double tar) {
        auto &v = std::get<0>(buffer[index]);
        if (v.type() == typeid(bool)) {
            v = (bool)(tar);
        } else if (v.type() == typeid(int8_t)) {
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
        } else {
            error(std::string("unknown type: ") + v.type().name());
        }
    }

  private:

    std::any &assemble(size_t index) {
        auto &[v, type] = buffer[index];
        if (rulesetxml::baseData.contains(type)) {
            return v;
        }
        if (data.isArray(type)) {
            auto tmp = std::any_cast<std::vector<std::any>>(std::move(v));
            for (auto &&[name, ind] : relation[index]) {
                tmp[std::stoi(name)] = assemble(ind);
            }
            relation[index].clear();
            std::get<0>(buffer[index]) = std::move(tmp);
            return std::get<0>(buffer[index]);
        } else {
            auto tmp = std::any_cast<CSValueMap>(std::move(v));
            for (auto &&[name, ind] : relation[index]) {
                tmp[name] = assemble(ind);
            }
            relation[index].clear();
            std::get<0>(buffer[index]) = std::move(tmp);
            return std::get<0>(buffer[index]);
        }
    }
    std::map<std::string, size_t> managedString; /**< string managed by this context */
    // {value, type}
    std::vector<std::tuple<std::any, std::string>> buffer; /**< buffer for storing values */
    // index -> {memberName, index}
    std::map<size_t, std::map<std::string, size_t>> relation; /**< relation between values */
    // name -> index
    std::map<std::string, size_t> bufferMap; /**< map from input/output/cache value name to index in buffer */
    // name -> value
    std::unordered_map<std::string, std::any> originalValue;
};

} // namespace rulejit::cq