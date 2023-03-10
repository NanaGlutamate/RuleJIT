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

namespace rulejit {
// template<typename Alloc = std::allocator<size_t>>
struct [[deprecated]]_StaticBaseGarbageCollector {
    using ManagedReference = size_t;
    static void assign(ManagedReference *tar, ManagedReference src) {
        release(*tar);
        *tar = src;
        retain(src);
    }
    static void *getValuePointer(ManagedReference ref) {
        if(isSmallValueObject(ref)){}
        return managedObjects[ref].data.get() + managedObjects[ref].memberRefNum;
    }
    static ManagedReference *getNthRefMemberPtr(ManagedReference ref, size_t index) {
        return &(managedObjects[ref].data.get()[index]);
    }
    static ManagedReference getNthRefMember(ManagedReference ref, size_t index) {
        return managedObjects[ref].data.get()[index];
    }
    static ManagedReference alloc(size_t memberRefNum, size_t valueSize) {
        inline static struct InitJob {
            InitJob() {
                managedObjects.reserve(1024);
            }
        } init;
        size_t ret;
        if (memberRefNum == 0 && valueSize <= 4) {
            // TODO: small value object optimization
        }
        if (unused.empty()) {
            managedObjects.push_back(
                ManagedObjectBase{std::make_unique<size_t[]>(valueSize + memberRefNum), 0, memberRefNum});
            ret = managedObjects.size() - 1;
        } else {
            auto index = unused.top();
            unused.pop();
            managedObjects[index].refCounter = 0;
            managedObjects[index].memberRefNum = memberRefNum;
            managedObjects[index].data = std::make_unique<size_t[]>(valueSize + memberRefNum);
            ret = index;
        }
        for (size_t i = 0; i < managedObjects[ret].memberRefNum; i++) {
            managedObjects[ret].data[i] = 0;
        }
        return ret;
    }
    template <typename T> static ManagedReference alloc(size_t memberRefNum) { return alloc(memberRefNum, sizeof(T)); }
    // template <size_t Length>
    // struct ManagedObject : public ManagedObjectBase{};
  private:
    static void fullGC(){}
    consteval static size_t mask(){return (size_t(1) << sizeof(size_t) * 8 - 1);}
    constexpr static bool isSmallValueObject(size_t ref) { return ref & mask(); }
    // small value object optimization
    struct alignas(4 * 128 * sizeof(size_t)) FreeList {
        struct alignas(4 * sizeof(size_t)) Node {
            union{
                size_t data[4];
                Node* next;
            };
        } data[128];
        FreeList(Node*& head) {
            Node* tmp = head;
            for (size_t i = 0; i < 128; i++) {
                data[i].next = tmp;
                tmp = data + i;
            }
            head = tmp;
        }
    };
    inline static std::list<FreeList> freeList;
    inline static FreeList::Node* head = nullptr;

    static void release(size_t tar) {
        if (tar == 0) {
            return;
        }
        // TODO: small value object
        managedObjects[tar].refCounter--;
        if (managedObjects[tar].refCounter == 0) {
            unused.push(tar);
            for (size_t i = 0; i < managedObjects[tar].memberRefNum; i++) {
                release(managedObjects[tar].data[i]);
            }
        }
    };
    static size_t *getData(size_t ref) {
        if (isSmallValueObject(ref)) {
            // TODO: small value object
        }
    }
    static void retain(size_t tar) { managedObjects[tar].refCounter++; };
    // TODO: small value object optimization
    struct ManagedObjectBase {
        // size_t size;
        std::unique_ptr<size_t[]> data;
        size_t refCounter;
        union{
            size_t memberRefNum;

        };
    };
    inline static std::stack<size_t> unused;
    // root node, cannot referenced
    inline static std::vector<ManagedObjectBase> managedObjects{{nullptr, 1, 0}};
};

} // namespace rulejit