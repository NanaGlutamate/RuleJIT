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
#include <type_traits>

namespace tools::myunique {

/**
 * @brief cast a unique_ptr to another unique_ptr
 * 
 * @attention if dynamic_cast is legal, return a new unique_ptr and clear the original unique_ptr,
 * otherwise return nullptr and donot change src
 * 
 * @tparam Tar target type that unique_ptr point to (return std::unique_ptr<Tar>)
 * @tparam Src source type that unique_ptr point to, for auto deduction (given std::unique_ptr<Src>)
 * @param src source unique pointer
 * @return std::unique_ptr<Tar> casted unique pointer
 */
template <typename Tar, typename Src> [[deprecated]] std::unique_ptr<Tar> unique_cast(std::unique_ptr<Src> &src) {
    if constexpr (std::is_base_of_v<Tar, Src>) {
        return std::move(src);
    } else {
        auto p = dynamic_cast<Tar *>(src.get());
        if (!p) {
            return nullptr;
        } else {
            return std::unique_ptr<Tar>(dynamic_cast<Tar *>(src.release()));
        }
    }
}

} // namespace rulejit
