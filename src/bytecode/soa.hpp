/**
 * @file soa.hpp
 * @author djw
 * @brief GC/Small Object Allocator
 * @date 2023-09-21
 * 
 * @details Includes a small object allocator, which is used by GC.
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-09-21</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <assert.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>
#include <map>

namespace memorymanager {

constexpr size_t operator""_KB(size_t size){
    return size * 1024;
}

constexpr size_t operator""_MB(size_t size){
    return size * 1024_KB;
}

constexpr size_t operator""_GB(size_t size){
    return size * 1024_MB;
}

struct MemPool;

/**
 * @brief Small Object Allocator.
 * 
 * @tparam Size: size in byte of each small object.
 * @tparam Len: number of objects in each block.
 * 
 */
template <size_t Size, size_t Len>
struct SmallObjectAllocator {
    /**
     * @brief object pool designed with free list.
     * 
     */
    struct alignas(Size * Len) FreeList {
        /**
         * @brief node of free list
         * 
         */
        struct alignas(Size) Node {
            union {
                char data[Size];
                Node *next;
            };
        } data[Len];

        /**
         * @brief Construct a new Free List object
         * 
         * @param[in] head head pointer of SOA, will changed to
         * the last node of new-added block.
         */
        FreeList(Node *&head) {
            Node *tmp = head;
            for (size_t i = 0; i < Len; i++) {
                data[i].next = tmp;
                tmp = data + i;
            }
            head = tmp;
        }
    };

    /**
     * @brief Construct a new Small Object Allocator object
     * @attention reserve means number of objects, not blockes.
     * 
     * still, may reserve more objects than declared to fill block.
     * 
     * @param reserve: number of objects to reserve
     * @param observer: pointer to mempool which need to informed when alloc mem
     * 
     */
    SmallObjectAllocator(size_t reserve, MemPool* observer, size_t cnt = -1): observer(observer), cnt(cnt) {
        head = nullptr;
        for (size_t i = 0; i < (reserve + Len - 1) / Len; i++) {
            freeList.emplace_back(head);
        }
    }

    /**
     * @brief allocate a small object, auto extend if no space retained.
     * 
     * @return void* 
     */
    void *alloc() {
        if (!head) {
            if(cnt == 0)return nullptr;
            extend();
        }
        if(cnt != 0)cnt--;
        auto tmp = head;
        head = head->next;
        return reinterpret_cast<void *>(tmp);
    }

    /**
     * @brief release a small object.
     * @attention no check for p, if p is not a valid pointer or is not allocated
     * by this SOA, undefined behavior will occur; double-free also result in UB
     * 
     * @param p pointer to the object.
     */
    void release(void *p) {
        cnt++;
        auto tmp = reinterpret_cast<FreeList::Node *>(p);
        tmp->next = head;
        head = tmp;
    }

  private:
    void extend();
    // alignas 2^n
    static_assert(!((Size * Len) & (Size * Len - 1)));
    static_assert(Size >= sizeof(void *));
    FreeList::Node *head = nullptr;
    MemPool* observer;
    size_t cnt;
    std::list<FreeList> freeList;
};

struct MemPool {
    inline static constexpr size_t pageSize = 4_KB;
    uint8_t* alloc(size_t size) {
        uint8_t* ret;
        if((ret = inner_alloc(size)) == nullptr){
            return new alignas(uint64_t) uint8_t[size];
        }
        return ret;
    }
    void release(uint8_t* tar) {
        auto it = memtype.upper_bound(tar);
        if(it == memtype.end() || tar - it->first >= pageSize) {
            delete[] tar;
            return;
        }
        auto size = it->second;
        if(size == 16) {
            soa1.release(tar);
        } else if(size == 32) {
            soa2.release(tar);
        } else if(size == 64) {
            soa3.release(tar);
        } else if(size == 128) {
            soa4.release(tar);
        } else {
            assert(0 && "unknown size");
        }
    }
    MemPool(): soa1(1, this, 1024 * 1024), soa2(1, this, 512 * 1024), soa3(1, this, 256 * 1024), soa4(1, this, 128 * 1024) {}
private:
    uint8_t* inner_alloc(size_t size) {
        if(size <= 16) {
            return reinterpret_cast<uint8_t*>(soa1.alloc());
        } else if(size <= 32) {
            return reinterpret_cast<uint8_t*>(soa2.alloc());
        } else if(size <= 64) {
            return reinterpret_cast<uint8_t*>(soa3.alloc());
        } else if(size <= 128) {
            return reinterpret_cast<uint8_t*>(soa4.alloc());
        } else {
            return new alignas(uint64_t) uint8_t[size];
        }
    }
    friend struct SmallObjectAllocator<16, pageSize / 16>;
    friend struct SmallObjectAllocator<32, pageSize / 32>;
    friend struct SmallObjectAllocator<64, pageSize / 64>;
    friend struct SmallObjectAllocator<128, pageSize / 128>;
    void notify(uint8_t* tar, size_t size) {
        memtype.emplace(tar, size);
    }
    std::map<uint8_t*, size_t> memtype;
    SmallObjectAllocator<16, pageSize / 16> soa1;
    SmallObjectAllocator<32, pageSize / 32> soa2;
    SmallObjectAllocator<64, pageSize / 64> soa3;
    SmallObjectAllocator<128, pageSize / 128> soa4;
};

template <size_t Size, size_t Len>
inline void SmallObjectAllocator<Size, Len>::extend() {
    freeList.emplace_back(head);
    observer->notify(&freeList.back(), Size);
}

}