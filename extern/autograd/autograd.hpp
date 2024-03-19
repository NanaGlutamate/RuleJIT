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
struct MemoryPool {
    template<typename ...Args>
    SharedToken get(Args... args) {
        // noexcept
        return std::shared_ptr(new (reinterpret_cast<char*>(soa.alloc())) Ty{std::forward<Args>(args)...}, [this](auto p) {
            this->soa.release(p);
        });
    }

  private:
    SmallObjectAllocator<sizeof(Ty), 4 * 1024 / sizeof(Ty)> soa;
};

template <typename Element>
struct FrontAutoGradManager {
    struct Var : public MemoryPool<Var>::TakedType {
        Element value;
        std::unordered_map<Var*, Element> grad;
    };

    struct Token : public MemoryPool<Var>::SharedToken {
        explicit Token(Var* var) noexcept : var(var) {}

      private:
        Var* val;
    };

    Token getVar(Var initValue, bool requiresGrad = false) noexcept {}

  private:
    MemoryPool<Var> value;
};

} // namespace autograd