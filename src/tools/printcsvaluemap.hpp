#pragma once

#include <unordered_map>
#include <string>
#include <any>
#include <iostream>

namespace rulejit{

void printCSValueMap(const std::unordered_map<std::string, std::any> &v) {
    using namespace std;
    cout << "{";
    bool start = false;
    for (auto &[k, v] : v) {
        if (!start) {
            start = true;
        } else {
            cout << ", ";
        }
        cout << k << ": ";
        if (v.type() == typeid(int8_t)) {
            cout << (int64_t)std::any_cast<int8_t>(v);
        } else if (v.type() == typeid(uint8_t)) {
            cout << (uint64_t)std::any_cast<uint8_t>(v);
        } else if (v.type() == typeid(int16_t)) {
            cout << (int64_t)std::any_cast<int16_t>(v);
        } else if (v.type() == typeid(uint16_t)) {
            cout << (uint64_t)std::any_cast<uint16_t>(v);
        } else if (v.type() == typeid(int32_t)) {
            cout << std::any_cast<int32_t>(v);
        } else if (v.type() == typeid(uint32_t)) {
            cout << std::any_cast<uint32_t>(v);
        } else if (v.type() == typeid(int64_t)) {
            cout << std::any_cast<int64_t>(v);
        } else if (v.type() == typeid(uint64_t)) {
            cout << std::any_cast<uint64_t>(v);
        } else if (v.type() == typeid(float)) {
            cout << std::any_cast<float>(v);
        } else if (v.type() == typeid(double)) {
            cout << std::any_cast<double>(v);
        } else if (v.type() == typeid(std::unordered_map<std::string, std::any>)) {
            printCSValueMap(std::any_cast<std::unordered_map<std::string, std::any>>(v));
        } else if (v.type() == typeid(std::vector<std::any>)) {
            auto tmp = std::any_cast<std::vector<std::any>>(v);
            cout << "{";
            bool start_ = false;
            for (auto &&item : tmp) {
                if (!start_) {
                    start_ = true;
                } else {
                    cout << ", ";
                }
                printCSValueMap(std::any_cast<std::unordered_map<std::string, std::any>>(item));
            }
            cout << "}";
        } else {
            cout << "unknown";
        }
    }
    cout << "}";
}

}