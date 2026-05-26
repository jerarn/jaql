# cmake/JaqlModule.cmake
#
# Provides jaql_add_module() — the canonical way to define a JAQL library module.
#
# Usage:
#   jaql_add_module(
#       <name>
#       [SOURCES  <src1.cpp> <src2.cpp> ...]
#       [HEADERS  <hdr1.hpp> <hdr2.hpp> ...]
#       [DEPS     <jaql::dep1> <jaql::dep2> <ExtLib::Target> ...]
#   )
#
# Effects:
#   - Creates target  jaql_<name>  (STATIC when SOURCES provided, INTERFACE otherwise)
#   - Creates alias   jaql::<name> (use this in target_link_libraries throughout the project)
#   - Public include root: ${CMAKE_SOURCE_DIR}/include
#     (consumers include as  #include <jaql/<name>/header.hpp>)
#   - Private include roots (compiled targets only):
#       ${CMAKE_CURRENT_SOURCE_DIR}          — for sibling .cpp/.hpp in the same module dir
#       ${CMAKE_CURRENT_SOURCE_DIR}/detail   — for internal implementation headers
#   - Applies jaql_apply_warnings()   (compiled targets only)
#   - Applies jaql_apply_sanitizers() (compiled targets only)
#   - Registers install rules for the target and its public headers

function(jaql_add_module NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;HEADERS;DEPS" ${ARGN})

    if(ARG_SOURCES)
        # ── Compiled static library ───────────────────────────────────────────
        add_library(jaql_${NAME} STATIC ${ARG_SOURCES})

        set_target_properties(jaql_${NAME} PROPERTIES
            OUTPUT_NAME             "jaql_${NAME}"
            POSITION_INDEPENDENT_CODE ON
        )

        target_include_directories(jaql_${NAME}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/detail
        )

        if(ARG_DEPS)
            target_link_libraries(jaql_${NAME} PUBLIC ${ARG_DEPS})
        endif()

        jaql_apply_warnings(jaql_${NAME})
        jaql_apply_sanitizers(jaql_${NAME})

        install(
            TARGETS  jaql_${NAME}
            EXPORT   jaqlTargets
            ARCHIVE  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
    else()
        # ── Header-only / stub library ────────────────────────────────────────
        # Used during scaffolding phases before source files are added.
        # Switches to STATIC automatically once SOURCES are provided.
        add_library(jaql_${NAME} INTERFACE)

        target_include_directories(jaql_${NAME}
            INTERFACE
                $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
        )

        if(ARG_DEPS)
            target_link_libraries(jaql_${NAME} INTERFACE ${ARG_DEPS})
        endif()

        install(
            TARGETS jaql_${NAME}
            EXPORT  jaqlTargets
        )
    endif()

    # Alias must be declared after the underlying target exists.
    add_library(jaql::${NAME} ALIAS jaql_${NAME})

    # Install public headers for this module.
    install(
        DIRECTORY   "${CMAKE_SOURCE_DIR}/include/jaql/${NAME}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jaql/${NAME}"
    )
endfunction()
