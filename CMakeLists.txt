cmake_minimum_required(VERSION 3.25)

project(
    jaql
    VERSION 0.1.0
    DESCRIPTION "Modular C++23 quantitative finance library"
    LANGUAGES CXX
)

# ── Language standard ─────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)        # Strict ISO C++ — no GNU/MSVC extensions

# Generate compile_commands.json for clangd / clang-tidy.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ───────────────────────────────────────────────────────────────────
option(JAQL_BUILD_TESTS        "Build unit and integration tests"          ON)
option(JAQL_BUILD_BENCHMARKS   "Build benchmarks"                          OFF)
option(JAQL_BUILD_EXAMPLES     "Build examples"                            OFF)
option(JAQL_ENABLE_ASAN        "Enable AddressSanitizer"                   OFF)
option(JAQL_ENABLE_UBSAN       "Enable UndefinedBehaviorSanitizer"         OFF)
option(JAQL_ENABLE_TSAN        "Enable ThreadSanitizer"                    OFF)
option(JAQL_ENABLE_COVERAGE    "Enable gcov/lcov coverage instrumentation" OFF)
option(JAQL_ENABLE_CLANG_TIDY  "Run clang-tidy during the build"           OFF)
option(JAQL_WARNINGS_AS_ERRORS "Treat all warnings as errors (-Werror)"    OFF)

# ── CMake helpers ─────────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GNUInstallDirs)
include(JaqlModule)
include(CompilerWarnings)
include(Sanitizers)

# ── clang-tidy integration ────────────────────────────────────────────────────
if(JAQL_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-17 clang-tidy)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found; JAQL_ENABLE_CLANG_TIDY has no effect")
    endif()
endif()

# ── Dependencies (provided by Conan 2 CMakeDeps generator) ───────────────────
find_package(tl-expected   REQUIRED)
find_package(spdlog        REQUIRED)
find_package(Eigen3        REQUIRED)
find_package(nlohmann_json REQUIRED)

if(JAQL_BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    include(GoogleTest)
endif()

if(JAQL_BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)
endif()

# ── Source tree ───────────────────────────────────────────────────────────────
# src/CMakeLists.txt is created in Phase 4 (module scaffolding).
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt")
    add_subdirectory(src)
else()
    message(STATUS "jaql: src/CMakeLists.txt not found — skipping module targets (run Phase 4)")
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
if(JAQL_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

# ── Benchmarks ────────────────────────────────────────────────────────────────
if(JAQL_BUILD_BENCHMARKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/CMakeLists.txt")
    add_subdirectory(benchmarks)
endif()

# ── Examples ──────────────────────────────────────────────────────────────────
if(JAQL_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# ── Coverage report target ────────────────────────────────────────────────────
if(JAQL_ENABLE_COVERAGE)
    find_program(LCOV_EXE    lcov)
    find_program(GENHTML_EXE genhtml)
    if(LCOV_EXE AND GENHTML_EXE)
        add_custom_target(coverage
            COMMAND ${LCOV_EXE}
                    --capture
                    --directory .
                    --output-file coverage.info
                    --exclude "*/build/*"
                    --exclude "*/tests/*"
                    --exclude "/usr/*"
                    --ignore-errors unused
            COMMAND ${GENHTML_EXE}
                    coverage.info
                    --output-directory coverage/
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Generating HTML coverage report → ${CMAKE_BINARY_DIR}/coverage/index.html"
            VERBATIM
        )
    else()
        message(WARNING "lcov/genhtml not found; 'coverage' target unavailable")
    endif()
endif()

# ── Install export ────────────────────────────────────────────────────────────
# Guard required: install(EXPORT) fails if no targets have been registered yet.
# The export is populated by jaql_add_module() once src/ is scaffolded (Phase 4).
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt")
    install(
        EXPORT      jaqlTargets
        FILE        jaqlTargets.cmake
        NAMESPACE   jaql::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jaql"
    )
endif()
