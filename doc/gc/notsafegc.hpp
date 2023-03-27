#pragma once

#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace rulejit {

// cannot deal with circular reference
struct [[deprecated]]SimpleGarbageCollector{
    static_assert(8==sizeof(size_t));
    static size_t* alloc(size_t len){
        mem.push_back(std::make_unique<size_t[]>(len+2));
        auto it = &(mem.back());
        auto p = mem.back().get();
        p[0] = reinterpret_cast<size_t>(it);
        memset(p+1, 0, sizeof(size_t)*(len+1));
        return p+2;
    }
    static void assign(size_t** tar, size_t* src){
        release(*tar);
        *tar = src;
        retain(src);
    }
    static void release(size_t* n){
        if(n){
            n[-1]--;
            if(n[-1] == 0){
                auto p = reinterpret_cast<std::unique_ptr<size_t []> *>(n[-2]);
                *p = nullptr;
            }
        }
    }
    static void retain(size_t* n){
        if(n){
            n[-1]++;
        }
    }
    void fullGC(){
        auto it = mem.begin();
        while(it != mem.end()){
            if(*it){
                it++;
            }else{
                it = mem.erase(it);
            }
        }
    }
private:
    static inline std::list<std::unique_ptr<size_t[]>> mem;
};

}