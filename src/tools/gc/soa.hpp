/**
 * @file soa.hpp
 * @author djw
 * @brief GC/Small Object Allocator
 * @date 2023-03-28
 * 
 * @details Includes a small object allocator, which is used by GC.
 * 
 * if __RULEJIT_SOA_VIRTUAL defined, all small object allocator will 
 * inherit from ISmallObjectAllocator.
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include "defines/marco.hpp"

#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace rulejit::gc {

#ifdef __RULEJIT_SOA_VIRTUAL
struct ISmallObjectAllocator {
    virtual void *alloc() = 0;
    virtual void release(void *p) = 0;
    virtual ~ISmallObjectAllocator() = default;
};
#endif

/**
 * @brief Small Object Allocator.
 * 
 * @tparam Size: size in byte of each small object.
 * @tparam Len: number of objects in each block.
 * 
 * @attention if __RULEJIT_SOA_VIRTUAL defined, all small object allocator will 
 * inherit from ISmallObjectAllocator.
 * 
 */
template <size_t Size, size_t Len>
struct SmallObjectAllocator
#ifdef __RULEJIT_SOA_VIRTUAL
    : public ISmallObjectAllocator
#endif
{
    /**
     * @brief object pool designed with free list.
     * 
     */
    struct alignas(Size *Len) FreeList {
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
     */
    SmallObjectAllocator(size_t reserve) {
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
            extend();
        }
        auto tmp = head;
        head = head->next;
        return reinterpret_cast<void *>(tmp);
    }

    /**
     * @brief release a small object.
     * @attention no check for p, if p is not a valid pointer or is not allocated
     * by this SOA, undefined behavior will occur.
     * 
     * @param p pointer to the object.
     */
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