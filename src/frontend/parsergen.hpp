/**
 * @file parsergen.hpp
 * @author djw
 * @brief FrontEnd/Parser generator
 * @date 2023-03-28
 * 
 * @details Parser generator like Bison, not used for now
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace parsergen {

// s=="" means shift state
// if symbol, s=symbol; else s=TokenType
using TransCondition = std::string;

// compressed Tire Tree
struct StringCheckMerger {
    struct Item{
        char c;
        size_t next;
    };
    std::vector<Item> items;
    StringCheckMerger(const std::set<std::string>& accepted){
        // do not support string with '\0'
        // build Tire Tree, then compress Tire Tree to vector<Item>
        // std::map<std::string, size_t> prefix_index;
        std::map<size_t, std::map<char, size_t>> node_jump_table{{0, {}}};
        size_t first_unused_index=1;
        for(auto&& s : accepted){
            size_t curr_node=0;
            for(auto&& c : s){
                if(auto it = node_jump_table[curr_node].find(c); it!=node_jump_table[curr_node].end()){
                    curr_node = it->second;
                }else{
                    node_jump_table[curr_node][c] = first_unused_index;
                    node_jump_table[first_unused_index] = {};
                    curr_node = first_unused_index;
                    ++first_unused_index;
                }
            }
            node_jump_table[curr_node]['\0'] = size_t(-1); // 0;
        }
        size_t total_size = 0;
        for(auto&& [ind, sub] : node_jump_table){
            total_size += 1 + sub.size();
        }
        items.resize(total_size);
        size_t free = 0;
        // compress
        std::map<size_t, size_t> allocated;
        for(auto&& [ind, sub] : node_jump_table){
            size_t self;
            if(auto it = allocated.find(ind); it!=allocated.end()){
                self = it->second;
            }else{
                self = free;
                free += 1 + sub.size();
            }
            items[self].c = '\0';
            items[self].next = sub.size();
            for(auto&& [c, next] : sub){
                if(auto it = allocated.find(next); it!=allocated.end()){
                    items[self+1].c = c;
                    items[self+1].next = it->second;
                    ++self;
                }else{
                    allocated[next] = free;
                    items[self+1].c = c;
                    items[self+1].next = free;
                    ++self;
                    free += 1 + node_jump_table[next].size();
                }
            }
        }
    }
    bool accept(const std::string& s){
        auto self = items.begin();
    }
private:
    // void emplaceNode(const std::map<char, size_t>& subNode, std::map<size_t, size_t>& rename){}
};

struct FSMNode {
    inline static std::vector<size_t> empty{};
    std::map<std::string, std::vector<size_t>> succ;
    std::vector<size_t>& shift(){
        return accept("");
    }
    std::vector<size_t>& accept(const std::string& s){
        if(auto it = succ.find(s); it!=succ.end()){
            return it->second;
        }
        return empty;
    }
};



// bool canAccept(const std::string& s)override{
// }
// int accept(const std::string& s, int currState)override{
// }

} // namespace parsergen