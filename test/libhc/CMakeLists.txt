include_directories (${CMAKE_SOURCE_DIR}/compiler)

macro(add_util_test name file)
    add_cmocka_test (${ARGV0} ${ARGV1}
        ${UTIL_SOURCES})
endmacro()

macro(add_helium_test name file)
    add_cmocka_test (${ARGV0} ${ARGV1}
        ${UTIL_SOURCES}
        ${HELIUM_SOURCES}
        ${BISON_HeliumParser_OUTPUTS}
        ${FLEX_HeliumLexer_OUTPUTS}
        ${BISON_MIPSParser_OUTPUTS}
        ${FLEX_MIPSLexer_OUTPUTS})
endmacro()

add_helium_test(Test_Bench bench.c)

add_util_test(Test_BitArray bitarray.c)
add_util_test(Test_Vector vector.c)
add_util_test(Test_String str.c)
add_util_test(Test_List list.c)
add_util_test(Test_Pair pair.c)
add_util_test(Test_Table table.c)
add_util_test(Test_Stack stack.c)

# add_helium_test(Escaping escaping.c)
