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
 * <tr><td>djw</td><td>2023-09-20</td><td>reuse in bytecode VM.</td></tr>
 * </table>
 */
#pragma once

#include <algorithm>
#include <assert.h>
#include <format>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#define ADD_BUILDIN_TYPE(type)                                                                                         \
    {                                                                                                                  \
        #type, TypeLayoutInfo { alignof(type), sizeof(type) }                                                          \
    }

namespace dynamicstruct {

using TypeIndentifier = std::string;
using MemberIdentifier = std::string;

using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float_t;
using f64 = double_t;
using ptr = uint64_t;
struct func {
    ptr captured;
    ptr symbol;
};

struct TypeLayoutInfo {
    size_t align;
    size_t size;
};

using StructInfo = std::vector<std::tuple<MemberIdentifier, TypeIndentifier>>;

struct StructLayoutManager;

struct ComplexType {
    ComplexType(const StructInfo& defines, const StructLayoutManager& structManager);
    ComplexType(const ComplexType&) = default;
    ComplexType(ComplexType&&) = default;
    ComplexType& operator=(const ComplexType&) = default;
    ComplexType& operator=(ComplexType&&) = default;

    const TypeLayoutInfo& getLayout() const { return layoutInfo; }
    const StructInfo& getDefines() const { return defines; }
    std::ptrdiff_t getOffset(const MemberIdentifier& memberName) const { return memberOffset.find(memberName)->second; }
    std::optional<TypeIndentifier> getType(std::string_view memberName) const {
        auto it = std::find_if(defines.begin(), defines.end(),
                               [memberName](auto& a) { return std::get<0>(a) == memberName; });
        if (it == defines.end()) {
            return std::nullopt;
        }
        return std::get<1>(*it);
    }

  private:
    void optimize(StructInfo& info, const StructLayoutManager& structManager);

    TypeLayoutInfo layoutInfo;
    StructInfo defines;
    std::map<MemberIdentifier, std::ptrdiff_t> memberOffset;
};

struct StructLayoutManager {
    StructLayoutManager() : typeLayout(buildInTypeLayoutInfo), complexType() {}
    StructLayoutManager(const StructLayoutManager&) = delete;
    StructLayoutManager(StructLayoutManager&&) = delete;

    struct TypeToken {
        size_t token;
        // ComplexType& getType(StructLayoutManager& sm){
        // }
    };

    std::string print(uint8_t* tar, const std::string& type) {
        static const std::unordered_map<std::string_view, std::function<std::string(uint8_t*)>> table{
            {"u8", [](uint8_t* p) { return std::to_string(*reinterpret_cast<u8*>(p)); }},
            {"u16", [](uint8_t* p) { return std::to_string(*reinterpret_cast<u16*>(p)); }},
            {"u32", [](uint8_t* p) { return std::to_string(*reinterpret_cast<u32*>(p)); }},
            {"u64", [](uint8_t* p) { return std::to_string(*reinterpret_cast<u64*>(p)); }},
            {"i8", [](uint8_t* p) { return std::to_string(*reinterpret_cast<i8*>(p)); }},
            {"i16", [](uint8_t* p) { return std::to_string(*reinterpret_cast<i16*>(p)); }},
            {"i32", [](uint8_t* p) { return std::to_string(*reinterpret_cast<i32*>(p)); }},
            {"i64", [](uint8_t* p) { return std::to_string(*reinterpret_cast<i64*>(p)); }},
            {"f32", [](uint8_t* p) { return std::to_string(*reinterpret_cast<f32*>(p)); }},
            {"f64", [](uint8_t* p) { return std::to_string(*reinterpret_cast<f64*>(p)); }},
            {"ptr", [](uint8_t* p) { return std::format("{#08x}", *reinterpret_cast<uint64_t*>(p)); }},
            {"func", [](uint8_t* p) {
                 return std::format("{#08x}({#08x})", reinterpret_cast<uint64_t*>(p)[1],
                                    reinterpret_cast<uint64_t*>(p)[0]);
             }}};
        if (auto it = table.find(type); it != table.end()) {
            return it->second(tar);
        } else if (auto it = complexType.find(type); it != complexType.end()) {
            std::string members;
            for (auto& [name, type] : it->second.getDefines()) {
                members += std::format("{}: {};", name, print(tar + it->second.getOffset(name), type));
            }
            return std::format("{}{{{}}}", type, members);
        } else {
            return "[unknown]";
        }
    }
    TypeLayoutInfo getTypeLayoutInfo(const std::string& s) const {
        auto it = typeLayout.find(s);
        assert(it != typeLayout.end());
        return it->second;
    }
    ComplexType* addDefinition(const std::string& name, const StructInfo& defines) {
        if (complexType.find(name) != complexType.end())
            return nullptr; // TODO: check if is same definition
        complexType.emplace(name, ComplexType{defines, *this});
        return &complexType.find(name)->second;
    }
    const ComplexType& getInfo(const std::string& name) const {
        auto it = complexType.find(name);
        assert(it != complexType.end());
        return it->second;
    }

  private:
    std::map<TypeIndentifier, ComplexType> complexType;
    std::map<TypeIndentifier, TypeLayoutInfo> typeLayout;
    inline static const std::map<TypeIndentifier, TypeLayoutInfo> buildInTypeLayoutInfo{
        ADD_BUILDIN_TYPE(u8),  ADD_BUILDIN_TYPE(i8),  ADD_BUILDIN_TYPE(u16), ADD_BUILDIN_TYPE(i16),
        ADD_BUILDIN_TYPE(u32), ADD_BUILDIN_TYPE(i32), ADD_BUILDIN_TYPE(i64), ADD_BUILDIN_TYPE(u64),
        ADD_BUILDIN_TYPE(f32), ADD_BUILDIN_TYPE(f64), ADD_BUILDIN_TYPE(ptr), ADD_BUILDIN_TYPE(func)};
};

inline void ComplexType::optimize(StructInfo& info, const StructLayoutManager& structManager) {
    // if (info.size() <= 1)
    //     return;
    std::sort(info.begin(), info.end(), [&](auto& a, auto& b) {
        auto align_a = structManager.getTypeLayoutInfo(std::get<1>(a)).align;
        auto align_b = structManager.getTypeLayoutInfo(std::get<1>(b)).align;
        return align_a < align_b;
    });
}

inline ComplexType::ComplexType(const StructInfo& defines, const StructLayoutManager& structManager)
    : defines(defines), layoutInfo(), memberOffset() {
    size_t align = 0, offset = 0;
    optimize(this->defines, structManager);
    auto alignOffset = [](size_t align, size_t& offset) {
        if (offset % align != 0) {
            offset = align * (offset / align + 1);
        }
    };
    for (auto&& [ident, type] : defines) {
        auto layout = structManager.getTypeLayoutInfo(type);
        if (layout.align > align) {
            align = layout.align;
            assert(!(align & (align - 1)));
        }
        alignOffset(layout.align, offset);
        memberOffset[ident] = offset;
        offset += layout.size;
    }
    alignOffset(align, offset);
    layoutInfo = {align, offset};
}

} // namespace dynamicstruct