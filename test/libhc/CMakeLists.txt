include_directories (${CMAKE_SOURCE_DIR}/helium)

macro(add_helium_test name file)
    add_cmocka_test (${ARGV0} ${ARGV1}
        ${HELIUM_SOURCES}
        # TODO solve these dependencies
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS})
endmacro()

add_helium_test(BitArray bitarray.c)
add_helium_test(Vector vector.c)
add_helium_test(Table table.c)
add_helium_test(Stack stack.c)

add_helium_test(Escaping escaping.c)

# add_cmocka_test (ScannerTest scanner_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# TODO: do not compile in all the sources here
# add_cmocka_test (TableTest table_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# add_cmocka_test (VectorTest vector_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# add_cmocka_test (StackTest stack_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})
#
# add_cmocka_test (BitArrayTest bitarray_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# add_cmocka_test (LivenessTest liveness_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# add_cmocka_test (RegAllocTest regalloc_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})
#
# add_cmocka_test (SemantTest semant_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})
#
# add_cmocka_test (EscapeTest escape_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})
#
# add_cmocka_test (MIPSAsmTest mips_asm_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})
#
# add_cmocka_test (ProgramTest program_test.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# add_cmocka_test (TestAST test_ast.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})

# add_cmocka_test (TestProgram test_program.c
#     ${HELIUM_SOURCES}
#     ${BISON_Parser_OUTPUTS}
#     ${FLEX_Lexer_OUTPUTS})
