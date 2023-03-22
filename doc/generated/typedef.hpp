#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <vector>
#include <type_traits>

namespace ruleset{

using f64 = double;
using float64 = double;
using float32 = float;
using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

template <typename T> constexpr inline bool is_vector_v = false;

template <typename T> constexpr inline bool is_vector_v<std::vector<T>> = true;

template <typename T>
constexpr inline bool is_base_v =
    std::is_same_v<T, bool> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, int16_t> ||
    std::is_same_v<T, uint16_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, float> ||
    std::is_same_v<T, double>;// || std::is_same_v<T, std::string>;

template <typename T>
struct typedReal{
    double v;
    using type = T;
    operator double() const{ return v; }
    typedReal(double v) : v(v){}
    typedReal() : v(0.){}
    typedReal(const typedReal&) = default;
    typedReal& operator=(const typedReal&) = default;
};

template <typename T> constexpr inline bool is_typedReal_v = false;

template <typename T> constexpr inline bool is_typedReal_v<typedReal<T>> = true;

using CSValueMap = std::unordered_map<std::string, std::any>;

template <typename T>
void emplaceFromAny(T& t, const std::any& v){
    using Ty = std::remove_cv_t<typename std::remove_reference<T>::type>;
    if constexpr(is_base_v<Ty>){
        t = std::any_cast<Ty>(v);
    }else if constexpr(is_typedReal_v<Ty>){
        t = double(std::any_cast<typename Ty::type>(v));
    }else if constexpr(is_vector_v<Ty>){
        auto tmp = std::any_cast<std::vector<std::any>>(&(v));
        t.resize(tmp->size());
        for(size_t i = 0; i < tmp->size(); ++i){
            emplaceFromAny(t[i], (*tmp)[i]);
        }
    }else{
        t.FromValueMap(std::any_cast<CSValueMap>(v));
    }
}

template <typename T>
std::any toAny(const T& t){
    using Ty = std::remove_cv_t<typename std::remove_reference<T>::type>;
    if constexpr(is_base_v<Ty>){
        return t;
    }else if constexpr(is_typedReal_v<Ty>){
        return (typename Ty::type)(t);
    }else if constexpr(is_vector_v<Ty>){
        std::vector<std::any> tmp;
        tmp.reserve(t.size());
        for(auto&& i : t){
            tmp.emplace_back(toAny(i));
        }
        return tmp;
    }else{
        return t.ToValueMap();
    }
}

struct NoInstanceType{};


struct Location {
    float64 altitude;
    float64 longitude;
    float64 latitude;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("altitude"); it != v.end()){
            emplaceFromAny(altitude, it->second);
        }
        if(auto it = v.find("longitude"); it != v.end()){
            emplaceFromAny(longitude, it->second);
        }
        if(auto it = v.find("latitude"); it != v.end()){
            emplaceFromAny(latitude, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("altitude", toAny(altitude));
        tmp.emplace("longitude", toAny(longitude));
        tmp.emplace("latitude", toAny(latitude));
        return tmp;
    }
};

struct Input {
    typedReal<uint32> Input1;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("Input1"); it != v.end()){
            emplaceFromAny(Input1, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("Input1", toAny(Input1));
        return tmp;
    }
};

struct Vector3 {
    float64 x;
    float64 y;
    float64 z;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("x"); it != v.end()){
            emplaceFromAny(x, it->second);
        }
        if(auto it = v.find("y"); it != v.end()){
            emplaceFromAny(y, it->second);
        }
        if(auto it = v.find("z"); it != v.end()){
            emplaceFromAny(z, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("x", toAny(x));
        tmp.emplace("y", toAny(y));
        tmp.emplace("z", toAny(z));
        return tmp;
    }
};

struct Output {
    typedReal<uint32> Output1;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("Output1"); it != v.end()){
            emplaceFromAny(Output1, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("Output1", toAny(Output1));
        return tmp;
    }
};

struct Cache {
    typedReal<uint32> Cache1;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("Cache1"); it != v.end()){
            emplaceFromAny(Cache1, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("Cache1", toAny(Cache1));
        return tmp;
    }
};


}

