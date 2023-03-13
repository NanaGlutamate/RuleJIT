#pragma once

#include <list>
#include <memory>
#include <stack>
#include <vector>

#include "tools/gc/soa.hpp"

namespace rulejit {

struct NeverGCGarbageCollector {
    static_assert(8 == sizeof(size_t));
    static size_t *alloc(size_t len) {
        mem.push_back(std::make_unique<size_t[]>(len));
        return mem.back().get();
    }
    void fullGC() { mem.clear(); }

  private:
    static inline std::vector<std::unique_ptr<size_t[]>> mem;
};

} // namespace rulejit