# cmake/Sanitizers.cmake
#
# Provides jaql_apply_sanitizers(TARGET).
# Called automatically by jaql_add_module() for all compiled (non-INTERFACE) targets.
#
# Controlled by project-level options (set via CMakePresets.json or -D on the command line):
#   JAQL_ENABLE_ASAN     — AddressSanitizer         (incompatible with TSan)
#   JAQL_ENABLE_UBSAN    — UndefinedBehaviorSanitizer
#   JAQL_ENABLE_TSAN     — ThreadSanitizer           (incompatible with ASan)
#   JAQL_ENABLE_COVERAGE — gcov/lcov instrumentation

function(jaql_apply_sanitizers TARGET)
    # ── Mutual exclusion guard ────────────────────────────────────────────────
    if(JAQL_ENABLE_ASAN AND JAQL_ENABLE_TSAN)
        message(FATAL_ERROR
            "JAQL_ENABLE_ASAN and JAQL_ENABLE_TSAN are mutually exclusive. "
            "Use the 'asan' or 'tsan' preset, not both simultaneously."
        )
    endif()

    # ── Build the sanitizer list ──────────────────────────────────────────────
    # Combining multiple sanitizers into one -fsanitize= flag is required —
    # using separate flags causes linker errors with some toolchains.
    set(_sanitizers "")
    if(JAQL_ENABLE_ASAN)
        list(APPEND _sanitizers "address")
    endif()
    if(JAQL_ENABLE_UBSAN)
        list(APPEND _sanitizers "undefined")
    endif()
    if(JAQL_ENABLE_TSAN)
        list(APPEND _sanitizers "thread")
    endif()

    if(_sanitizers)
        if(MSVC)
            # MSVC supports only ASan natively (/fsanitize=address).
            if(JAQL_ENABLE_ASAN)
                target_compile_options(${TARGET} PRIVATE /fsanitize=address)
            else()
                message(WARNING
                    "UBSan/TSan are not natively supported on MSVC; "
                    "sanitizer flags for target '${TARGET}' ignored."
                )
            endif()
        else()
            list(JOIN _sanitizers "," _san_flags)
            target_compile_options(${TARGET} PRIVATE
                -fsanitize=${_san_flags}
                -fno-omit-frame-pointer
            )
            # Sanitizer runtime must also be linked into every binary.
            target_link_options(${TARGET} PUBLIC -fsanitize=${_san_flags})
        endif()
    endif()

    # ── Coverage instrumentation (GCC / Clang only) ───────────────────────────
    if(JAQL_ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${TARGET} PRIVATE --coverage)
            target_link_options(${TARGET} PUBLIC --coverage)
        else()
            message(WARNING
                "JAQL_ENABLE_COVERAGE is only supported with GCC and Clang; "
                "ignoring for target '${TARGET}'."
            )
        endif()
    endif()
endfunction()
