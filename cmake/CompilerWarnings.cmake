# cmake/CompilerWarnings.cmake
#
# Provides jaql_apply_warnings(TARGET).
# Called automatically by jaql_add_module() for all compiled (non-INTERFACE) targets.
#
# The warning set is controlled by two project-level options:
#   JAQL_WARNINGS_AS_ERRORS   — appends -Werror / /WX

function(jaql_apply_warnings TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE
            /W4
            /w14640   # thread un-safe static member initialization
            /w14265   # class has virtual functions but destructor is not virtual
            /w14826   # conversion from signed to unsigned, sign-extended
            /w15038   # data member will be initialized after another data member
        )
        if(JAQL_WARNINGS_AS_ERRORS)
            target_compile_options(${TARGET} PRIVATE /WX)
        endif()
    else()
        # GCC and Clang
        target_compile_options(${TARGET} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )
        if(JAQL_WARNINGS_AS_ERRORS)
            target_compile_options(${TARGET} PRIVATE -Werror)
        endif()
    endif()
endfunction()
