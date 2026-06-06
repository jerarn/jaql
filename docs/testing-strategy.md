# Testing Strategy

This document defines JAQL's approach to testing: what to test, how to test it, and
how to validate numerical correctness in a quantitative finance codebase where "correct"
often means "within 0.01 bps of a known reference".

---

## Test Categories

JAQL tests are organised into four categories with distinct purposes, directory
locations, and execution frequencies:

| Category        | Directory              | Purpose                                        | Run Frequency       |
|-----------------|------------------------|------------------------------------------------|---------------------|
| Unit            | `tests/unit/`          | Verify a single function or class in isolation | Every commit        |
| Integration     | `tests/integration/`   | Verify correct interaction between modules     | Every commit        |
| Regression      | `tests/regression/`    | Prevent silent changes in numerical output     | Every commit        |
| Performance     | `benchmarks/`          | Measure and guard against performance regressions | On demand |

---

## Unit Tests

### Principles

Unit tests verify a single unit of behaviour in complete isolation. Dependencies are
either test doubles (via Google Mock) or simple in-memory stubs. A unit test must not
touch the file system, network, or database.

Each test file maps to one public header:

```
include/jaql/core/strong_type.hpp  →  tests/unit/core/test_strong_type.cpp
include/jaql/math/numeric.hpp      →  tests/unit/math/test_numeric.cpp
```

### Structure with GoogleTest

```cpp
#include "jaql/core/strong_type.hpp"
#include <gtest/gtest.h>

namespace jaql::core::test {

// Fixture for shared setup (use when tests share state)
class StrongTypeTest : public ::testing::Test {
protected:
    using TestRate = StrongType<struct TestRateTag, double>;
};

TEST_F(StrongTypeTest, ExplicitConstructionSucceeds) {
    TestRate rate{0.05};
    EXPECT_DOUBLE_EQ(rate.value(), 0.05);
}

TEST_F(StrongTypeTest, ImplicitConstructionFromDoubleIsIllformed) {
    // Verify at compile time — this test documents the constraint
    static_assert(!std::is_convertible_v<double, TestRate>,
        "StrongType must not be implicitly constructible from underlying type");
}

TEST_F(StrongTypeTest, MoveSemantics) {
    TestRate original{0.05};
    TestRate moved = std::move(original);
    EXPECT_DOUBLE_EQ(moved.value(), 0.05);
}

} // namespace jaql::core::test
```

### Naming Convention

```
TEST(ClassName, MethodName_Scenario_ExpectedOutcome)
TEST(ApproxEqual, BothZero_ReturnsTrue)
TEST(ApproxEqual, RelativeDifferenceBelowTolerance_ReturnsTrue)
TEST(ApproxEqual, SubnormalValues_DoesNotReturnNaN)
```

### What to Test

- All public functions with at least one test per logical branch.
- Edge cases: empty inputs, boundary values, negative values where domain permits.
- Type constraints: use `static_assert` to document and verify compile-time invariants.
- Error cases: `Result<T>` error paths are tested as explicitly as success paths.

```cpp
// Test the error path — it is as important as the success path
TEST(DiscountFactor, NegativeTimeReturnsError) {
    auto result = discount_factor(YearFraction{-1.0}, Rate{0.05});
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, Error::Code::InvalidArgument);
}
```

---

## Integration Tests

Integration tests verify that modules work correctly together. They are allowed to
use multiple JAQL modules and more complex setup.

### Example: Curve + Pricing Integration

```cpp
// tests/integration/test_bond_pricing_pipeline.cpp
TEST(BondPricingPipeline, FairValueAtIssuanceIsZero) {
    // Arrange: build a flat 5% curve
    auto curve = FlatYieldCurve::build(Rate{0.05}, DayCountConvention::Actual365Fixed);
    ASSERT_TRUE(curve.has_value());

    // Build a 5-year bond with 5% annual coupon
    auto bond = FixedRateBond::build({
        .notional    = Notional{1'000'000.0},
        .coupon_rate = Rate{0.05},
        .maturity    = CalendarDate{2031, 5, 16},
        .convention  = DayCountConvention::Actual365Fixed
    });
    ASSERT_TRUE(bond.has_value());

    // Price
    DCFEngine engine;
    auto result = engine.price(*bond, **curve);
    ASSERT_TRUE(result.has_value());

    // Assert: at-market bond prices at par
    EXPECT_NEAR(result->pv / 1'000'000.0, 1.0, 1e-6);
}
```

---

## Regression Tests

Regression tests protect against silent numerical drift. They store golden reference
values (computed against a trusted external source — QuantLib, Bloomberg, or a
hand-calculated analytical result) and fail if the output changes.

### Golden Value Convention

Reference values are stored as `constexpr double` in the test file with a comment
documenting the source:

```cpp
// Reference value computed via QuantLib 1.34, FlatForward(5%, Actual365Fixed)
// Bond: 5Y maturity, 5% annual coupon, 1M notional, priced 2026-05-16
// QuantLib result: 1,000,000.00 (at par)
constexpr double bond_par_value_reference = 1'000'000.00;
constexpr double bond_par_value_tolerance = 0.01;  // 1 cent tolerance

TEST(BondRegression, ParBondValue) {
    // ... setup ...
    EXPECT_NEAR(result->pv, bond_par_value_reference, bond_par_value_tolerance);
}
```

### Managing Golden Files

For larger regression datasets (e.g., a full curve of 20 tenors), golden values are
stored in JSON files under `tests/regression/data/`:

```
tests/regression/data/eur_swap_curve_20250516.json
tests/regression/data/vanilla_option_surface_20250516.json
```

The regression test loads the JSON file and compares to computed values with defined
tolerances. These files are committed to the repository and updated only intentionally
(with a PR that explains the change).

---

## Floating-Point Precision Testing

Numerical correctness testing in quantitative finance requires careful handling of
floating-point comparison. **Never use `==` to compare doubles.** Use the appropriate
comparison strategy for the domain:

### Absolute Tolerance (`EXPECT_NEAR`)

Use when you know the expected absolute precision of the result:

```cpp
// DV01 should be within 0.01 bps = $1 on a $1M notional
EXPECT_NEAR(computed_dv01, reference_dv01, 1.0);
```

### Relative Tolerance (`JAQL_EXPECT_NEAR_REL`)

Use when the scale of the result varies (e.g., discount factors range from ~0.1 to ~1.0):

```cpp
// Defined in tests/helpers/numeric_approx.hpp
#define JAQL_EXPECT_NEAR_REL(actual, expected, rtol) \
    EXPECT_NEAR((actual), (expected), std::abs((expected)) * (rtol))

// Usage: 0.01% relative tolerance
JAQL_EXPECT_NEAR_REL(computed_df, reference_df, 1e-4);
```

### Tolerance Guidelines by Domain

| Quantity                  | Typical Tolerance  | Unit                  |
|---------------------------|--------------------|-----------------------|
| Discount factor           | 1e-8 relative      | —                     |
| Zero rate                 | 0.01 bps           | = 1e-6 absolute       |
| Bond PV                   | $0.01 on $1M notional | = 1e-5 relative    |
| Option vega               | 0.001 vega points  |                       |
| Monte Carlo PV (N=100k)   | 1e-3 relative      | (√N convergence)      |

### Special Floating-Point Cases

All numerical functions must be tested for:
- **Inputs near zero** (subnormal values, signed zero)
- **Very large inputs** (near `std::numeric_limits<double>::max()`)
- **Inputs that produce exact mathematical zero** (e.g., interpolation at a known node)
- **NaN and Inf propagation** (functions receiving NaN must return a `Result` error, not
  silently propagate NaN)

```cpp
TEST(ApproxEqual, NaNInputsReturnsFalse) {
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(approx_equal(nan, nan));    // NaN != NaN, per IEEE 754
    EXPECT_FALSE(approx_equal(nan, 0.0));
}
```

---

## Benchmarks

### When to Benchmark

Write a benchmark when:
- A function is in a hot path (called thousands of times per pricing run).
- An optimisation has been applied and needs to be verified against the baseline.
- A regression in performance needs to be detected.

### Google Benchmark Structure

```cpp
#include "jaql/math/numeric.hpp"
#include <benchmark/benchmark.h>

static void BM_approx_equal(benchmark::State& state) {
    double a = 0.123456789;
    double b = 0.123456790;
    for (auto _ : state) {
        benchmark::DoNotOptimize(jaql::math::approx_equal(a, b, 1e-9));
    }
}
BENCHMARK(BM_approx_equal);

BENCHMARK_MAIN();
```

### Benchmark Presets

Run benchmarks using the `benchmark` preset to ensure release-mode compilation with LTO:

```bash
cmake --preset benchmark
cmake --build --preset benchmark
./build/benchmark/benchmarks/math/bench_math
```

### Performance Regression Policy

Benchmark results are not stored in CI by default (they are too hardware-sensitive).
Instead:
- The `benchmark` preset must build without errors.
- Manually run benchmarks before and after a performance-relevant PR.
- Document the measured impact in the PR description.
- If a benchmark regresses by more than 10%, the change requires explicit justification.

---

## CI Test Matrix

The full test suite runs on every push to every branch:

| Platform       | Compiler   | Build Type | Sanitizers | Coverage |
|----------------|------------|------------|------------|----------|
| ubuntu-24.04   | GCC 13     | Debug      | —          | ✓        |
| ubuntu-24.04   | GCC 13     | Release    | —          | —        |
| ubuntu-24.04   | Clang 17   | Debug      | —          | —        |
| ubuntu-24.04   | Clang 17   | Release    | —          | —        |

Coverage is reported for the `ubuntu-24.04 / GCC 13 / Debug` configuration only.
The coverage target is ≥80% line coverage for all `core` and `math` public APIs.

---

## Determinism and Reproducibility

### Random Tests

Any test that uses random numbers must seed the RNG with a fixed seed and document it:

```cpp
TEST(MonteCarlo, GeometricBrownianMotionMeanConverges) {
    constexpr std::uint64_t seed = 42;   // fixed seed for reproducibility
    MersenneTwister rng{seed};
    // ...
}
```

### Non-Determinism Budget

The following sources of non-determinism are acceptable:
- Timing-based tests (not used in JAQL — timing is for benchmarks only).
- Platform-specific floating-point rounding (document tolerance accordingly).

The following are not acceptable:
- Unseeded RNG in tests.
- Tests that depend on container ordering without sorting first.
- Tests that pass on one platform and fail on another without documented justification.
