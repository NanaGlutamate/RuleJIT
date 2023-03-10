#pragma once

#define __RULEJIT_GC_DEBUG

#ifdef __RULEJIT_GC_DEBUG
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

#include "soa.hpp"

// struct content{
//     uint32_t referenceCount, totalCount;
//     size_t reference[referenceCount];
//     size_t value[totalCount - referenceCount];
//     content nextFram[];
// };

namespace rulejit {

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
            Init2(Data *dst) { root = dst; }
        } init2(dst);
        return dst->data;
    }
    static void releaseLastAllocated() {
        if (!head)
            return;
        auto tmp = head->next;
        innerRelease(head);
        head = tmp;
    }
    static void tryReleaseInSet(std::unordered_set<uint64_t *> &tar) {
        while (head && tar.contains(reinterpret_cast<uint64_t *>(head) + HeadSize)) {
            auto tmp = head->next;
            innerRelease(head);
            head = tmp;
        }
    }
    static void fullGC() {
#ifdef __RULEJIT_GC_DEBUG
        auto tmp = allocatedCount;
#endif
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
#ifdef __RULEJIT_GC_DEBUG
        log("FULL GC", std::format("obj number: {}->{}", tmp, allocatedCount));
#endif
    }

  private:
#ifdef __RULEJIT_GC_DEBUG
    static void log(const std::string &type, const std::string &msg) {
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
#endif
    static void tryFree() {
        if (allocatedCount > gcThreshold) {
#ifdef __RULEJIT_GC_DEBUG
            auto tmp = gcThreshold;
#endif
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
#ifdef __RULEJIT_GC_DEBUG
            log("AUTO GC", std::format("change GC Threshold from {} to {}", tmp, gcThreshold));
#endif
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
#ifdef __RULEJIT_GC_DEBUG
            log("SOA1 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
#endif
            return std::make_tuple(p, 1);
        } else if (size * sizeof(uint64_t) < 32) {
            auto p = reinterpret_cast<Data *>(soa2.alloc());
#ifdef __RULEJIT_GC_DEBUG
            log("SOA2 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
#endif
            return std::make_tuple(p, 2);
        } else if (size * sizeof(uint64_t) < 64) {
            auto p = reinterpret_cast<Data *>(soa3.alloc());
#ifdef __RULEJIT_GC_DEBUG
            log("SOA3 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
#endif
            return std::make_tuple(p, 3);
        } else if (size * sizeof(uint64_t) < 128) {
            auto p = reinterpret_cast<Data *>(soa4.alloc());
#ifdef __RULEJIT_GC_DEBUG
            log("SOA4 ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
#endif
            return std::make_tuple(p, 4);
        }
        auto p = reinterpret_cast<Data *>(new size_t[size * sizeof(uint64_t)]);
#ifdef __RULEJIT_GC_DEBUG
        log("SYS ALLOC", std::to_string(size) + " uint64_t at " + std::to_string((size_t)p));
#endif
        return std::make_tuple(p, 0);
    }
    static void innerRelease(Data *p) {
        allocatedCount--;
#ifdef __RULEJIT_GC_DEBUG
        log("FREE", std::to_string((size_t)p));
#endif
        // small object [soacnt-1]
        switch (p->soacnt) {
        case 0:
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