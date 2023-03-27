#pragma once

#include <list>
#include <memory>
#include <stack>
#include <vector>

#define __RULEJIT_GC_DEBUG_ASSERTION

// struct content{
//     int32_t referenceCount, totalCount;
//     size_t reference[referenceCount];
//     size_t value[totalCount - referenceCount];
//     content nextFram[];
// };

namespace rulejit::gc {
// template<typename Alloc = std::allocator<size_t>>
struct [[deprecated]]StaticExtendableMarkSweepGarbageCollector {
    using DataType = int64_t;
    struct chunk {
        int32_t referenceCount, totalCount;
        int64_t data[];
    };
    struct data {
        data *pre, *next;
        int8_t mark, issmall;
        int16_t chunkNum;
        // num of int64_t contained
        int32_t capacity, size;
        int64_t data[];
    };
    static const size_t HeadSize = sizeof(data) / sizeof(int64_t) - 1;
    static_assert(sizeof(data) == sizeof(int64_t) * 4);
    static_assert(sizeof(data *) == 8);
    static int64_t *extend(int64_t *origin, size_t newSize) {
        data *src = reinterpret_cast<data *>(origin - HeadSize);
        auto size = src->size;
        auto capacity = src->capacity;
        src->size = newSize;
        if (newSize <= capacity) {
            return origin;
        }
        data *dst = innerAlloc(((capacity + HeadSize) * 3) >> 1);
        memcpy(dst, src, (HeadSize + size) * sizeof(int64_t));
        if (dst->pre)
            dst->pre->next = dst;
        if (dst->next)
            dst->next->pre = dst;
        dst->capacity = ((capacity + HeadSize) * 3) >> 1 - HeadSize;
    }
    static int64_t *alloc(size_t size){
        data *dst = innerAlloc(size + HeadSize);
        dst->size = size;
        dst->capacity = size;
        return dst->data;
    }
  private:
    // size of int64_t
    inline static data* root = nullptr;
    inline static data tail;
    static data *innerAlloc(size_t size) {
        // TODO: SOA
    }
};

} // namespace rulejit