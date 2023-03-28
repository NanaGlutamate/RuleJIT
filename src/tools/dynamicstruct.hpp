/**
 * @file dynamicstruct.hpp
 * @author djw
 * @brief 
 * @date 2023-03-28
 * 
 * @details Includes DynamicStruct-related works. Not used for now.
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <string>
#include <vector>
#include <map>
#include <tuple>

#define ADD_BUILDIN_TYPE(type) {#type, {alignof(type), sizeof(type)}}

namespace dynamicstruct {

using TypeIndentifier = std::string;
using MemberIdentifier = std::string;

using int32 = int32_t;
using int64 = int64_t;
using float64 = double;
using string = std::string;

inline static std::map<TypeIndentifier, TypeLayoutInfo> buildInTypeLayoutInfo{
    ADD_BUILDIN_TYPE(int32),
    ADD_BUILDIN_TYPE(int64),
    ADD_BUILDIN_TYPE(float64),
    ADD_BUILDIN_TYPE(bool),
};

struct TypeLayoutInfo{
    size_t align;
    size_t size;
    // void(*constructor)();
};

struct ComplexType{
    template<typename Container>
    ComplexType(const Container& define){
        for(auto&& [ident, layoutInfo] : define){}
    }
    std::ptrdiff_t getOffset(const MemberIdentifier& memberName){
        return memberOffset.find(memberName)->second;
    }
private:
    std::map<MemberIdentifier, std::ptrdiff_t> memberOffset;
};

struct StructManager{
    StructManager() = default;
private:
    std::map<TypeIndentifier, TypeLayoutInfo> structInfo;
    std::map<TypeIndentifier, ComplexType> complexType;
};

}