include_directories (${CMAKE_SOURCE_DIR}/helium)

macro(add_helium_test name file)
    add_cmocka_test (${ARGV0} ${ARGV1}
        ${HELIUM_SOURCES}
        # TODO solve these dependencies
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS})
endmacro()

# TODO make it separate
# add_helium_test(Test_BitArray bitarray.c)
# add_helium_test(Test_Vector vector.c)
# add_helium_test(Test_String str.c)
# add_helium_test(Test_Table table.c)
# add_helium_test(Test_Stack stack.c)

# add_helium_test(Escaping escaping.c)
