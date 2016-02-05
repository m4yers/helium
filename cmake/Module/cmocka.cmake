function (add_cmocka_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} /usr/local/lib/libcmocka.a)

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
