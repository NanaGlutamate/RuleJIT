#pragma once

#include <ranges>

namespace tools::myrange {

template <std::ranges::range... Args> auto zip(const Args &...args) {
    struct ZipView {
        struct Iterator {};
        Iterator begin() const;
        Iterator end() const;
    };
    return ZipView{};
}

} // namespace tools::myrange