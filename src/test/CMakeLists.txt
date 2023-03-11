add_executable(lexer_test ${FRONTEND_SRC} lexermain.cpp)
add_executable(astprinter_test ${FRONTEND_SRC} astprintermain.cpp)
add_executable(typeparse_test ${FRONTEND_SRC} ${AST_SRC} typeparsemain.cpp)
add_executable(parse_test ${FRONTEND_SRC} parsemain.cpp)

add_executable(gc_test ${TOOLS_SRC} gcmain.cpp)

add_executable(cq_test ${BACKEND_SRC} cqmain.cpp)