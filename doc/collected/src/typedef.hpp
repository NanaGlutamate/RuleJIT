#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <vector>
#include <type_traits>

namespace ruleset{
using std::string;

using f32 = float;
using f64 = double;
using i32 = int32_t;
using i64 = int64_t;
using u32 = uint32_t;
using u64 = uint64_t;

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
    std::is_same_v<T, double> || std::is_same_v<T, std::string>;

template <typename T>
struct typedReal{
    double v;
    using type = T;
    operator double() const{ return v; }
    typedReal(double v) : v(v){}
    typedReal() : v(0.){}
    typedReal(const typedReal&) = default;
    template<typename Other>
    void operator=(const typedReal<Other>& o){
        // type trans?
        v = o.v;
    }
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

struct NoInstanceType{
    NoInstanceType() = default;
    NoInstanceType(const NoInstanceType&) = delete;
    NoInstanceType& operator=(const NoInstanceType&) = delete;
    NoInstanceType(NoInstanceType&&) = delete;
    NoInstanceType& operator=(NoInstanceType&&) = delete;
};


struct _Cache {
    typedReal<float64> ax;
    typedReal<float64> ay;
    typedReal<float64> az;
    typedReal<float64> x11;
    typedReal<float64> y11;
    typedReal<float64> z11;
    typedReal<float64> x12;
    typedReal<float64> y12;
    typedReal<float64> z12;
    typedReal<float64> x21;
    typedReal<float64> y21;
    typedReal<float64> z21;
    typedReal<float64> x22;
    typedReal<float64> y22;
    typedReal<float64> z22;
    typedReal<float64> d11;
    typedReal<float64> d12;
    typedReal<float64> d21;
    typedReal<float64> d22;
    typedReal<uint64> FD;
    typedReal<uint64> DM;
    typedReal<uint64> HT;
    typedReal<uint64> FA;
    typedReal<uint64> AW;
    typedReal<uint64> TM;
    typedReal<uint64> MS;
    typedReal<uint64> BA;
    typedReal<uint64> BT;
    typedReal<uint64> BF;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("ax"); it != v.end()){
            emplaceFromAny(ax, it->second);
        }
        if(auto it = v.find("ay"); it != v.end()){
            emplaceFromAny(ay, it->second);
        }
        if(auto it = v.find("az"); it != v.end()){
            emplaceFromAny(az, it->second);
        }
        if(auto it = v.find("x11"); it != v.end()){
            emplaceFromAny(x11, it->second);
        }
        if(auto it = v.find("y11"); it != v.end()){
            emplaceFromAny(y11, it->second);
        }
        if(auto it = v.find("z11"); it != v.end()){
            emplaceFromAny(z11, it->second);
        }
        if(auto it = v.find("x12"); it != v.end()){
            emplaceFromAny(x12, it->second);
        }
        if(auto it = v.find("y12"); it != v.end()){
            emplaceFromAny(y12, it->second);
        }
        if(auto it = v.find("z12"); it != v.end()){
            emplaceFromAny(z12, it->second);
        }
        if(auto it = v.find("x21"); it != v.end()){
            emplaceFromAny(x21, it->second);
        }
        if(auto it = v.find("y21"); it != v.end()){
            emplaceFromAny(y21, it->second);
        }
        if(auto it = v.find("z21"); it != v.end()){
            emplaceFromAny(z21, it->second);
        }
        if(auto it = v.find("x22"); it != v.end()){
            emplaceFromAny(x22, it->second);
        }
        if(auto it = v.find("y22"); it != v.end()){
            emplaceFromAny(y22, it->second);
        }
        if(auto it = v.find("z22"); it != v.end()){
            emplaceFromAny(z22, it->second);
        }
        if(auto it = v.find("d11"); it != v.end()){
            emplaceFromAny(d11, it->second);
        }
        if(auto it = v.find("d12"); it != v.end()){
            emplaceFromAny(d12, it->second);
        }
        if(auto it = v.find("d21"); it != v.end()){
            emplaceFromAny(d21, it->second);
        }
        if(auto it = v.find("d22"); it != v.end()){
            emplaceFromAny(d22, it->second);
        }
        if(auto it = v.find("FD"); it != v.end()){
            emplaceFromAny(FD, it->second);
        }
        if(auto it = v.find("DM"); it != v.end()){
            emplaceFromAny(DM, it->second);
        }
        if(auto it = v.find("HT"); it != v.end()){
            emplaceFromAny(HT, it->second);
        }
        if(auto it = v.find("FA"); it != v.end()){
            emplaceFromAny(FA, it->second);
        }
        if(auto it = v.find("AW"); it != v.end()){
            emplaceFromAny(AW, it->second);
        }
        if(auto it = v.find("TM"); it != v.end()){
            emplaceFromAny(TM, it->second);
        }
        if(auto it = v.find("MS"); it != v.end()){
            emplaceFromAny(MS, it->second);
        }
        if(auto it = v.find("BA"); it != v.end()){
            emplaceFromAny(BA, it->second);
        }
        if(auto it = v.find("BT"); it != v.end()){
            emplaceFromAny(BT, it->second);
        }
        if(auto it = v.find("BF"); it != v.end()){
            emplaceFromAny(BF, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("ax", toAny(ax));
        tmp.emplace("ay", toAny(ay));
        tmp.emplace("az", toAny(az));
        tmp.emplace("x11", toAny(x11));
        tmp.emplace("y11", toAny(y11));
        tmp.emplace("z11", toAny(z11));
        tmp.emplace("x12", toAny(x12));
        tmp.emplace("y12", toAny(y12));
        tmp.emplace("z12", toAny(z12));
        tmp.emplace("x21", toAny(x21));
        tmp.emplace("y21", toAny(y21));
        tmp.emplace("z21", toAny(z21));
        tmp.emplace("x22", toAny(x22));
        tmp.emplace("y22", toAny(y22));
        tmp.emplace("z22", toAny(z22));
        tmp.emplace("d11", toAny(d11));
        tmp.emplace("d12", toAny(d12));
        tmp.emplace("d21", toAny(d21));
        tmp.emplace("d22", toAny(d22));
        tmp.emplace("FD", toAny(FD));
        tmp.emplace("DM", toAny(DM));
        tmp.emplace("HT", toAny(HT));
        tmp.emplace("FA", toAny(FA));
        tmp.emplace("AW", toAny(AW));
        tmp.emplace("TM", toAny(TM));
        tmp.emplace("MS", toAny(MS));
        tmp.emplace("BA", toAny(BA));
        tmp.emplace("BT", toAny(BT));
        tmp.emplace("BF", toAny(BF));
        return tmp;
    }
};

struct _Output {
    typedReal<uint64> leader_tacticID;
    typedReal<uint64> wingman_tacticID;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("leader_tacticID"); it != v.end()){
            emplaceFromAny(leader_tacticID, it->second);
        }
        if(auto it = v.find("wingman_tacticID"); it != v.end()){
            emplaceFromAny(wingman_tacticID, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("leader_tacticID", toAny(leader_tacticID));
        tmp.emplace("wingman_tacticID", toAny(wingman_tacticID));
        return tmp;
    }
};

struct MissileRange {
    typedReal<float64> DRmax;
    typedReal<float64> phiRmax;
    typedReal<float64> DMmax;
    typedReal<float64> DMmin;
    typedReal<float64> phiMmax;
    typedReal<float64> DMKmax;
    typedReal<float64> DMKmin;
    typedReal<float64> phiMK;
    typedReal<float64> DNmax;
    typedReal<float64> DNmin;
    typedReal<float64> DNok;
    typedReal<float64> phiNmax;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("DRmax"); it != v.end()){
            emplaceFromAny(DRmax, it->second);
        }
        if(auto it = v.find("phiRmax"); it != v.end()){
            emplaceFromAny(phiRmax, it->second);
        }
        if(auto it = v.find("DMmax"); it != v.end()){
            emplaceFromAny(DMmax, it->second);
        }
        if(auto it = v.find("DMmin"); it != v.end()){
            emplaceFromAny(DMmin, it->second);
        }
        if(auto it = v.find("phiMmax"); it != v.end()){
            emplaceFromAny(phiMmax, it->second);
        }
        if(auto it = v.find("DMKmax"); it != v.end()){
            emplaceFromAny(DMKmax, it->second);
        }
        if(auto it = v.find("DMKmin"); it != v.end()){
            emplaceFromAny(DMKmin, it->second);
        }
        if(auto it = v.find("phiMK"); it != v.end()){
            emplaceFromAny(phiMK, it->second);
        }
        if(auto it = v.find("DNmax"); it != v.end()){
            emplaceFromAny(DNmax, it->second);
        }
        if(auto it = v.find("DNmin"); it != v.end()){
            emplaceFromAny(DNmin, it->second);
        }
        if(auto it = v.find("DNok"); it != v.end()){
            emplaceFromAny(DNok, it->second);
        }
        if(auto it = v.find("phiNmax"); it != v.end()){
            emplaceFromAny(phiNmax, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("DRmax", toAny(DRmax));
        tmp.emplace("phiRmax", toAny(phiRmax));
        tmp.emplace("DMmax", toAny(DMmax));
        tmp.emplace("DMmin", toAny(DMmin));
        tmp.emplace("phiMmax", toAny(phiMmax));
        tmp.emplace("DMKmax", toAny(DMKmax));
        tmp.emplace("DMKmin", toAny(DMKmin));
        tmp.emplace("phiMK", toAny(phiMK));
        tmp.emplace("DNmax", toAny(DNmax));
        tmp.emplace("DNmin", toAny(DNmin));
        tmp.emplace("DNok", toAny(DNok));
        tmp.emplace("phiNmax", toAny(phiNmax));
        return tmp;
    }
};

struct output_info_ {
    typedReal<float64> Longitude;
    typedReal<float64> Latitude;
    typedReal<float64> Altitude;
    typedReal<float64> vx;
    typedReal<float64> vy;
    typedReal<float64> vz;
    typedReal<float64> Speed;
    typedReal<float64> Roll;
    typedReal<float64> Pitch;
    typedReal<float64> Yaw;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("Longitude"); it != v.end()){
            emplaceFromAny(Longitude, it->second);
        }
        if(auto it = v.find("Latitude"); it != v.end()){
            emplaceFromAny(Latitude, it->second);
        }
        if(auto it = v.find("Altitude"); it != v.end()){
            emplaceFromAny(Altitude, it->second);
        }
        if(auto it = v.find("vx"); it != v.end()){
            emplaceFromAny(vx, it->second);
        }
        if(auto it = v.find("vy"); it != v.end()){
            emplaceFromAny(vy, it->second);
        }
        if(auto it = v.find("vz"); it != v.end()){
            emplaceFromAny(vz, it->second);
        }
        if(auto it = v.find("Speed"); it != v.end()){
            emplaceFromAny(Speed, it->second);
        }
        if(auto it = v.find("Roll"); it != v.end()){
            emplaceFromAny(Roll, it->second);
        }
        if(auto it = v.find("Pitch"); it != v.end()){
            emplaceFromAny(Pitch, it->second);
        }
        if(auto it = v.find("Yaw"); it != v.end()){
            emplaceFromAny(Yaw, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("Longitude", toAny(Longitude));
        tmp.emplace("Latitude", toAny(Latitude));
        tmp.emplace("Altitude", toAny(Altitude));
        tmp.emplace("vx", toAny(vx));
        tmp.emplace("vy", toAny(vy));
        tmp.emplace("vz", toAny(vz));
        tmp.emplace("Speed", toAny(Speed));
        tmp.emplace("Roll", toAny(Roll));
        tmp.emplace("Pitch", toAny(Pitch));
        tmp.emplace("Yaw", toAny(Yaw));
        return tmp;
    }
};

struct _Input {
    output_info_ A1_output;
    output_info_ A2_output;
    output_info_ T1_output;
    output_info_ T2_output;
    typedReal<bool> radar_flag;
    typedReal<bool> missile_flag;
    typedReal<bool> radar_state;
    typedReal<bool> missile_state;
    MissileRange DT;
    MissileRange DA;
    typedReal<uint64> BTS;
    void FromValueMap(const CSValueMap& v){
        if(auto it = v.find("A1_output"); it != v.end()){
            emplaceFromAny(A1_output, it->second);
        }
        if(auto it = v.find("A2_output"); it != v.end()){
            emplaceFromAny(A2_output, it->second);
        }
        if(auto it = v.find("T1_output"); it != v.end()){
            emplaceFromAny(T1_output, it->second);
        }
        if(auto it = v.find("T2_output"); it != v.end()){
            emplaceFromAny(T2_output, it->second);
        }
        if(auto it = v.find("radar_flag"); it != v.end()){
            emplaceFromAny(radar_flag, it->second);
        }
        if(auto it = v.find("missile_flag"); it != v.end()){
            emplaceFromAny(missile_flag, it->second);
        }
        if(auto it = v.find("radar_state"); it != v.end()){
            emplaceFromAny(radar_state, it->second);
        }
        if(auto it = v.find("missile_state"); it != v.end()){
            emplaceFromAny(missile_state, it->second);
        }
        if(auto it = v.find("DT"); it != v.end()){
            emplaceFromAny(DT, it->second);
        }
        if(auto it = v.find("DA"); it != v.end()){
            emplaceFromAny(DA, it->second);
        }
        if(auto it = v.find("BTS"); it != v.end()){
            emplaceFromAny(BTS, it->second);
        }
    }
    CSValueMap ToValueMap() const {
        CSValueMap tmp;
        tmp.emplace("A1_output", toAny(A1_output));
        tmp.emplace("A2_output", toAny(A2_output));
        tmp.emplace("T1_output", toAny(T1_output));
        tmp.emplace("T2_output", toAny(T2_output));
        tmp.emplace("radar_flag", toAny(radar_flag));
        tmp.emplace("missile_flag", toAny(missile_flag));
        tmp.emplace("radar_state", toAny(radar_state));
        tmp.emplace("missile_state", toAny(missile_state));
        tmp.emplace("DT", toAny(DT));
        tmp.emplace("DA", toAny(DA));
        tmp.emplace("BTS", toAny(BTS));
        return tmp;
    }
};


}

