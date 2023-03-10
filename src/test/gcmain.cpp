#include <iostream>

#include "tools/gc/marksweepgc.hpp"

int main(){
    using namespace std;
    using namespace rulejit;
    uint64_t* root = StaticMarkSweepGarbageCollector::alloc(10);
    uint64_t* frame = StaticMarkSweepGarbageCollector::alloc(5);
    root[0] = (uint64_t)frame;
    for(int i=0;i<5;i++){
        frame[i] = (uint64_t)StaticMarkSweepGarbageCollector::alloc(1);
    }
    StaticMarkSweepGarbageCollector::fullGC();
    root[0] = 0;
    StaticMarkSweepGarbageCollector::fullGC();
    frame = StaticMarkSweepGarbageCollector::alloc(5);
    root[0] = (uint64_t)frame;
    for(int i=0;i<5;i++){
        frame[i] = (uint64_t)StaticMarkSweepGarbageCollector::alloc(1);
    }
    for(size_t i = 0; i<10000;i++){
        StaticMarkSweepGarbageCollector::alloc(1);
    }
}