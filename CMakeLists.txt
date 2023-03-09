cmake_minimum_required(VERSION 3.6)
set(PROJ_NAME RuleJIT)
project(${PROJ_NAME})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "D:/Desktop/FinalProj/Code/llvm-install-x86-release-1507-nozlibstd/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(AFTER ${PROJECT_SOURCE_DIR}/src)
include_directories(AFTER ${PROJECT_SOURCE_DIR}/extern)
include_directories(AFTER ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RELEASE_POSTFIX _release)
set(CMAKE_DEBUG_POSTFIX _debug)
set(CMAKE_RELWITHDEBINFO_POSTFIX _relwithdebinfo)
set(CMAKE_MINSIZEREL_POSTFIX _minsizerel)

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /permissive- /Zc:__cplusplus")
endif()

add_subdirectory(src)
add_subdirectory(extern)

# Now build our tools
# add_executable(simple-tool HowToUseLLJIT.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
# llvm_map_components_to_libnames(
#   llvm_libs 
#   Core
#   OrcJIT
#   Support
#   nativecodegen
# )

# Link against LLVM libraries
# target_link_libraries(simple-tool ${llvm_libs})