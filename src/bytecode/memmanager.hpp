/**
 * @file memmanager.hpp
 * @author djw
 * @brief 
 * @date 2023-09-20
 * 
 * @details GC / RTTI
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-09-20</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <string_view>
#include <map>
#include <vector>
#include <list>

namespace memorymanager {

struct MemoryManager {
    uint8_t* alloc(size_t byteSize, std::string_view type){
        // constexpr size_t rate = sizeof(uint64_t) / sizeof(uint8_t);
        // auto unitSize = (byteSize + rate - 1) / rate;
    }
    uint8_t* getHeadPtr(uint8_t* ptr){}
    std::string_view getTypeByHeadPtr(uint8_t* ptr){}
    void gc(){}
    MemoryManager() = default;
    ~MemoryManager(){
        for(auto l : sizeMap){
            for(auto t : l){
                delete[] t.headPtr;
            }
        }
        for(auto t : oversizeThunk){
            delete[] t.headPtr;
        }
    }
private:
    struct Thunk{
        uint8_t* headPtr;
        uint8_t* freePtr;
        size_t totalSize;
        size_t blockSize;
    };
    struct DataBlock{
        DataBlock* next;
        struct{
            uint32_t typeLayoutID;
            uint16_t color;
            uint16_t reserved;
        }runtimeHeader;
        uint8_t data[0];
    };
    static_assert(sizeof(DataBlock) == sizeof(uint64_t) * 2);
    // std::list<> ptrMemberDiff;
    // std::vector<FastTypeInfo>
    std::map<uint8_t*, Thunk*> memoryMap;
    std::vector<std::list<Thunk>> sizeMap;
    std::list<Thunk> oversizeThunk;
};

}