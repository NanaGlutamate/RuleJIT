cmake_minimum_required(VERSION 3.6)

project(RuleJIT)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJ_NAME RuleJIT)

# Now build our tools
add_library(${PROJ_NAME} SHARED HowToUseLLJIT.cpp)

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
# target_link_libraries(Test ${llvm_libs})