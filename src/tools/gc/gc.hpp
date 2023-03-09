#pragma once

#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace rulejit {

// template<typename Alloc = std::allocator<size_t>>
struct StaticBaseGarbageCollector {
    using ManagedReference = size_t;
    static void assign(ManagedReference *tar, ManagedReference src) {
        release(*tar);
        *tar = src;
        retain(src);
    }
    static ManagedReference *getValuePointer(ManagedReference ref) {
        return managedObjects[ref].data.get() + managedObjects[ref].memberRefNum;
    }
    static ManagedReference *getNthRefMemberPtr(ManagedReference ref, size_t index) {
        return &(managedObjects[ref].data.get()[index]);
    }
    static ManagedReference getNthRefMember(ManagedReference ref, size_t index) {
        return managedObjects[ref].data.get()[index];
    }
    static ManagedReference alloc(size_t valueSize, size_t memberRefNum) {
        inline static struct InitJob {
            InitJob() {
                managedObjects.reserve(1024);
                freeList.reserve(8);
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
    template <typename T> static ManagedReference alloc(size_t memberRefNum) { return alloc(sizeof(T), memberRefNum); }
    // template <size_t Length>
    // struct ManagedObject : public ManagedObjectBase{};
  private:
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
        size_t memberRefNum;
    };
    inline static std::stack<size_t> unused;
    // root node, cannot referenced
    inline static std::vector<ManagedObjectBase> managedObjects{{nullptr, 1, 0}};
    // small value object optimization
    struct alignas(4 * 128 * sizeof(size_t)) FreeList {
        FreeList(size_t start) {
            for (size_t i = 0; i < 128; i++) {
                data[i].data[0] = start - i;
            }
        }
        struct alignas(4 * sizeof(size_t)) Node {
            size_t data[4];
        };
        Node data[128];
    };
    inline static std::vector<FreeList> freeList;
    inline static size_t border = size_t(-1);
    inline static size_t head = size_t(-1);
    static bool isSmallValueObject(size_t ref) { return ref > border; }
};

} // namespace rulejit