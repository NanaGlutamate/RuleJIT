/**
 * @file myassert.hpp
 * @author djw
 * @brief Tools/Assert
 * @date 2023-03-28
 *
 * @details provides assertion function.
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-03-28</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include "defines/marco.hpp"

#ifndef __DISABLE_ASSERT

#if __cplusplus >= 202002L

#include <format>
#include <iostream>
#include <source_location>
#include <string>

#include "tools/seterror.hpp"

inline int my_assert(bool check, const std::string &message = "[assertion failed with no info provided]",
                      const std::source_location location = std::source_location::current()) {
    using namespace std;
    if (!check) {
        error(format("error: {}\nin file {}, line {}", message, location.file_name(), location.line()));
    }
    return 0;
}

#else // #if __cplusplus >= 202002L

#ifdef assert

#define my_assert(check, message) (assert(check), 0)

#else // #ifdef assert

#include <iostream>
#include <string>

#include "tools/seterror.hpp"

inline int my_assert(bool check, std::string message) {
    if (!check) {
        std::cout << message << std::endl;
        error(message);
    }
    return 0;
}

#endif // #ifdef assert

#endif // #if __cplusplus >= 202002L

#else // #ifdef __ACTIVE_ASSERT

inline int my_assert(bool check, std::string message) {
    // do nothing
    return 0;
}

#endif // #ifdef __ACTIVE_ASSERT