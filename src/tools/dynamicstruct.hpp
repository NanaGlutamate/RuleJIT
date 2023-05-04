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

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "seterror.hpp"

#define ADD_BUILDIN_TYPE(type)                                                                                         \
    {                                                                                                                  \
        #type, { alignof(type), sizeof(type) }                                                                         \
    }

namespace dynamicstruct {

using TypeIndentifier = std::string;
using MemberIdentifier = std::string;

using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;
using float32 = float;
using float64 = double;
using string = std::string;

struct TypeLayoutInfo {
    size_t align;
    size_t size;
};

inline static std::map<TypeIndentifier, TypeLayoutInfo> buildInTypeLayoutInfo{
    ADD_BUILDIN_TYPE(int32),   ADD_BUILDIN_TYPE(uint32),  ADD_BUILDIN_TYPE(int64), ADD_BUILDIN_TYPE(uint64),
    ADD_BUILDIN_TYPE(float32), ADD_BUILDIN_TYPE(float64), ADD_BUILDIN_TYPE(bool),
};

using StructInfo = std::vector<std::tuple<MemberIdentifier, TypeIndentifier>>;
struct ComplexType {
    ComplexType(const StructInfo &defines, const StructManager &structManager) {
        size_t align = 0, offset = 0;
        for (auto &&[ident, type] : defines) {
            auto &layout = structManager.getTypeLayoutInfo(type);
            if (layout.align > align) {
                align = layout.align;
                if (align & (align - 1)) {
                    error("align is not power of 2");
                }
            }
            if (offset % layout.align != 0) {
                offset = layout.align * (offset / layout.align + 1);
            }
            memberOffset[ident] = offset;
            offset += layout.size;
        }
        if (offset % align != 0) {
            offset = align * (offset / align + 1);
        }
        size_t size = offset;
        layoutInfo = { align, size };
        // TODO: empty type?
    }
    std::ptrdiff_t getOffset(const MemberIdentifier &memberName) { return memberOffset.find(memberName)->second; }
    TypeLayoutInfo layoutInfo;
    std::map<MemberIdentifier, std::ptrdiff_t> memberOffset;
};

struct StructManager {
    StructManager() = default;
    const TypeLayoutInfo &getTypeLayoutInfo(const std::string &s) const {
        auto it = buildInTypeLayoutInfo.find(s);
        if (it != buildInTypeLayoutInfo.end()) {
            return it->second;
        }
        auto it2 = complexType.find(s);
        if (it2 == complexType.end()) {
            error("type not found");
        }
        return it2->second.layoutInfo;
    }
    std::map<TypeIndentifier, ComplexType> complexType;
};

} // namespace dynamicstruct