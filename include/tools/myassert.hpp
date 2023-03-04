#pragma once

#if __cplusplus >= 202002L

#include <string>
#include <iostream>
#include <source_location>
#include <format>

inline void my_assert(bool check,
    const std::string& message = "",
    const std::source_location location = std::source_location::current())
{
    using namespace std;
    if(!check){
        cout << format("error: {}\nin file {}, line {}", message, location.file_name(), location.line()) << endl;
        abort();//throw std::runtime_error{""};
    }
}

#else

#ifdef assert

#define my_assert(check, message) assert(check)

#else

#include <string>
#include <iostream>

inline void my_assert(bool check, std::string message){
    if(!check){
        std::cout << message << std::endl;
        throw std::logic_error(message);
    }
}

#endif

#endif