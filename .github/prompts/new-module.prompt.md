---
agent: agent
description: Scaffold a complete new JAQL module — headers, sources, tests, and CMake wiring.
---

Scaffold a new JAQL module named **`${input:moduleName}`** in layer **`${input:layer}`**
(one of: `foundation`, `domain`, `analytics`).

Consult `docs/architecture/module-dependencies.md` to verify which modules this new
module may depend on, and refuse to wire a dependency that would violate the one-way
layer constraint.

---

## Files to create

### 1. Public header — `include/jaql/${input:moduleName}/${input:moduleName}.hpp`

```cpp
#pragma once

// [JAQL headers — alphabetical]
// [Third-party headers — alphabetical]
// [Standard library headers — alphabetical]

namespace jaql::${input:moduleName} {

// TODO: add public types and functions

}  // namespace jaql::${input:moduleName}
```

### 2. Source file — `src/${input:moduleName}/${input:moduleName}.cpp`

```cpp
#include "jaql/${input:moduleName}/${input:moduleName}.hpp"

// [Other JAQL headers — alphabetical]
// [Third-party headers — alphabetical]
// [Standard library headers — alphabetical]

namespace jaql::${input:moduleName} {

// TODO: implement

}  // namespace jaql::${input:moduleName}
```

### 3. Unit test — `tests/unit/${input:moduleName}/test_${input:moduleName}.cpp`

Follow the pattern in `docs/testing-strategy.md`:
- One `TEST` or `TEST_F` per logical behaviour.
- Naming: `TEST(ClassName, MethodName_Scenario_ExpectedOutcome)`.
- Test every `Result<T>` error path as explicitly as the success path.
- Never compare `double` with `==`; use `EXPECT_NEAR` or `JAQL_EXPECT_NEAR_REL`.

### 4. Module CMakeLists — `src/${input:moduleName}/CMakeLists.txt`

```cmake
add_library(jaql_${input:moduleName}
    ${input:moduleName}.cpp
)

add_library(jaql::${input:moduleName} ALIAS jaql_${input:moduleName})

target_include_directories(jaql_${input:moduleName}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link only the modules listed in docs/architecture/module-dependencies.md
# for the ${input:moduleName} module.
target_link_libraries(jaql_${input:moduleName}
    PUBLIC
        # TODO: add allowed dependencies
)

set_target_properties(jaql_${input:moduleName} PROPERTIES
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF
)
```

### 5. Test CMakeLists — `tests/unit/${input:moduleName}/CMakeLists.txt`

```cmake
add_executable(test_${input:moduleName}
    test_${input:moduleName}.cpp
)

target_link_libraries(test_${input:moduleName}
    PRIVATE
        jaql::${input:moduleName}
        GTest::gtest_main
        GTest::gmock
)

gtest_discover_tests(test_${input:moduleName})
```

---

## Wiring into the parent build

After creating the above files, remind the user to:

1. Add `add_subdirectory(${input:moduleName})` to `src/CMakeLists.txt`.
2. Add `add_subdirectory(${input:moduleName})` to `tests/unit/CMakeLists.txt`.
3. Add the new `jaql::${input:moduleName}` target to the install targets in the root
   `CMakeLists.txt` if this module should be part of the public install set.

---

## Checklist before finishing

- [ ] No naming violations (see `docs/coding-standards.md` Naming Conventions table).
- [ ] No dependency violations (see `docs/architecture/module-dependencies.md`).
- [ ] All `Result<T>`-returning functions are `[[nodiscard]]`.
- [ ] Headers use `#pragma once`, `.hpp` extension, correct include order.
- [ ] Error paths are tested in the unit test file.
