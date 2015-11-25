cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

project (helium)

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

#
# TODO move to external module
if (${CMAKE_C_COMPILER_ID} MATCHES "(GNU|Clang)")

    # add -Wconversion ?
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic -pedantic-errors")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wshadow -Wmissing-prototypes -Wdeclaration-after-statement")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunused -Wfloat-equal -Wpointer-arith -Wwrite-strings -Wformat-security")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-format-attribute -Wundef")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
endif (${CMAKE_C_COMPILER_ID} MATCHES "(GNU|Clang)")

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Module)

enable_testing ()

include (cmocka)

find_package (FLEX)
find_package (BISON)

FLEX_TARGET (Lexer
    ${CMAKE_SOURCE_DIR}/helium/lexer.lex
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

BISON_TARGET (Parser
    ${CMAKE_SOURCE_DIR}/helium/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.c
    VERBOSE parser.output)

ADD_FLEX_BISON_DEPENDENCY (Lexer Parser)

include_directories (${CMAKE_SOURCE_DIR}/helium)

set (HELIUM_SOURCES
    ${CMAKE_SOURCE_DIR}/helium/ext/bitarray.c
    ${CMAKE_SOURCE_DIR}/helium/ext/list.c
    ${CMAKE_SOURCE_DIR}/helium/ext/mem.c
    ${CMAKE_SOURCE_DIR}/helium/ext/stack.c
    ${CMAKE_SOURCE_DIR}/helium/ext/table.c
    ${CMAKE_SOURCE_DIR}/helium/ext/util.c
    ${CMAKE_SOURCE_DIR}/helium/ext/vector.c
    ${CMAKE_SOURCE_DIR}/helium/asm.c
    ${CMAKE_SOURCE_DIR}/helium/ast.c
    ${CMAKE_SOURCE_DIR}/helium/canon.c
    ${CMAKE_SOURCE_DIR}/helium/env.c
    ${CMAKE_SOURCE_DIR}/helium/error.c
    ${CMAKE_SOURCE_DIR}/helium/escape.c
    ${CMAKE_SOURCE_DIR}/helium/flowgraph.c
    ${CMAKE_SOURCE_DIR}/helium/graph.c
    ${CMAKE_SOURCE_DIR}/helium/liveness.c
    ${CMAKE_SOURCE_DIR}/helium/machine.c
    ${CMAKE_SOURCE_DIR}/helium/mipscodegen.c
    ${CMAKE_SOURCE_DIR}/helium/mipsframe.c
    ${CMAKE_SOURCE_DIR}/helium/mipsmachine.c
    ${CMAKE_SOURCE_DIR}/helium/parse.c
    ${CMAKE_SOURCE_DIR}/helium/regalloc.c
    ${CMAKE_SOURCE_DIR}/helium/semant.c
    ${CMAKE_SOURCE_DIR}/helium/symbol.c
    ${CMAKE_SOURCE_DIR}/helium/temp.c
    ${CMAKE_SOURCE_DIR}/helium/translate.c
    ${CMAKE_SOURCE_DIR}/helium/tree.c
    ${CMAKE_SOURCE_DIR}/helium/types.c
    ${CMAKE_SOURCE_DIR}/helium/program.c)

add_executable (Helium_Compiler ${CMAKE_SOURCE_DIR}/helium/main.c
    ${HELIUM_SOURCES}
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS})

set_target_properties (Helium_Compiler PROPERTIES OUTPUT_NAME "helium")

add_subdirectory (test/libhc)

install (
    FILES "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:Helium_Compiler>"
    PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE
    DESTINATION ${CMAKE_INSTALL_PREFIX}/helium)
