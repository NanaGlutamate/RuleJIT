link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(GLOB_RECURSE _SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/*.C ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(${PROJ_NAME}_interpreter SHARED ${_SRC} ${FRONTEND_SRC} ${AST_SRC} ${CQ_BACKEND_SRC})

if(UNIX)
target_link_libraries(cq_interpreter dl)
else(UNIX)
endif(UNIX)
