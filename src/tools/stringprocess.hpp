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
        if (first) {
            first = false;
        } else {
            result += middle;
        }
        result += item;
    }
    return result;
}

struct StringJoinner {
    template <std::ranges::range Ty> friend std::string operator|(Ty &&tar, const StringJoinner &j) {
        return join(std::forward<Ty>(tar), j.middle);
    }
    std::string middle;
};

template <typename S> inline StringJoinner join(S &&middle) { return StringJoinner{std::forward<S>(middle)}; }

template <typename Mid> struct StringSplitter {
    Mid middle;
    // TODO: wide string support
    struct SplitView {
        std::string_view src;
        Mid middle;
        struct EndIterator {};
        struct Iterator {
            std::string_view src;
            Mid middle;
            decltype(src.find(middle)) pos;
            std::string_view operator*() const {
                // no need to check pos == std::string_view::npos
                return src.substr(0, pos);
            }
            Iterator &operator++() {
                if (pos == std::string_view::npos) {
                    // use middle to mark end
                    middle = {};
                } else {
                    src = src.substr(pos + middle.size());
                    pos = src.find(middle);
                }
                return *this;
            }
            bool operator==(const EndIterator &) const { return middle.empty(); }
            bool operator!=(const EndIterator &) const { return !middle.empty(); }
        };
        Iterator begin() const { return Iterator{src, middle, src.find(middle)}; }
        EndIterator end() const { return EndIterator{}; }
    };
    SplitView friend operator|(std::string_view s, const StringSplitter &mid) {
        return SplitView{s, mid.middle};
    }
    SplitView friend operator|(std::string_view s, StringSplitter &&mid) {
        return SplitView{s, std::move(mid.middle)};
    }
};

template <typename T> auto split_view(T &&middle) {
    // TODO: char middle
    return StringSplitter<std::string>{std::forward<T>(middle)};
}

} // namespace rulejit::mystr