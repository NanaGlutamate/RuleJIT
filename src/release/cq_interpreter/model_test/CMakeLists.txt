file(GLOB SRCS *.cpp *.cc *.h)
file(GLOB CSCONFIG_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../csconfig/*.cc)

# include_directories(
#   ${CMAKE_SOURCE_DIR}/src/csmodel_base
#   ${CMAKE_SOURCE_DIR}/src/csconfig
#   ${CMAKE_SOURCE_DIR}/external
# )

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if(WIN32)
add_compile_options(/utf-8)
else(WIN32)
link_libraries(dl)
endif(WIN32)

add_executable(cq_dll_test ${SRCS} ${CSCONFIG_SRCS})
add_dependencies(cq_dll_test cq_interpreter)
