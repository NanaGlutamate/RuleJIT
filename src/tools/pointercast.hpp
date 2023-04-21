/**
 * @file pointercast.hpp
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

#include <memory>

namespace rulejit {

template <typename Tar, typename Src> std::unique_ptr<Tar> unique_cast(std::unique_ptr<Src> &src) {
    auto p = dynamic_cast<Tar *>(src.get());
    if (!p) {
        return nullptr;
    } else {
        return std::unique_ptr<Tar>(dynamic_cast<Tar *>(src.release()));
    }
}

} // namespace rulejit
