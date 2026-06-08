---
name: jaql-new-module
description: Scaffold a complete new JAQL module with headers, sources, tests, and CMake wiring. Use when the user asks to scaffold a module, create a new JAQL module, or add a module in foundation/domain/analytics layer.
disable-model-invocation: true
---

# JAQL New Module

Scaffold a new JAQL module using the module name and layer the user provides
(one of: `foundation`, `domain`, `analytics`).

## Before Creating

Read [docs/architecture/module-dependencies.md](docs/architecture/module-dependencies.md) to verify which modules the new module may depend on. Refuse to wire a dependency that would violate the one-way layer constraint.

## Files to Create

### 1. Public header — `include/jaql/<moduleName>/<moduleName>.hpp`

```cpp
#pragma once

// [JAQL headers — alphabetical]
// [Third-party headers — alphabetical]
// [Standard library headers — alphabetical]

namespace jaql::<moduleName> {

// TODO: add public types and functions

}  // namespace jaql::<moduleName>
```

### 2. Source file — `src/<moduleName>/<moduleName>.cpp`

```cpp
#include "jaql/<moduleName>/<moduleName>.hpp"

// [Other JAQL headers — alphabetical]
// [Third-party headers — alphabetical]
// [Standard library headers — alphabetical]

namespace jaql::<moduleName> {

// TODO: implement

}  // namespace jaql::<moduleName>
```

### 3. Unit test — `tests/unit/<moduleName>/test_<moduleName>.cpp`

Follow [docs/testing-strategy.md](docs/testing-strategy.md):
- One `TEST` or `TEST_F` per logical behaviour.
- Naming: `TEST(ClassName, MethodName_Scenario_ExpectedOutcome)`.
- Test every `Result<T>` error path as explicitly as the success path.
- Never compare `double` with `==`; use `EXPECT_NEAR` or `JAQL_EXPECT_NEAR_REL`.

### 4. Module CMakeLists — `src/<moduleName>/CMakeLists.txt`

```cmake
add_library(jaql_<moduleName>
    <moduleName>.cpp
)

add_library(jaql::<moduleName> ALIAS jaql_<moduleName>)

target_include_directories(jaql_<moduleName>
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(jaql_<moduleName>
    PUBLIC
        # TODO: add allowed dependencies from module-dependencies.md
)

set_target_properties(jaql_<moduleName> PROPERTIES
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF
)
```

### 5. Test CMakeLists — `tests/unit/<moduleName>/CMakeLists.txt`

```cmake
add_executable(test_<moduleName>
    test_<moduleName>.cpp
)

target_link_libraries(test_<moduleName>
    PRIVATE
        jaql::<moduleName>
        GTest::gtest_main
        GTest::gmock
)

gtest_discover_tests(test_<moduleName>)
```

## Wiring into the Parent Build

After creating the files:

1. Add `add_subdirectory(<moduleName>)` to `src/CMakeLists.txt`.
2. Add `add_subdirectory(<moduleName>)` to `tests/unit/CMakeLists.txt`.
3. Add the new `jaql::<moduleName>` target to install targets in root `CMakeLists.txt` if public.

## Checklist Before Finishing

- [ ] No naming violations (see `docs/coding-standards.md`).
- [ ] No dependency violations (see `docs/architecture/module-dependencies.md`).
- [ ] All `Result<T>`-returning functions are `[[nodiscard]]`.
- [ ] Headers use `#pragma once`, `.hpp` extension, correct include order.
- [ ] Error paths are tested in the unit test file.
- [ ] Run `./scripts/test.sh` to validate the scaffold builds and tests pass.
