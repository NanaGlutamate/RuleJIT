#pragma once

#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace rulejit {

struct NeverGCGarbageCollector {
    static_assert(8 == sizeof(size_t));
    static size_t *alloc(size_t len) {
        mem.push_back(std::make_unique<size_t[]>(len));
        return mem.back().get();
    }
    // static void assign(size_t **tar, size_t *src) { *tar = src; }
    static void release(size_t *n) {}
    static void retain(size_t *n) {}
    void fullGC() { mem.clear(); }

  private:
    static inline std::list<std::unique_ptr<size_t[]>> mem;
};

} // namespace rulejit