function(jaql_enable_doxygen)
    # DOXYGEN_EXECUTABLE is injected by the Conan toolchain when build_docs=True.
    # Run ./scripts/bootstrap.sh --docs to configure the docs build.
    if(NOT DOXYGEN_EXECUTABLE)
        message(FATAL_ERROR
            "DOXYGEN_EXECUTABLE is not set. "
            "Configure with: ./scripts/bootstrap.sh --docs"
        )
    endif()

    set(DOXYGEN_INPUT_DIR "${CMAKE_SOURCE_DIR}/include")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")

    find_program(_jaql_dot_executable NAMES dot)
    if(_jaql_dot_executable)
        set(DOXYGEN_HAVE_DOT "YES")
    else()
        set(DOXYGEN_HAVE_DOT "NO")
    endif()

    configure_file(
        "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
        "${CMAKE_BINARY_DIR}/Doxyfile"
        @ONLY
    )

    add_custom_target(doxygen
        COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endfunction()
