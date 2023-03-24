#pragma once

#include "defines/marco.hpp"

#ifndef __DISABLE_ASSERT

#if __cplusplus >= 202002L

#include <format>
#include <iostream>
#include <source_location>
#include <string>

inline void my_assert(bool check, const std::string &message = "[assertion failed with no info provided]",
                      const std::source_location location = std::source_location::current()) {
    using namespace std;
    if (!check) {
        throw std::logic_error{
            format("error: {}\nin file {}, line {}", message, location.file_name(), location.line())};
    }
}

#else // #if __cplusplus >= 202002L

#ifdef assert

#define my_assert(check, message) assert(check)

#else // #ifdef assert

#include <iostream>
#include <string>

inline void my_assert(bool check, std::string message) {
    if (!check) {
        std::cout << message << std::endl;
        throw std::logic_error(message);
    }
}

#endif // #ifdef assert

#endif  #if __cplusplus >= 202002L

#else // #ifdef __ACTIVE_ASSERT

inline void my_assert(bool check, std::string message) {
    // do nothing
}

#endif // #ifdef __ACTIVE_ASSERT