add_executable(lexer_test ${FRONTEND_SRC} ${AST_SRC} lexermain.cpp)
add_executable(astprinter_test ${FRONTEND_SRC} ${AST_SRC} astprintermain.cpp)
add_executable(typeparse_test ${FRONTEND_SRC} ${AST_SRC} typeparsemain.cpp)
add_executable(parse_test ${FRONTEND_SRC} ${AST_SRC} parsemain.cpp)
add_executable(semantic_test ${FRONTEND_SRC} ${AST_SRC} semanticmain.cpp)

add_executable(ir_test ${FRONTEND_SRC} ${AST_SRC} ${IR_SRC} irmain.cpp)

add_executable(gc_test ${TOOLS_SRC} gcmain.cpp)

add_executable(repl_test ${FRONTEND_SRC} ${AST_SRC} ${CQ_BACKEND_SRC} replmain.cpp)
add_executable(cq_test ${FRONTEND_SRC} ${RULESET_SRC} ${AST_SRC} ${CQ_BACKEND_SRC} cqmain.cpp)
add_executable(cppbe_test ${FRONTEND_SRC} ${RULESET_SRC} ${AST_SRC} ${CPP_BACKEND_SRC} cppbemain.cpp)