#pragma once

#include <any>
#include <string>
#include <unordered_map>

using CSValueMap = std::unordered_map<std::string, std::any>;

inline std::vector<CSValueMap> inputs{
    CSValueMap{
        {"d", double(55000)},       {"phiA", double(50)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(45000)},       {"phiA", double(80)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(35000)},       {"phiA", double(80)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(80)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(80)},      {"radar", double(1)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(80)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(20)},      {"radar", double(1)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(45000)},       {"phiA", double(20)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(35000)},       {"phiA", double(60)},       {"phiT", double(20)},      {"radar", double(1)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(35000)},       {"phiA", double(60)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(60)},       {"phiT", double(20)},      {"radar", double(1)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(60)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(80)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(80)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(60)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(60)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(40)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(40)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(40)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(80)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(80)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(60)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(60)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(40)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(40)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(1)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },

    CSValueMap{
        {"d", double(25000)},       {"phiA", double(20)},       {"phiT", double(20)},      {"radar", double(0)},
        {"missile", double(0)},     {"DTRmax", double(50000)},  {"DTMmax", double(40000)}, {"DTMmin", double(10000)},
        {"DTMKmax", double(30000)}, {"DTMKmin", double(20000)}, {"phiTRmax", double(70)},  {"phiTMmax", double(50)},
        {"phiTMK", double(30)},     {"DARmax", double(50000)},  {"DAMmax", double(40000)}, {"DAMmin", double(10000)},
        {"DAMKmax", double(30000)}, {"DAMKmin", double(20000)}, {"phiARmax", double(70)},  {"phiAMmax", double(50)},
        {"phiAMK", double(30)},
    },
};

inline std::vector<CSValueMap> inputs2{
    CSValueMap{
        {"Rel_",
         CSValueMap{
             {"d", double(11000)},
             {"phiA", double(140)},
             {"phiT", double(20)},
         }},
        {"A_output",
         CSValueMap{
             {"Longitude", double(0)},
             {"Latitude", double(10)},
             {"Altitude", double(500)},
             {"Speed", double(300)},
             {"Roll", double(0)},
             {"Pitch", double(500)},
             {"Yaw", double(350)},
         }},
        {"T_output",
         CSValueMap{
             {"Longitude", double(0)},
             {"Latitude", double(10)},
             {"Altitude", double(500)},
             {"Speed", double(350)},
             {"Roll", double(0)},
             {"Pitch", double(0)},
             {"Yaw", double(0)},
         }},
        {"DANmax", double(10000)},
        {"DANmin", double(2000)},
        {"DANok", double(5000)},
        {"DTNmax", double(10000)},
        {"DTNmin", double(2000)},
        {"DTNok", double(5000)},
        {"phiANmax", double(50)},
    },
    
    CSValueMap{
        {"Rel_",
         CSValueMap{
             {"d", double(3000)},
             {"phiA", double(20)},
             {"phiT", double(140)},
         }},
        {"A_output",
         CSValueMap{
             {"Longitude", double(0)},
             {"Latitude", double(10)},
             {"Altitude", double(500)},
             {"Speed", double(300)},
             {"Roll", double(0)},
             {"Pitch", double(0)},
             {"Yaw", double(0)},
         }},
        {"T_output",
         CSValueMap{
             {"Longitude", double(0)},
             {"Latitude", double(10)},
             {"Altitude", double(300)},
             {"Speed", double(350)},
             {"Roll", double(0)},
             {"Pitch", double(0)},
             {"Yaw", double(0)},
         }},
        {"DANmax", double(10000)},
        {"DANmin", double(2000)},
        {"DANok", double(5000)},
        {"DTNmax", double(10000)},
        {"DTNmin", double(2000)},
        {"DTNok", double(5000)},
        {"phiANmax", double(50)},
    },
};