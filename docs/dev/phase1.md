# Phase 1 Execution Plan

This document defines the implementation plan for Phase 1: Core Utilities.
It is the working plan for delivering the foundational C++ primitives that all
financial domain code will rely on.

## Goal

Implement a complete, tested, and documented foundation layer: a structured logger in
`infra`, and in `core` a full error handling framework, a validated `StrongType` library,
a composable assertion system, and a correct date/calendar system.

## Scope Decisions

- Only `infra` and `core` are in scope. `math` and all domain modules are deferred to
  later phases.
- The `Date` type uses an `int` serial number (days since 1970-01-01) for arithmetic
  performance. Conversion to/from `std::chrono::sys_days` is exposed for interop.
- Phase 1 calendar support is limited to `WeekendCalendar`. Holiday calendars are
  deferred to Phase 3.
- Day count conventions (Act/360, 30/360, etc.) belong to the `instruments`/`curves`
  layer and are out of scope here.
- `Result<T>` continues to alias `tl::expected<T, Error>`. Migration to `std::expected`
  is deferred until the C++23 baseline is confirmed stable across all target compilers.
- Unit test coverage target is ≥ 95 % line coverage for every public header in `infra`
  and `core`.

## Deliverables

### Phase A — `infra`: Structured Logger

`infra` has no JAQL dependencies and can be implemented independently.

Public API surface (`include/jaql/infra/logger.hpp`):

- `get_logger(std::string_view name)` — return or create a named `spdlog::logger`.
- `configure_logging(level, sink)` — set the global level and output sink; call once at
  startup.

Implementation (`src/infra/logger.cpp`):

- spdlog registry initialisation.
- Lazy named-logger creation backed by the configured sink.

Build wiring:

- `src/infra/CMakeLists.txt`: add `SOURCES logger.cpp`; the module switches from
  `INTERFACE` to `STATIC`.
- `tests/unit/infra/CMakeLists.txt`: add `test_infra_logger` executable linked to
  `jaql::infra` and `GTest::gtest_main`.

Tests (`tests/unit/infra/test_logger.cpp`):

- `get_logger` called twice with the same name returns the same pointer.
- Level filtering suppresses messages below the configured level.
- `configure_logging` changes take effect on subsequently obtained loggers.

### Phase B — `core`: Error Type and Result\<T\> Utilities

Depends on Phase A being buildable (even as an INTERFACE stub).

Public API surface:

- `include/jaql/core/error.hpp` — `Code` enum (`Ok`, `InvalidArgument`, `OutOfRange`,
  `InvalidDate`, `NumericalFailure`, `NotFound`, `ParseError`, `InternalError`); `Error`
  class holding a `Code`, a `std::string` message, and a `std::source_location`; free
  function `to_string(Code)`.
- `include/jaql/core/result.hpp` — `Result<T>` alias for `tl::expected<T, Error>`; free
  functions `ok()`, `err<Code>()`, `result_cast<U>()`, `try_or()`, and
  `all_of(std::vector<Result<T>>)` (fail-fast: returns the first error encountered).

Implementation (`src/core/error.cpp`):

- `to_string()` switch table mapping every `Code` to a string literal.
- `Error::what()` producing a formatted string combining code, message, and source
  location.

Build wiring:

- `src/core/CMakeLists.txt`: add `SOURCES error.cpp` (further sources added in Phase D).
- `tests/unit/core/CMakeLists.txt`: add `test_core_error` and `test_core_result`.

Tests:

- `test_error.cpp`: every `Code` value has `to_string()` coverage; `what()` output
  contains code, message, file name, and line; source location is captured at the call
  site, not inside the `Error` constructor.
- `test_result.cpp`: success and error paths for each utility; `all_of` with empty input,
  all-ok input, first-element failure, and last-element failure.

### Phase C — `core`: StrongType and Assertion Framework

Parallel with Phase B. Depends only on `infra` being an INTERFACE target.

Public API surface:

- `include/jaql/core/strong_type.hpp` — `StrongType<Tag, T, Policies...>` with
  `using value_type = T`. CRTP policy mix-ins:
  - `Arithmetic` — binary `+`, `-`, `*`, `/` between two instances of the same type.
  - `Comparable` — full ordering via `operator<=>`.
  - `Hashable` — `std::hash` specialisation enabling use as an unordered map key.
  - `Formattable` — `std::formatter` specialisation enabling `std::format`.
  - `Scalable` — `operator*(value_type)` and `operator/(value_type)` for scalar scaling;
    derives the scalar type from `Derived::value_type`.
  - `Incrementable` — prefix and postfix `++`/`--`.
- `include/jaql/core/assert.hpp` — `ViolationInfo` struct (expression, message,
  `std::source_location`); `ViolationHandler` function-pointer typedef; `get_violation_handler()`
  and `set_violation_handler()`with inline-static storage; three built-in handlers:
  `abort_handler` (default, `[[noreturn]]`), `throwing_handler` (for test isolation),
  `log_handler` (delegates to `infra::get_logger`); macros `JAQL_ASSERT`, `JAQL_EXPECTS`,
  `JAQL_ENSURES`.

Build wiring:

- Both headers are implemented entirely in-header; no new `.cpp` files required.
- `tests/unit/core/CMakeLists.txt`: add `test_core_strong_type` and `test_core_assert`.

Tests:

- `test_strong_type.cpp`: explicit-only construction; each policy exercised independently;
  `Rate * double` via `Scalable`; `std::unordered_map<Rate, int>` via `Hashable`;
  `std::format("{}", rate)` via `Formattable`; `static_assert` that a `Rate` cannot be
  constructed from a `Spread` (type-safety proof); verify that missing policies produce
  clear compilation errors.
- `test_assert.cpp`: passing condition is a no-op; failing condition invokes the handler;
  swapping to `throwing_handler` then calling `JAQL_ASSERT(false)` throws; `JAQL_EXPECTS`
  and `JAQL_ENSURES` follow the same pattern; a RAII guard restores the original handler
  after each test.

### Phase D — `core`: Date and Calendar System

Depends on Phase B (needs `Result<T>` and `Error`).

Public API surface (`include/jaql/core/date.hpp`):

- `DayOfWeek` enum (`Monday` through `Sunday`).
- `BusinessDayConvention` enum: `Unadjusted`, `Following`, `ModifiedFollowing`,
  `Preceding`, `ModifiedPreceding`.
- `Date` class — serial-based storage; factory `from_ymd(year, month, day)` returning
  `Result<Date>`; `from_serial(int)` noexcept; accessors `year()`, `month()`, `day()`,
  `serial()`, `day_of_week()`; arithmetic `add_days(int)`, `add_months(int)` (end-of-month
  clamping), `add_years(int)`; `operator<=>` and `operator-(Date, Date) -> int`.
- `Calendar` abstract base class with `is_business_day(const Date&) -> bool`.
- `WeekendCalendar` — Saturday and Sunday are non-business days.
- Free functions `adjust()`, `add_business_days()`, `business_days_between()`.
- Utilities `is_leap_year(int)` and `days_in_month(month, year)` (public for testing).

Implementation (`src/core/date.cpp`):

- Proleptic Gregorian serial ↔ year/month/day conversion using a well-known arithmetic
  algorithm (no Julian calendar, no lookup tables).
- Leap year logic: divisible by 400, or divisible by 4 but not 100.
- End-of-month clamping in `add_months`: January 31 + 1 month → February 28 or 29.
- All five `BusinessDayConvention` implementations including month-boundary handling for
  `ModifiedFollowing` and `ModifiedPreceding`.

Build wiring:

- `src/core/CMakeLists.txt`: add `date.cpp` to SOURCES.
- `tests/unit/core/CMakeLists.txt`: add `test_core_date`.

Tests (`tests/unit/core/test_date.cpp`):

- Leap year rules: 2000 (leap), 1900 (not), 2024 (leap), 2023 (not).
- `days_in_month` for every month including leap and non-leap February.
- `from_ymd` with valid input, invalid month (13), invalid day (32), and February 30.
- `add_days`: positive, negative, crossing a month boundary, crossing a year boundary.
- `add_months`: normal case, end-of-month clamping (January 31 → February 28/29).
- `add_years`: leap-day source date (February 29 → February 28 on a non-leap target year).
- `day_of_week` for a set of known dates (at least one of each weekday).
- `WeekendCalendar::is_business_day` for all seven day values.
- All five `BusinessDayConvention` cases, including a date that lands on a weekend at
  month end (tests the Modified variants' month-boundary rollback).
- `add_business_days` with positive and negative counts crossing multiple weekends.
- `business_days_between` for various date ranges.
- `operator<=>`: less-than, equal, greater-than.
- `operator-`: positive and negative day differences.

### Phase E — Benchmarks

Depends on Phase B.

Files:

- `benchmarks/CMakeLists.txt` — new top-level directory; root `CMakeLists.txt` already
  gates on `JAQL_BUILD_BENCHMARKS`.
- `benchmarks/core/bench_result.cpp` — two benchmarks:
  - `BM_RawDouble`: a chain of five arithmetic operations on a `double`.
  - `BM_ResultChain`: the same chain expressed as five `.map()` calls on
    `Result<double>`, always in the success path.

The success criterion is that `BM_ResultChain` adds ≤ 2 ns overhead relative to
`BM_RawDouble` when built with the `benchmark` preset.

### Phase F — Doxygen Documentation Pass

After all headers in `infra` and `core` are finalised.

- Add `/// @brief`, `@param`, and `@return` Doxygen comments to every public API in
  `include/jaql/infra/` and `include/jaql/core/`.
- Confirm that `docs/Doxyfile.in` lists both directories under `INPUT`.

## Recommended Order

1. Phase A (`infra` logger) — unblocks everything; no JAQL dependencies.
2. Phase B (`Error`, `Result<T>`) and Phase C (`StrongType`, assertions) — run in
   parallel; each needs only the `infra` INTERFACE target.
3. Phase D (`Date`) — starts once Phase B is merged.
4. Phase E (benchmarks) — starts once Phase B is merged.
5. Phase F (Doxygen pass) — last, after all APIs are stable.

## Success Criteria

Phase 1 is complete when all of the following are true:

1. `cmake --build --preset jaql-debug` compiles all `infra` and `core` sources without
   warnings.
2. `./scripts/test.sh` reports all tests passing with line coverage ≥ 95 % across every
   public header in `infra` and `core`.
3. `./scripts/lint.sh` reports no clang-tidy violations.
4. `./scripts/check_headers.sh` confirms every new header is self-contained.
5. `StrongType` prevents mixing `Rate` and `Spread` at compile time; a misuse produces a
   readable error message pointing to the policy, not to internal templates.
6. Date arithmetic is correct for leap years, end-of-month edges, and all five business
   day convention cases at month boundaries.
7. The `benchmark` preset with `JAQL_BUILD_BENCHMARKS=ON` confirms that
   `BM_ResultChain` adds ≤ 2 ns overhead relative to `BM_RawDouble`.
8. All public APIs in `infra` and `core` carry Doxygen comments.

## Out of Scope

- `math` module and all domain/analytics modules
- DST handling, leap seconds, and timezone support
- Holiday calendars (deferred to Phase 3)
- Day count conventions (deferred to the `instruments`/`curves` layer)
- `std::expected` migration (deferred until C++23 baseline is confirmed on all targets)
- Windows CI gating
