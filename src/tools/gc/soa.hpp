#pragma once

#include "defines/marco.hpp"

#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace rulejit {

struct ISmallObjectAllocator {
    virtual void *alloc() = 0;
    virtual void release(void *p) = 0;
    virtual ~ISmallObjectAllocator() = default;
};

template <size_t Size, size_t Len>
struct SmallObjectAllocator
#ifdef __RULEJIT_SOA_VIRTUAL
    : public ISmallObjectAllocator
#endif
{
    struct alignas(Size *Len) FreeList {
        struct alignas(Size) Node {
            union {
                char data[Size];
                Node *next;
            };
        } data[Len];
        FreeList(Node *&head) {
            Node *tmp = head;
            for (size_t i = 0; i < Len; i++) {
                data[i].next = tmp;
                tmp = data + i;
            }
            head = tmp;
        }
    };
    //! @param reserve: number of objects to reserve
    SmallObjectAllocator(size_t reserve) {
        for (size_t i = 0; i < (reserve + Len - 1) / Len; i++) {
            freeList.emplace_back(head);
        }
    }
    void *alloc() {
        if (!head) {
            extend();
        }
        auto tmp = head;
        head = head->next;
        return reinterpret_cast<void *>(tmp);
    }
    void release(void *p) {
        auto tmp = reinterpret_cast<FreeList::Node *>(p);
        tmp->next = head;
        head = tmp;
    }

  private:
    void extend() { freeList.emplace_back(head); }
    // alignas 2^n
    static_assert(!((Size * Len) & (Size * Len - 1)));
    static_assert(Size >= sizeof(void *));
    FreeList::Node *head = nullptr;
    std::list<FreeList> freeList;
};

} // namespace rulejit