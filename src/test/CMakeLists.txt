add_executable(lexer_test ${FRONTEND_SRC} ${AST_SRC} lexermain.cpp)
add_executable(astprinter_test ${FRONTEND_SRC} ${AST_SRC} astprintermain.cpp)
add_executable(typeparse_test ${FRONTEND_SRC} ${AST_SRC} typeparsemain.cpp)
add_executable(parse_test ${FRONTEND_SRC} ${AST_SRC} parsemain.cpp)
add_executable(semantic_test ${FRONTEND_SRC} ${AST_SRC} semanticmain.cpp)

add_executable(ir_test ${FRONTEND_SRC} ${AST_SRC} ${IR_SRC} irmain.cpp)

add_executable(gc_test ${TOOLS_SRC} gcmain.cpp)
add_executable(imgui_test ${IMGUI_SRC} imguimain.cpp)

add_executable(cq_test ${FRONTEND_SRC} ${AST_SRC} ${CQ_BACKEND_SRC} cqmain.cpp)
add_executable(repl_test ${FRONTEND_SRC} ${AST_SRC} ${CQ_BACKEND_SRC} replmain.cpp)
add_executable(cppbe_test ${FRONTEND_SRC} ${AST_SRC} ${CPP_BACKEND_SRC} cppbemain.cpp)

if(WIN32)
target_link_libraries(imgui_test d3d12.lib d3dcompiler.lib dxgi.lib)
endif()