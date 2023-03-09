#pragma once

#include <memory>

namespace rulejit {

template<typename Alloc = std::allocator<size_t>>
struct NoClearGarbageCollector{
    void clear(){}
};

}