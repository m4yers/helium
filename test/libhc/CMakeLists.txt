include_directories (${CMAKE_SOURCE_DIR}/compiler)

macro(add_helium_test name file)
    add_cmocka_test (${ARGV0} ${ARGV1}
        ${HELIUM_SOURCES}
        # TODO solve these dependencies
        ${BISON_HeliumParser_OUTPUTS}
        ${FLEX_HeliumLexer_OUTPUTS}
        ${BISON_MIPSParser_OUTPUTS}
        ${FLEX_MIPSLexer_OUTPUTS})
endmacro()

add_helium_test(Test_Bench bench.c)

# TODO make it separate
# add_helium_test(Test_BitArray bitarray.c)
# add_helium_test(Test_Vector vector.c)
# add_helium_test(Test_String str.c)
# add_helium_test(Test_List list.c)
# add_helium_test(Test_Pair pair.c)
# add_helium_test(Test_Table table.c)
# add_helium_test(Test_Stack stack.c)

# add_helium_test(Escaping escaping.c)
