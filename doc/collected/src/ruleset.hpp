#pragma once

#include <set>

#include "typedef.hpp"
#include "funcdef.hpp"

namespace ruleset{

struct RuleSet{
    _Input in;
    _Output out;
    _Cache cache;
    CSValueMap out_map;
    RuleSet() = default;
    void Init(){
        
    }
    CSValueMap* GetOutput(){
        return &out_map;
    }
    void SetInput(const CSValueMap& map){
        in.FromValueMap(map);
    }
    void Tick(){
        // auto &_in = in;
        // auto &_out = out;
        // auto _base = 0;
        // auto loadCache = [](auto x, auto y, auto z){};
        subRuleSet0.Tick(*this);

        subRuleSet0.writeBack(*this);

        subRuleSet1.Tick(*this);
        subRuleSet2.Tick(*this);
        subRuleSet3.Tick(*this);
        subRuleSet4.Tick(*this);
        subRuleSet5.Tick(*this);
        subRuleSet6.Tick(*this);
        subRuleSet7.Tick(*this);
        subRuleSet8.Tick(*this);
        subRuleSet9.Tick(*this);
        subRuleSet10.Tick(*this);
        subRuleSet11.Tick(*this);

        subRuleSet1.writeBack(*this);
        subRuleSet2.writeBack(*this);
        subRuleSet3.writeBack(*this);
        subRuleSet4.writeBack(*this);
        subRuleSet5.writeBack(*this);
        subRuleSet6.writeBack(*this);
        subRuleSet7.writeBack(*this);
        subRuleSet8.writeBack(*this);
        subRuleSet9.writeBack(*this);
        subRuleSet10.writeBack(*this);
        subRuleSet11.writeBack(*this);
        out_map = out.ToValueMap();
    }
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = ([&](){{(loadCache(_base, &_Cache::y21, 10), cache.y21) = ((_in.T2_output).Altitude) - ((_in.A1_output).Altitude);};{(loadCache(_base, &_Cache::HT, 21), cache.HT) = ((((_in.T1_output).Altitude) > (1000) && ((_in.T2_output).Altitude) > (1000)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((2)));};{(loadCache(_base, &_Cache::BF, 28), cache.BF) = (((_in.radar_flag) == (0) && (_in.missile_flag) == (0)) ? (typedReal<f64>)((0)) : (typedReal<f64>)((1)));};{(loadCache(_base, &_Cache::AW, 23), cache.AW) = (((_in.radar_flag) == (0)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((2)));};{(loadCache(_base, &_Cache::x22, 12), cache.x22) = ((_in.T2_output).Latitude) - ((_in.A2_output).Latitude) * (111000);};{(loadCache(_base, &_Cache::FD, 19), cache.FD) = (((_in.radar_flag) == (0) && (_in.missile_flag) == (0)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((((_in.radar_flag) == (1) && (_in.missile_flag) == (0)) ? (typedReal<f64>)((2)) : (typedReal<f64>)((3)))));};{(loadCache(_base, &_Cache::TM, 24), cache.TM) = (((_in.missile_flag) == (0)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((2)));};{(loadCache(_base, &_Cache::ax, 0), cache.ax) = ((_in.A2_output).Latitude) - ((_in.A1_output).Latitude) * (111000);};{(loadCache(_base, &_Cache::ay, 1), cache.ay) = ((_in.A2_output).Altitude) - ((_in.A1_output).Altitude);};{(loadCache(_base, &_Cache::az, 2), cache.az) = ((_in.A2_output).Longitude) - ((_in.A1_output).Longitude) * (111000) * (cos(((_in.A1_output).Latitude) / (180) * (3.1415926)));};{(loadCache(_base, &_Cache::x11, 3), cache.x11) = ((_in.T1_output).Latitude) - ((_in.A1_output).Latitude) * (111000);};{(loadCache(_base, &_Cache::x12, 6), cache.x12) = ((_in.T1_output).Latitude) - ((_in.A2_output).Latitude) * (111000);};{(loadCache(_base, &_Cache::x21, 9), cache.x21) = ((_in.T2_output).Latitude) - ((_in.A1_output).Latitude) * (111000);};{(loadCache(_base, &_Cache::y11, 4), cache.y11) = ((_in.T1_output).Altitude) - ((_in.A1_output).Altitude);};{(loadCache(_base, &_Cache::y12, 7), cache.y12) = ((_in.T1_output).Altitude) - ((_in.A2_output).Altitude);};{(loadCache(_base, &_Cache::y22, 13), cache.y22) = ((_in.T2_output).Altitude) - ((_in.A2_output).Altitude);};{(loadCache(_base, &_Cache::z11, 5), cache.z11) = ((_in.T1_output).Longitude) - ((_in.A1_output).Longitude) * (111000) * (cos(((_in.A1_output).Latitude) / (180) * (3.1415926)));};{(loadCache(_base, &_Cache::z12, 8), cache.z12) = ((_in.T1_output).Longitude) - ((_in.A2_output).Longitude) * (111000) * (cos(((_in.A2_output).Latitude) / (180) * (3.1415926)));};{(loadCache(_base, &_Cache::z21, 11), cache.z21) = ((_in.T2_output).Longitude) - ((_in.A1_output).Longitude) * (111000) * (cos(((_in.A1_output).Latitude) / (180) * (3.1415926)));};{(loadCache(_base, &_Cache::z22, 14), cache.z22) = ((_in.T2_output).Longitude) - ((_in.A2_output).Longitude) * (111000) * (cos(((_in.A2_output).Latitude) / (180) * (3.1415926)));};{(loadCache(_base, &_Cache::FA, 22), cache.FA) = (((_func_buildIn644964absfunc40f5452414562f5452((loadCache(_base, &_Cache::ay, 1), cache.ay))) < (4000) && (_func_buildIn644964absfunc40f5452414562f5452((loadCache(_base, &_Cache::ax, 0), cache.ax) * (cos(((_in.A2_output).Yaw) - ((_in.A1_output).Yaw) / (180) * (3.1415926))) + (loadCache(_base, &_Cache::az, 2), cache.az) * (sin(((_in.A2_output).Yaw) - ((_in.A1_output).Yaw) / (180) * (3.1415926))))) < (8000)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((2)));};{(loadCache(_base, &_Cache::d11, 15), cache.d11) = (pow((loadCache(_base, &_Cache::x11, 3), cache.x11) * (loadCache(_base, &_Cache::x11, 3), cache.x11) + (loadCache(_base, &_Cache::y11, 4), cache.y11) * (loadCache(_base, &_Cache::y11, 4), cache.y11) + (loadCache(_base, &_Cache::z11, 5), cache.z11) * (loadCache(_base, &_Cache::z11, 5), cache.z11), (0.5)));};{(loadCache(_base, &_Cache::d12, 16), cache.d12) = (pow((loadCache(_base, &_Cache::x12, 6), cache.x12) * (loadCache(_base, &_Cache::x12, 6), cache.x12) + (loadCache(_base, &_Cache::y12, 7), cache.y12) * (loadCache(_base, &_Cache::y12, 7), cache.y12) + (loadCache(_base, &_Cache::z12, 8), cache.z12) * (loadCache(_base, &_Cache::z12, 8), cache.z12), (0.5)));};{(loadCache(_base, &_Cache::d21, 17), cache.d21) = (pow((loadCache(_base, &_Cache::x21, 9), cache.x21) * (loadCache(_base, &_Cache::x21, 9), cache.x21) + (loadCache(_base, &_Cache::y21, 10), cache.y21) * (loadCache(_base, &_Cache::y21, 10), cache.y21) + (loadCache(_base, &_Cache::z21, 11), cache.z21) * (loadCache(_base, &_Cache::z21, 11), cache.z21), (0.5)));};{(loadCache(_base, &_Cache::d22, 18), cache.d22) = (pow((loadCache(_base, &_Cache::x22, 12), cache.x22) * (loadCache(_base, &_Cache::x22, 12), cache.x22) + (loadCache(_base, &_Cache::y22, 13), cache.y22) * (loadCache(_base, &_Cache::y22, 13), cache.y22) + (loadCache(_base, &_Cache::z22, 14), cache.z22) * (loadCache(_base, &_Cache::z22, 14), cache.z22), (0.5)));};{(loadCache(_base, &_Cache::BA, 26), cache.BA) = (((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DA).DRmax) || (_func_buildIn644964absfunc40f5452414562f5452((acos(((_in.T1_output).vx) * (loadCache(_base, &_Cache::x11, 3), cache.x11) + ((_in.T1_output).vy) * (loadCache(_base, &_Cache::y11, 4), cache.y11) + ((_in.T1_output).vz) * (loadCache(_base, &_Cache::z11, 5), cache.z11) / (loadCache(_base, &_Cache::d11, 15), cache.d11) * ((_in.T1_output).Speed))) / (3.1415926) * (180))) > ((_in.DA).phiRmax)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DA).DMmax) && (loadCache(_base, &_Cache::d11, 15), cache.d11) < ((_in.DA).DRmax) || (_func_buildIn644964absfunc40f5452414562f5452((acos(((_in.T1_output).vx) * (loadCache(_base, &_Cache::x11, 3), cache.x11) + ((_in.T1_output).vy) * (loadCache(_base, &_Cache::y11, 4), cache.y11) + ((_in.T1_output).vz) * (loadCache(_base, &_Cache::z11, 5), cache.z11) / (loadCache(_base, &_Cache::d11, 15), cache.d11) * ((_in.T1_output).Speed))) / (3.1415926) * (180))) > ((_in.DA).phiMmax)) ? (typedReal<f64>)((2)) : (typedReal<f64>)((((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DA).DMKmin) && (loadCache(_base, &_Cache::d11, 15), cache.d11) < ((_in.DA).DMKmax) || (_func_buildIn644964absfunc40f5452414562f5452((acos(((_in.T1_output).vx) * (loadCache(_base, &_Cache::x11, 3), cache.x11) + ((_in.T1_output).vy) * (loadCache(_base, &_Cache::y11, 4), cache.y11) + ((_in.T1_output).vz) * (loadCache(_base, &_Cache::z11, 5), cache.z11) / (loadCache(_base, &_Cache::d11, 15), cache.d11) * ((_in.T1_output).Speed))) / (3.1415926) * (180))) < ((_in.DA).phiMK)) ? (typedReal<f64>)((4)) : (typedReal<f64>)((3)))))));};{(loadCache(_base, &_Cache::BT, 27), cache.BT) = (((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DT).DRmax) || (_func_buildIn644964absfunc40f5452414562f5452((acos((-((_in.A1_output).vx)) * (loadCache(_base, &_Cache::x11, 3), cache.x11) - ((_in.A1_output).vy) * (loadCache(_base, &_Cache::y11, 4), cache.y11) - ((_in.A1_output).vz) * (loadCache(_base, &_Cache::z11, 5), cache.z11) / (loadCache(_base, &_Cache::d11, 15), cache.d11) * ((_in.A1_output).Speed))) / (3.1415926) * (180))) > ((_in.DT).phiRmax)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DT).DMmax) && (loadCache(_base, &_Cache::d11, 15), cache.d11) < ((_in.DT).DRmax) || (_func_buildIn644964absfunc40f5452414562f5452((-(acos(((_in.A1_output).vx) * (loadCache(_base, &_Cache::x11, 3), cache.x11) - ((_in.A1_output).vy) * (loadCache(_base, &_Cache::y11, 4), cache.y11) - ((_in.A1_output).vz) * (loadCache(_base, &_Cache::z11, 5), cache.z11) / (loadCache(_base, &_Cache::d11, 15), cache.d11) * ((_in.A1_output).Speed)))) / (3.1415926) * (180))) > ((_in.DT).phiMmax)) ? (typedReal<f64>)((2)) : (typedReal<f64>)((((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DT).DMKmin) && (loadCache(_base, &_Cache::d11, 15), cache.d11) < ((_in.DT).DMKmax) || (_func_buildIn644964absfunc40f5452414562f5452((acos((-((_in.A1_output).vx)) * (loadCache(_base, &_Cache::x11, 3), cache.x11) - ((_in.A1_output).vy) * (loadCache(_base, &_Cache::y11, 4), cache.y11) - ((_in.A1_output).vz) * (loadCache(_base, &_Cache::z11, 5), cache.z11) / (loadCache(_base, &_Cache::d11, 15), cache.d11) * ((_in.A1_output).Speed))) / (3.1415926) * (180))) < ((_in.DT).phiMK)) ? (typedReal<f64>)((4)) : (typedReal<f64>)((3)))))));};{(loadCache(_base, &_Cache::DM, 20), cache.DM) = (((loadCache(_base, &_Cache::d11, 15), cache.d11) > ((_in.DT).DMmax) && (loadCache(_base, &_Cache::d12, 16), cache.d12) > ((_in.DT).DMmax) && (loadCache(_base, &_Cache::d21, 17), cache.d21) > ((_in.DT).DMmax) && (loadCache(_base, &_Cache::d22, 18), cache.d22) > ((_in.DT).DMmax)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((((loadCache(_base, &_Cache::d11, 15), cache.d11) < ((_in.DT).DMmax) || (loadCache(_base, &_Cache::d12, 16), cache.d12) < ((_in.DT).DMmax) || (loadCache(_base, &_Cache::d21, 17), cache.d21) < ((_in.DT).DMmax) || (loadCache(_base, &_Cache::d22, 18), cache.d22) < ((_in.DT).DMmax)) ? (typedReal<f64>)((2)) : (typedReal<f64>)((0)))));};{(loadCache(_base, &_Cache::MS, 25), cache.MS) = (((loadCache(_base, &_Cache::FD, 19), cache.FD) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (1)) ? (typedReal<f64>)((1)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (0) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (0) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (1)) ? (typedReal<f64>)((2)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (0) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (2)) ? (typedReal<f64>)((3)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (0) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2)) ? (typedReal<f64>)((4)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (0) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1)) ? (typedReal<f64>)((5)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (1)) ? (typedReal<f64>)((6)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (1) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) && (loadCache(_base, &_Cache::HT, 21), cache.HT) == (1) && (loadCache(_base, &_Cache::FA, 22), cache.FA) == (2)) ? (typedReal<f64>)((7)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (1) && (loadCache(_base, &_Cache::TM, 24), cache.TM) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (0) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (1) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (1) && (loadCache(_base, &_Cache::TM, 24), cache.TM) == (1)) ? (typedReal<f64>)((8)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (1) && (loadCache(_base, &_Cache::TM, 24), cache.TM) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (1) && (loadCache(_base, &_Cache::TM, 24), cache.TM) == (2)) ? (typedReal<f64>)((9)) : (typedReal<f64>)((((loadCache(_base, &_Cache::FD, 19), cache.FD) == (2) && (_in.BTS) == (2) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2) || (loadCache(_base, &_Cache::FD, 19), cache.FD) == (3) && (loadCache(_base, &_Cache::DM, 20), cache.DM) == (2) && (loadCache(_base, &_Cache::AW, 23), cache.AW) == (2)) ? (typedReal<f64>)((10)) : (typedReal<f64>)((11)))))))))))))))))))));};return (0);}());
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    base.cache.AW = std::move(cache.AW);
                    base.cache.BA = std::move(cache.BA);
                    base.cache.BF = std::move(cache.BF);
                    base.cache.BT = std::move(cache.BT);
                    base.cache.DM = std::move(cache.DM);
                    base.cache.FA = std::move(cache.FA);
                    base.cache.FD = std::move(cache.FD);
                    base.cache.HT = std::move(cache.HT);
                    base.cache.MS = std::move(cache.MS);
                    base.cache.TM = std::move(cache.TM);
                    base.cache.ax = std::move(cache.ax);
                    base.cache.ay = std::move(cache.ay);
                    base.cache.az = std::move(cache.az);
                    base.cache.d11 = std::move(cache.d11);
                    base.cache.d12 = std::move(cache.d12);
                    base.cache.d21 = std::move(cache.d21);
                    base.cache.d22 = std::move(cache.d22);
                    base.cache.x11 = std::move(cache.x11);
                    base.cache.x12 = std::move(cache.x12);
                    base.cache.x21 = std::move(cache.x21);
                    base.cache.x22 = std::move(cache.x22);
                    base.cache.y11 = std::move(cache.y11);
                    base.cache.y12 = std::move(cache.y12);
                    base.cache.y21 = std::move(cache.y21);
                    base.cache.y22 = std::move(cache.y22);
                    base.cache.z11 = std::move(cache.z11);
                    base.cache.z12 = std::move(cache.z12);
                    base.cache.z21 = std::move(cache.z21);
                    base.cache.z22 = std::move(cache.z22);
                    break;

            }
            loaded.clear();
        }
    }subRuleSet0;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (3);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (4);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (5);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (6);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (7);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (8);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (11);return (9);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (6);(_out.wingman_tacticID) = (11);return (10);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (11);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (12);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (1) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (13);}())) : (typedReal<f64>)((-(1))))))))))))))))))))))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

                case 4:
                    break;

                case 5:
                    break;

                case 6:
                    break;

                case 7:
                    break;

                case 8:
                    break;

                case 9:
                    break;

                case 10:
                    break;

                case 11:
                    break;

                case 12:
                    break;

                case 13:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet1;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (2) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (2);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (2) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (2);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (2) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (2);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (2) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (2);return (3);}())) : (typedReal<f64>)((-(1))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet2;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (3) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (3);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (3) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (3);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (3) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (3);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (3) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (3);return (3);}())) : (typedReal<f64>)((-(1))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet3;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (4) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (7);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (4) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (2);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (4) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (1);return (2);}())) : (typedReal<f64>)((-(1))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet4;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (5) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (11);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (5) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (11);(_out.wingman_tacticID) = (11);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (5) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (11);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (5) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (11);return (3);}())) : (typedReal<f64>)((-(1))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet5;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (5);(_out.wingman_tacticID) = (5);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (5);(_out.wingman_tacticID) = (5);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (4);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (5);(_out.wingman_tacticID) = (5);return (3);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (4);return (4);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (4);return (5);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (6);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (6) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (7);}())) : (typedReal<f64>)((-(1))))))))))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

                case 4:
                    break;

                case 5:
                    break;

                case 6:
                    break;

                case 7:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet6;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (5);(_out.wingman_tacticID) = (5);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (5);(_out.wingman_tacticID) = (5);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (8);(_out.wingman_tacticID) = (8);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (5);(_out.wingman_tacticID) = (5);return (3);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (8);(_out.wingman_tacticID) = (8);return (4);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (8);(_out.wingman_tacticID) = (8);return (5);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (6);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (7) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (7);}())) : (typedReal<f64>)((-(1))))))))))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

                case 4:
                    break;

                case 5:
                    break;

                case 6:
                    break;

                case 7:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet7;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (11);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (11);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (6);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (6);(_out.wingman_tacticID) = (7);return (3);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (3);return (4);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (7);return (5);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (6);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (7);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (8);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (7);return (9);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (3);return (10);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (8) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (3);(_out.wingman_tacticID) = (7);return (11);}())) : (typedReal<f64>)((-(1))))))))))))))))))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

                case 4:
                    break;

                case 5:
                    break;

                case 6:
                    break;

                case 7:
                    break;

                case 8:
                    break;

                case 9:
                    break;

                case 10:
                    break;

                case 11:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet8;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (11);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (6);(_out.wingman_tacticID) = (11);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (11);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (11);return (3);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (4);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (5);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (11);return (6);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (11);return (7);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (11);return (8);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (9) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (11);return (9);}())) : (typedReal<f64>)((-(1))))))))))))))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

                case 4:
                    break;

                case 5:
                    break;

                case 6:
                    break;

                case 7:
                    break;

                case 8:
                    break;

                case 9:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet9;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (10) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (10) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (10) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (2);}())) : (typedReal<f64>)((-(1))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet10;
    struct {
        _Cache cache;
        std::set<size_t> loaded;
        int actived;
        template <typename T>
        void loadCache(RuleSet& base, T p, size_t name){
            if(auto it = loaded.find(name); it == loaded.end()){
                cache.*p = base.cache.*p;
                loaded.emplace(name);
            }
        }
        void Tick(RuleSet& _base){
            const auto& _in = _base.in;
            auto& _out = _base.out;
            actived = (((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (9);(_out.wingman_tacticID) = (9);return (0);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (1);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (2);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (1) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (3);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (4);return (4);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (9);(_out.wingman_tacticID) = (9);return (5);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (4);return (6);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (7);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (8);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (9);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (10);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (2) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (11);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (12);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (13);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (14);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (6);(_out.wingman_tacticID) = (6);return (15);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (2);return (16);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (17);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (18);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (3) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (1);(_out.wingman_tacticID) = (1);return (19);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (20);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (1) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (21);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (22);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (2) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (10);(_out.wingman_tacticID) = (10);return (23);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (24);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (3) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (4);(_out.wingman_tacticID) = (4);return (25);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (1)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (7);(_out.wingman_tacticID) = (7);return (26);}())) : (typedReal<f64>)((((loadCache(_base, &_Cache::MS, 25), cache.MS) == (11) && (loadCache(_base, &_Cache::BT, 27), cache.BT) == (4) && (loadCache(_base, &_Cache::BA, 26), cache.BA) == (4) && (loadCache(_base, &_Cache::BF, 28), cache.BF) == (0)) ? (typedReal<f64>)(([&](){(_out.leader_tacticID) = (2);(_out.wingman_tacticID) = (2);return (27);}())) : (typedReal<f64>)((-(1))))))))))))))))))))))))))))))))))))))))))))))))))))))))));
        }
        void writeBack(RuleSet& base){
            switch(actived){
                case -1:
                    break;
                case 0:
                    break;

                case 1:
                    break;

                case 2:
                    break;

                case 3:
                    break;

                case 4:
                    break;

                case 5:
                    break;

                case 6:
                    break;

                case 7:
                    break;

                case 8:
                    break;

                case 9:
                    break;

                case 10:
                    break;

                case 11:
                    break;

                case 12:
                    break;

                case 13:
                    break;

                case 14:
                    break;

                case 15:
                    break;

                case 16:
                    break;

                case 17:
                    break;

                case 18:
                    break;

                case 19:
                    break;

                case 20:
                    break;

                case 21:
                    break;

                case 22:
                    break;

                case 23:
                    break;

                case 24:
                    break;

                case 25:
                    break;

                case 26:
                    break;

                case 27:
                    break;

            }
            loaded.clear();
        }
    }subRuleSet11;

};

}
