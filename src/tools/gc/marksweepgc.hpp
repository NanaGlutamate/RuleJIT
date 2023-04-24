/**
 * @file marksweepgc.hpp
 * @author djw
 * @brief GC/Mark Sweep GC
 * @date 2023-03-28
 * 
 * @details Includes a mark sweep garbage collector.
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * <tr><td>djw</td><td>2023-04-19</td><td>Collect conditional compile marcos.</td></tr>
 * </table>
 */
#pragma once

#include "defines/marco.hpp"

#ifdef __RULEJIT_GC_DEBUG
#include "tools/myassert.hpp"
#include <format>
#include <iostream>
#include <string>
#endif

#include <list>
#include <memory>
#include <stack>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "tools/gc/soa.hpp"

namespace rulejit::gc {

#ifdef __RULEJIT_GC_DEBUG
    inline void debugLog(const std::string &type, const std::string &msg) {
        static std::string last;
        static size_t cnt = 0;
        static struct Last {
            ~Last() {
                if (cnt != 0) {
                    std::cout << std::format("    sent {} more times", cnt) << std::endl;
                }
            }
        } lastJob;
        if (last.empty() || last != type) {
            if (cnt != 0) {
                std::cout << std::format("    sent {} more times", cnt) << std::endl;
                cnt = 0;
            }
            last = type;
            std::cout << std::format("[{}]: {}", type, msg) << std::endl;
        } else {
            cnt++;
        }
    }
#else // __RULEJIT_GC_DEBUG
    inline void debugLog([[maybe_unused]] const std::string &type, [[maybe_unused]] const std::string &msg) {}
#endif // __RULEJIT_GC_DEBUG

// TODO: destructor support
// std::vector<std::function<void(uint64_t*)>> funcTable;
// std::unordered_map<uint64_t*, size_t> destructorBinding;

/**
 * @brief static class which contains a set of functions to manage memory.
 * 
 */
struct StaticMarkSweepGarbageCollector {
    using DataType = uint64_t;
    struct Data {
        Data *next;
        uint8_t mark, soacnt, reserve1, reserve2;
        uint32_t referenceCount;
        uint64_t data[0];
    };
    static const uint32_t HeadSize = sizeof(Data) / sizeof(uint64_t);
    static_assert(sizeof(Data) == sizeof(uint64_t) * 2);
    static_assert(sizeof(Data *) == 8);
    // root is normally a stack, with every reference is to a stack frame
    // TODO: remove root and use 'std::vector<uint64_t*> reachable' instead.
    static void extendRoot() {
        uint32_t originalSize = (root->referenceCount + HeadSize);
        uint32_t newSize = originalSize * 3 / 2;
        auto [newRoot, sign] = innerAlloc(newSize);
        memcpy(newRoot, root, originalSize * sizeof(uint64_t));
        memset(reinterpret_cast<uint64_t *>(newRoot) + originalSize, 0, (newSize - originalSize) * sizeof(uint64_t));
        newRoot->next = root->next;
        root = newRoot;
        root->soacnt = sign;
        root->referenceCount = newSize - HeadSize;
    }
    static uint64_t *alloc(uint32_t referenceCount, size_t valueCount = 0) {
        tryFree();
        auto [dst, sign] = innerAlloc(referenceCount + valueCount + HeadSize);
        memset(dst, 0, (referenceCount + valueCount + HeadSize) * sizeof(uint64_t));
        dst->next = head;
        head = dst;
        dst->soacnt = sign;
        dst->referenceCount = referenceCount;
        static struct Init2 {
            Init2(Data *dst, size_t counter, size_t ref) {
                root = dst;
                my_assert(counter == 0, "root must have no value");
                debugLog("INIT", std::format("init root node with {} member reference", ref));
            }
        } init2(dst, valueCount, referenceCount);
        return dst->data;
    }
    // static void releaseLastAllocated() {
    //     if (!head)
    //         return;
    //     auto tmp = head->next;
    //     innerRelease(head);
    //     head = tmp;
    // }
    // static void tryReleaseInSet(std::unordered_set<uint64_t *> &tar) {
    //     while (head && tar.contains(reinterpret_cast<uint64_t *>(head) + HeadSize)) {
    //         auto tmp = head->next;
    //         innerRelease(head);
    //         head = tmp;
    //     }
    // }
    static void fullGC() {
        auto tmp [[maybe_unused]] = allocatedCount;
        // TODO: mark in list
        if (!root)
            return;
        mark(root);
        while (head && !head->mark) {
            Data *tmp = head->next;
            innerRelease(head);
            head = tmp;
        }
        auto p = head;
        while (p) {
            p->mark = 0;
            while (p->next && !p->next->mark) {
                Data *tmp = p->next->next;
                innerRelease(p->next);
                p->next = tmp;
            }
            p = p->next;
        }
        debugLog("FULL GC", std::format("obj number: {}->{}", tmp, allocatedCount));
    }

  private:
    static void tryFree() {
        if (allocatedCount > gcThreshold) {
            auto tmp [[maybe_unused]] = gcThreshold;
            auto pre = allocatedCount;
            fullGC();
            if (gcThreshold > 512) {
                if (pre > allocatedCount * 8) {
                    gcThreshold = gcThreshold * 0.75;
                } else if (pre > allocatedCount * 4) {
                    gcThreshold = gcThreshold * 0.9;
                } else if (pre < allocatedCount * 2) {
                    gcThreshold = gcThreshold * 3Ui64 / 2;
                }
            }
            debugLog("AUTO GC", std::format("change GC Threshold from {} to {}", tmp, gcThreshold));
        }
    }
    static void mark(Data *t) {
        if (t->mark)
            return;
        t->mark = 1;
        for (uint32_t i = 0; i < t->referenceCount; ++i) {
            if (!t->data[i])
                continue;
            uint64_t *referedData = reinterpret_cast<uint64_t *>(t->data[i]) - HeadSize;
            mark(reinterpret_cast<Data *>(referedData));
        }
    }
    static void release(uint64_t *data) {
        Data *p = reinterpret_cast<Data *>(data - sizeof(Data));
        innerRelease(p);
    }
    inline static Data *root = nullptr;
    inline static Data *head = nullptr;
    // num of uint64_t
    static std::tuple<Data *, uint8_t> innerAlloc(size_t size) {
        allocatedCount++;
        if (size * sizeof(uint64_t) < 16) {
            auto p = reinterpret_cast<Data *>(soa1.alloc());
            debugLog("SOA1 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
            return std::make_tuple(p, 1);
        } else if (size * sizeof(uint64_t) < 32) {
            auto p = reinterpret_cast<Data *>(soa2.alloc());
            debugLog("SOA2 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
            return std::make_tuple(p, 2);
        } else if (size * sizeof(uint64_t) < 64) {
            auto p = reinterpret_cast<Data *>(soa3.alloc());
            debugLog("SOA3 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
            return std::make_tuple(p, 3);
        } else if (size * sizeof(uint64_t) < 128) {
            auto p = reinterpret_cast<Data *>(soa4.alloc());
            debugLog("SOA4 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
            return std::make_tuple(p, 4);
        }
        auto p = reinterpret_cast<Data *>(new size_t[size * sizeof(uint64_t)]);
        debugLog("SYS ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
        return std::make_tuple(p, 0);
    }
    static void innerRelease(Data *p) {
        allocatedCount--;
        debugLog("FREE", std::to_string((size_t)p));
        // small object [soacnt-1]
        switch (p->soacnt) {
        case 0:
            // TODO: destructor
            delete[] reinterpret_cast<size_t *>(p);
            break;
        case 1:
            soa1.release(p);
            break;
        case 2:
            soa2.release(p);
            break;
        case 3:
            soa3.release(p);
            break;
        case 4:
            soa4.release(p);
            break;
        }
    }
    inline static SmallObjectAllocator<16, 256> soa1{1024};
    inline static SmallObjectAllocator<32, 128> soa2{1024};
    inline static SmallObjectAllocator<64, 64> soa3{512};
    inline static SmallObjectAllocator<128, 32> soa4{256};

    // inline static size_t lastGCCount = 0;
    inline static size_t allocatedCount = 0;
    inline static size_t gcThreshold = 1024;
};

} // namespace rulejit