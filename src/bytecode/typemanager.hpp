/**
 * @file typemanager.hpp
 * @author djw
 * @brief 
 * @date 2023-09-20
 * 
 * @details manage literal type, map them to layout
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-09-20</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <string>

namespace typemanager{

struct TypeManager{
    using InnerTypeToken = size_t;
    struct TypeToken{
        TypeManager& managed;
        InnerTypeToken token;
    };
    struct TypeInfo{
        std::string typeName;
        enum struct Form{
            BUILD_IN,
            PTR,
            FUNCTION,
            INSTANTIATION,
            STRUCT
        }form;
    };


private:

    std::vector<TypeInfo> typeList;

    std::unordered_map<InnerTypeToken, InnerTypeToken> ptrBaseType;
    std::unordered_map<InnerTypeToken, InnerTypeToken> ptrOfType;
    
    std::map<InnerTypeToken, std::vector<InnerTypeToken>> functionBaseType;
    std::map<std::vector<InnerTypeToken>, InnerTypeToken> functionOfType;
    
    std::map<InnerTypeToken, std::tuple<std::string, std::vector<InnerTypeToken>>> instantiationBaseType;
    std::map<std::tuple<std::string, std::vector<InnerTypeToken>>, InnerTypeToken> instantiationOfType;
    
    std::map<InnerTypeToken, std::vector<std::tuple<std::string, InnerTypeToken>>> structBaseType;
    std::map<std::vector<std::tuple<std::string, InnerTypeToken>>, InnerTypeToken> structOfType;
};

}