/**
 * @file stringprocess.hpp
 * @author djw
 * @brief 
 * @date 2023-04-21
 * 
 * @details 
 * 
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-04-21</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <string>
#include <vector>

namespace rulejit::mystr{

template <typename _Range>
inline std::string join(_Range&& range, const std::string& middle){
    std::string result;
    bool first = true;
    for(auto&& item : range){
        if(first){
            first = false;
        }else{
            result += middle;
        }
        result += item;
    }
    return result;
}

struct StringJoinner{
    template<typename Ty>
    friend std::string operator|(Ty&& tar, const StringJoinner& j){
        return join(std::forward<Ty>(tar), j.middle);
    }
    std::string middle;
};

template <typename S>
inline StringJoinner join(S&& middle){
    return StringJoinner{std::forward<S>(middle)};
}

// std::vector<std::string> split(const std::string& src, const std::string& middle){
//
// }
//
// SplitView split_view(const std::string& src, const std::string& middle){
//    
// }

}