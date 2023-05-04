#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace tools::myfunc {

/**
 * @brief closure captured by reference
 * 
 * @tparam F function type
 * @tparam Args captured arguments
 */
template <typename F, typename... Args> struct ClosureRef {
    F func;
    std::tuple<Args...> args;
    template <typename... Args2> auto operator()(Args2 &&...args2) {
        if constexpr (std::is_invocable_v<F, Args..., Args2...>) {
            return std::apply(func, std::tuple_cat(std::move(args), std::tuple<Args2...>(std::forward<Args2>(args2)...)));
        } else {
            return ClosureRef<F, Args..., Args2...>{
                func, std::tuple_cat(std::move(args), std::tuple<Args2...>(std::forward<Args2>(args2)...))};
        }
    }
};

/**
 * @brief closure captured by value
 * 
 * @tparam F function type
 * @tparam Args captured arguments
 */
template <typename F, typename... Args> struct ClosureValue {
    F func;
    std::tuple<Args...> args;
    template <typename... Args2> auto operator()(Args2 &&...args2) {
        if constexpr (std::is_invocable_v<F, Args..., Args2...>) {
            return std::apply(func, std::tuple_cat(std::move(args), std::tuple<Args2...>(std::forward<Args2>(args2)...)));
        } else {
            return ClosureValue<F, Args..., std::remove_cvref_t<Args2>...>{
                func, std::tuple_cat(std::move(args), std::tuple<std::remove_cvref_t<Args2>...>(std::forward<Args2>(args2)...))};
        }
    }
};

template <typename F> auto curry_ref(F &&f) {
    return ClosureRef<F>{std::forward<F>(f), {}};
}

template <typename F> auto curry_value(F &&f) {
    return ClosureValue<F>{std::forward<F>(f), {}};
}

} // namespace tools::myfunc