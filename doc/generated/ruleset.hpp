#pragma once

#include <vector>
#include <functional>

#include "typedef.hpp"
#include "funcdef.hpp"

namespace ruleset{

struct RuleSet{
    Input in;
    Output out;
    Cache cache;
    CSValueMap out_map;
    RuleSet() = default;
    void Init(){
        
    }
    CSValueMap* GetOutput(){
        out_map = out.ToValueMap();
        return &out_map;
    }
    void SetInput(const CSValueMap& map){
        in.FromValueMap(map);
    }
    void Tick(){
        subRuleSet0.Tick(*this);
        subRuleSet1.Tick(*this);

        subRuleSet0.writeBack(*this);
        subRuleSet1.writeBack(*this);
    }
    
    struct {
        Cache cache;
        std::unordered_map<std::string, std::function<void(Cache*, Cache*)>> modified;
        template <typename T>
        void loadCache(RuleSet& base, T p, const std::string& name){
            if(auto it = modified.find(name); it == modified.end()){
                auto origin = base.cache.*p;
                cache.*p = base.cache.*p;
                modified.emplace(name, [origin, p](Cache* src, Cache* dst){
                    // only 1 write back to a same cached value valid through each subruleset,
                    // so if src->*p == tmp, there must no write back by this subruleset,
                    // or may write back same value.
                    // in the second case, assume no other write back requires to this value,
                    // so no need to write back.
                    if(src->*p == origin)return;
                    dst->*p = src->*p;
                });
            }
        }
        int Tick(RuleSet& base){
            return ((((double((loadCache(base, &Cache::Cache1, "Cache1"), cache.Cache1)) <= double((10)))) ? ([&](){(base.out.Output1) = double(((0)));return (0);}()) : (((double((loadCache(base, &Cache::Cache1, "Cache1"), cache.Cache1)) > double((10)))) ? ([&](){(base.out.Output1) = double(((1)));return (1);}()) : ((-((1)))))));
        }
        void writeBack(RuleSet& base){
            for(auto&& [_, f] : modified){
                f(&cache, &base.cache);
            }
            modified.clear();
        }
    }subRuleSet0;

    struct {
        Cache cache;
        std::unordered_map<std::string, std::function<void(Cache*, Cache*)>> modified;
        template <typename T>
        void loadCache(RuleSet& base, T p, const std::string& name){
            if(auto it = modified.find(name); it == modified.end()){
                auto origin = base.cache.*p;
                cache.*p = base.cache.*p;
                modified.emplace(name, [origin, p](Cache* src, Cache* dst){
                    // only 1 write back to a same cached value valid through each subruleset,
                    // so if src->*p == tmp, there must no write back by this subruleset,
                    // or may write back same value.
                    // in the second case, assume no other write back requires to this value,
                    // so no need to write back.
                    if(src->*p == origin)return;
                    dst->*p = src->*p;
                });
            }
        }
        int Tick(RuleSet& base){
            return ((((double((base.in.Input1)) != double((0)))) ? ([&](){(loadCache(base, &Cache::Cache1, "Cache1"), cache.Cache1) = double(((double((loadCache(base, &Cache::Cache1, "Cache1"), cache.Cache1)) + double((1)))));return (0);}()) : (((double((base.in.Input1)) == double((0)))) ? ([&](){(loadCache(base, &Cache::Cache1, "Cache1"), cache.Cache1) = double(((double((loadCache(base, &Cache::Cache1, "Cache1"), cache.Cache1)) - double((1)))));return (1);}()) : ((-((1)))))));
        }
        void writeBack(RuleSet& base){
            for(auto&& [_, f] : modified){
                f(&cache, &base.cache);
            }
            modified.clear();
        }
    }subRuleSet1;

};

}
