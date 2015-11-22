set (CMOCKA "/Users/m4yers/Development/SDK/cmoka/cmocka-1.0.1" CACHE PATH
    "CMocka test framework path")

list (APPEND CMAKE_MODULE_PATH ${CMOCKA}/cmake/Modules)

add_subdirectory (${CMOCKA} ${CMAKE_BINARY_DIR}/cmocka)

include_directories (SYSTEM ${CMOCKA}/include)

function (add_cmocka_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} cmocka_shared)

    add_test(${target} ${target})

    #TODO move it somewhere else
    if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        add_custom_command(TARGET ${target}
                           POST_BUILD
                           COMMAND ${target}
                           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                           COMMENT "Running ${target}" VERBATIM)
    endif()
endfunction ()
