#include <deque>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace autograd {

/**
 * @brief Small Object Allocator.
 *
 * @tparam Size: size in byte of each small object.
 * @tparam Len: number of objects in each block.
 *
 *
 */
template <size_t Size, size_t Len>
struct SmallObjectAllocator {
    /**
     * @brief object pool designed with free list.
     *
     */
    struct alignas(Size* Len) FreeList {
        /**
         * @brief node of free list
         *
         */
        struct alignas(Size) Node {
            union {
                char data[Size];
                Node* next;
            };
        } data[Len];

        /**
         * @brief Construct a new Free List object
         *
         * @param[in] head head pointer of SOA, will changed to
         * the last node of new-added block.
         */
        FreeList(Node*& head) {
            Node* tmp = head;
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
    char* alloc() {
        if (!head) {
            extend();
        }
        auto tmp = head;
        head = head->next;
        return tmp.data;
    }

    /**
     * @brief release a small object.
     * @attention no check for p, if p is not a valid pointer or is not allocated
     * by this SOA, undefined behavior will occur.
     *
     * @param p pointer to the object.
     */
    void release(void* p) {
        auto tmp = reinterpret_cast<FreeList::Node*>(p);
        tmp->next = head;
        head = tmp;
    }

  private:
    void extend() { freeList.emplace_back(head); }
    // alignas 2^n
    static_assert(!((Size * Len) & (Size * Len - 1)));
    static_assert(Size >= sizeof(void*));
    FreeList::Node* head = nullptr;
    std::list<FreeList> freeList;
};

template <typename Ty>
    requires std::is_base_of_v<typename MemoryPool<Ty>::ControlBlock, Ty>
struct MemoryPool {
    struct SharedToken {
        explicit SharedToken(Ty* v) noexcept : v(v) {
            v->addStrongRef();
        };
        SharedToken(const SharedToken& o) noexcept : v(o.v) {
            v->addStrongRef();
        };
        SharedToken(SharedToken&& o) noexcept : v(o.v) { o.v = nullptr; };
        ~SharedToken() {
            if (!v) {
                return;
            }
            v->removeStrongRef();
            v = nullptr;
        }

      private:
        Ty* v;
    };
    struct WeakToken {
        explicit WeakToken(const SharedToken& s) noexcept : v(s.v) { v->addWeakRef(); };
        WeakToken(const WeakToken& o) noexcept : v(o.v) { v->addWeakRef(); };
        WeakToken(WeakToken&& o) noexcept : v(o.v) { o.v = nullptr; };
        ~WeakToken() {
            if (!v) {
                return;
            }
            v->removeWeakRef();
            v = nullptr;
        }
        bool isExpired() { return v->isExpired(); }

      private:
        Ty* v;
    };
    struct ControlBlock {
        void addStrongRef() {
            addWeakRef();
            strongCount++;
        }
        void addWeakRef() { totalCount++; }
        void removeStrongRef() {
            strongCount--;
            removeWeakRef();
        }
        void removeWeakRef() {
            totalCount--;
            if (totalCount == 0) {
                src->release(this);
            }
        }
        bool isExpired(){
            return strongCount == 0;
        }
      private:
        MemoryPool* src;
        size_t totalCount;
        size_t strongCount;
    };

    template <typename... Args>
    SharedToken get(Args... args) {
        auto tar = reinterpret_cast<char*>(soa.alloc());
        auto p = new (tar) Ty{std::forward<Args>(args)...} p->totalCount = p->strongCount = 0;
        // noexcept
        return SharedToken{p};
    }

  private:
    void release(Ty* p) { soa->release(p); }
    SmallObjectAllocator<sizeof(Ty), 4 * 1024 / sizeof(Ty)> soa;
};

} // namespace autograd