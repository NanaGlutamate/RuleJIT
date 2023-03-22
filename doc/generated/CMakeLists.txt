cmake_minimum_required(VERSION 3.6)
set(PROJ_NAME testgen)
project(${PROJ_NAME})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /permissive- /Zc:__cplusplus")
endif()

add_executable(${PROJ_NAME} test.cpp)