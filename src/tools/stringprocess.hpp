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

namespace rulejit::mystr {

template <std::ranges::range _Range> inline std::string join(_Range &&range, const std::string &middle) {
    std::string result;
    bool first = true;
    for (auto &&item : range) {
        static_assert(std::is_same_v<std::string, std::remove_cvref_t<decltype(item)>> ||
                          std::is_same_v<std::string_view, std::remove_cvref_t<decltype(item)>>,
                      "join only support std::string and std::string_view");
        if (first) {
            first = false;
        } else {
            result += middle;
        }
        if constexpr (std::is_constructible_v<std::string, decltype(item)>) {
            result += item;
        }
    }
    return result;
}

struct StringJoinner {
    template <std::ranges::range Ty> friend std::string operator|(Ty &&tar, const StringJoinner &j) {
        return join(std::forward<Ty>(tar), j.middle);
    }
    std::string middle;
};

/**
 * @brief join a range of string with a middle string
 *
 * @tparam S middle string type, must be std::string
 * @param middle middle string
 * @return a struct to join a range of string
 */
template <typename S>
    requires requires(S s) { std::string(s); }
inline StringJoinner join(S &&middle) {
    return StringJoinner{std::forward<S>(middle)};
}

inline std::string repeat(const std::string &str, size_t times) {
    std::string result;
    for (size_t i = 0; i < times; i++) {
        result += str;
    }
    return result;
}

struct StringBuilder {
    template <typename Range>
    friend std::string operator| (Range &&range, const StringBuilder &builder) {
        std::string result;
        for (auto &&item : range) {
            result += item;
        }
        return result;
    }
};

inline StringBuilder buildString(){
    return StringBuilder{};
}

// template <typename Mid> struct StringSplitter {
//     Mid middle;
//     // TODO: wide string support
//     struct SplitView {
//         std::string_view src;
//         Mid middle;
//         struct EndIterator {};
//         struct Iterator {
//             std::string_view src;
//             Mid middle;
//             decltype(src.find(middle)) pos;
//             std::string_view operator*() const {
//                 // no need to check pos == std::string_view::npos
//                 return src.substr(0, pos);
//             }
//             Iterator &operator++() {
//                 if (pos == std::string_view::npos) {
//                     // use middle to mark end
//                     middle = {};
//                 } else {
//                     src = src.substr(pos + middle.size());
//                     pos = src.find(middle);
//                 }
//                 return *this;
//             }
//             bool operator==(const EndIterator &) const { return middle.empty(); }
//             bool operator!=(const EndIterator &) const { return !middle.empty(); }
//         };
//         Iterator begin() const { return Iterator{src, middle, src.find(middle)}; }
//         EndIterator end() const { return EndIterator{}; }
//     };
//     SplitView friend operator|(std::string_view s, const StringSplitter &mid) {
//         return SplitView{s, mid.middle};
//     }
//     SplitView friend operator|(std::string_view s, StringSplitter &&mid) {
//         return SplitView{s, std::move(mid.middle)};
//     }
// };

// /**
//  * @brief split a string by a middle string. if middle string is empty, return empty range;
//  * if middle string is not found or original string is empty, return a range contains original string.
//  *
//  * @tparam T type of middle string, must be std::string
//  * @param middle middle string
//  * @return a range of string_view
//  */
// template <typename T>
//     requires requires (T s) {std::string(s);}
// auto split_view(T &&middle) {
//     // TODO: char as middle
//     return StringSplitter<std::string>{std::forward<T>(middle)};
// }

} // namespace rulejit::mystr