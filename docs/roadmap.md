# Roadmap

This document defines the phased development plan for JAQL. Each phase represents a
coherent engineering milestone that delivers standalone value and creates the foundation
for the next phase.

The plan is deliberately **bottom-up**: infrastructure and correctness come before
functionality, and functionality comes before optimization. No phase is skipped.

---

## Phase 0 — Foundation Tooling and Architecture

**Status:** In Progress

### Objective

Establish the engineering foundation that all future work depends on. A developer should
be able to clone the repository, install dependencies, build, and run tests in under
10 minutes on a supported platform.

### Rationale

A platform that is painful to build or contribute to accumulates technical debt rapidly.
Investing in tooling and CI before any domain code ensures that every subsequent phase
is built on a stable, verifiable foundation.

### Deliverables

- [ ] CMake 3.25+ build system with `CMakePresets.json` (dev-debug, dev-release, ci-linux,
  asan, ubsan, tsan, benchmark)
- [ ] Conan 2 dependency management (`conanfile.py`, bootstrap script)
- [ ] `cmake/JaqlModule.cmake` helper for consistent module target creation
- [ ] Compiler warning configuration (GCC 13, Clang 17, MSVC 2022)
- [ ] Sanitizer integration (ASan, UBSan, TSan, LSan)
- [ ] `clang-format` and `clang-tidy` configuration files
- [ ] GitHub Actions CI workflow: Linux matrix (GCC 13, Clang 17) × (Debug, Release)
- [ ] GitHub Actions release workflow (triggered on semver tags)
- [ ] GitHub Actions docs workflow (Doxygen → GitHub Pages)
- [ ] Issue templates, PR template, CODEOWNERS
- [ ] Developer scripts: `bootstrap.sh`, `format.sh`, `lint.sh`, `check_headers.sh`
- [ ] VS Code workspace configuration (settings, tasks, extensions)
- [ ] Full documentation suite (this roadmap and companion documents)
- [ ] Module directory scaffolding for all 10 modules (CMakeLists, placeholder headers)

### Success Criteria

- `cmake --preset dev-debug && cmake --build --preset dev-debug` produces 0 errors and
  0 warnings on GCC 13 and Clang 17.
- `ctest --preset dev-debug` passes all tests (version, strong types, numeric utilities).
- GitHub Actions CI workflow is green on every push.
- `scripts/format.sh && git diff --exit-code` reports no diff.

### Risks

- CMake preset compatibility across IDE versions (CLion, VS Code CMake Tools).
  *Mitigation: test on both IDEs, document any known quirks.*
- Conan 2 profile detection on CI runners.
  *Mitigation: `conan profile detect` step in bootstrap script with fallback defaults.*

### Dependencies

None — this phase has no JAQL prerequisites.

---

## Phase 1 — Core Utilities

**Status:** Planned

### Objective

Implement the foundational C++ utilities that all financial domain code will rely on:
a complete date/time system, a complete error handling framework, and a validated
`StrongType` library.

### Rationale

Every module above `infra` needs dates, errors, and unit-safe types. Getting these right
before any domain code is written prevents a class of bugs that are expensive to fix
retroactively in a large codebase.

### Deliverables

- [ ] **Date/calendar system** in `core`:
  - `CalendarDate` (year, month, day) with Julian day number backing
  - `BusinessDayConvention` enum (Following, ModifiedFollowing, Preceding, Unadjusted)
  - Holiday calendar abstraction (`Calendar` concept) with a `WeekendCalendar` reference
    implementation
  - `Period` type (e.g., `3M`, `2Y`) with arithmetic on dates
- [ ] **Complete `StrongType<Tag, T>`** implementation with opt-in operator policies
  (arithmetic, comparison, hashing, formatting)
- [ ] **`Result<T>` utilities**: `result_cast`, `try_or`, `all_of` combinator for
  aggregating multiple `Result<T>` values
- [ ] **`Error` type** with a full `Code` enumeration and structured context information
- [ ] **Assertion framework**: `JAQL_ASSERT`, `JAQL_EXPECTS`, `JAQL_ENSURES` with
  configurable violation handlers (abort, throw, log)
- [ ] **Logger** in `infra`: structured logging with `spdlog` backing, named loggers per
  module, configurable sink/level
- [ ] Unit tests achieving ≥95% coverage of all `core` and `infra` public APIs
- [ ] Documentation: public API Doxygen comments for all `core` and `infra` headers

### Success Criteria

- Date arithmetic is correct for leap years, month-end, and business day adjustment.
- `StrongType` compilation errors are readable and point to the misuse clearly.
- `Result<T>` propagation in a chain of 5 operations adds ≤2 ns overhead vs raw double
  (benchmarked in the `benchmark` preset).

### Risks

- Date/calendar systems are deceptively complex (DST, leap seconds, jurisdiction-specific
  holidays). *Mitigation: start with `WeekendCalendar` only; custom holiday calendars
  are Phase 3.*
- `std::expected` vs `tl::expected` divergence. *Mitigation: wrapper typedef; migrate
  at C++23 baseline.*

### Dependencies

Phase 0 complete.

---

## Phase 2 — Mathematical Foundations

**Status:** Planned

### Objective

Build the numerical infrastructure that the pricing and simulation layers depend on:
linear algebra, interpolation, root-finding, numerical integration, and random number
generation.

### Rationale

Pricing models are mathematical. Without validated, reliable numerical primitives, every
pricing engine must reimplement the same computations, leading to duplication, divergence,
and bugs. Phase 2 creates the shared numerical vocabulary of the library.

### Deliverables

- [ ] **Linear algebra abstraction** wrapping Eigen 3.4:
  - Type aliases: `Vector<N>`, `Matrix<M,N>`, `DynamicVector`, `DynamicMatrix`
  - Fixed-size wrappers for common quant dimensions (1D, 2D, covariance matrices)
  - No raw Eigen types in public APIs — all wrapped in `jaql::math` types
- [ ] **Interpolation framework**:
  - `Interpolator` concept
  - `LinearInterpolator<X, Y>`, `LogLinearInterpolator<X, Y>`, `CubicSplineInterpolator`
  - Extrapolation policies: `FlatExtrapolation`, `LinearExtrapolation`, `ErrorExtrapolation`
- [ ] **1D root-finding**: Brent's method, Newton-Raphson with analytical or numerical
  derivative, bisection — all returning `Result<double>` with convergence diagnostics
- [ ] **Numerical integration**: Gauss-Legendre quadrature for 1D integrals
- [ ] **Statistical utilities**: sample mean, variance, covariance, Cholesky decomposition
  for correlation matrices
- [ ] **Random number generation (RNG) framework**:
  - `UniformRNG` concept
  - `MersenneTwister` adapter over `std::mt19937_64`
  - `NormalRNG<Engine>`: Box-Muller and inverse CDF transforms
  - Seed management utilities for reproducibility
- [ ] **Normal distribution**: CDF, inverse CDF (Abramowitz and Stegun), PDF (Black-Scholes
  building block)
- [ ] Benchmarks for all numerically critical paths (interpolation, root-finding, RNG)
- [ ] Numerical validation tests: compare against known analytical results and
  QuantLib/Excel reference values

### Success Criteria

- Interpolation of a 10-tenor curve evaluates in ≤100 ns on a release build.
- Root-finding converges on standard test cases (Black-Scholes implied vol inversion)
  in ≤20 iterations.
- `NormalRNG` passes Kolmogorov-Smirnov test at 5% significance level.

### Risks

- Eigen version pinning: Eigen releases occasionally introduce subtle numerical changes.
  *Mitigation: pin exact version in `conanfile.py`; golden baseline tests catch drift.*
- Interpolation edge cases at boundaries are a known source of production bugs.
  *Mitigation: explicit `ErrorExtrapolation` as default — fail loudly rather than silently.*

### Dependencies

Phase 1 complete (`core` date types, `StrongType`, `Result<T>`).

---

## Phase 3 — Market Primitives and Pricing Framework

**Status:** Planned

### Objective

Implement the domain layer: instrument abstractions, market data framework, yield curve
bootstrapping, and the first working pricing engines (bond and vanilla interest rate
derivatives).

### Rationale

Phase 3 delivers the first end-to-end pricing capability. A bond can be valued from
market rates. This validates the entire architecture from market data ingestion to
pricing result output and creates the reference implementation for future instrument types.

### Deliverables

- [ ] **`market` module**:
  - In-memory `MarketSnapshot`: stores quotes keyed by instrument identifiers
  - `Quote<T>` with timestamp, source, and value
  - Market data feed concept for live data integration (future)
- [ ] **`curves` module**:
  - `DiscountCurve` backed by `LogLinearInterpolator` on discount factors
  - `ZeroRateCurve`: zero rates with `Actual/365` and `Actual/360` day counts
  - Bootstrap algorithm for par swap rates → zero rates (iterative, using Phase 2
    root-finder)
  - Tenor point management: add, remove, shift, bump for sensitivity analysis
- [ ] **`instruments` module**:
  - `FixedRateBond`: fixed coupon, bullet maturity, standard day counts
  - `FloatingRateLeg`: LIBOR-style coupon schedule (forward rates from curve)
  - `IRS`: interest rate swap (pay-fixed / receive-float or vice versa)
  - Cashflow generation and schedule building
- [ ] **`pricing` module**:
  - `DCFEngine`: discounted cashflow pricing engine (bonds, fixed legs)
  - `SwapEngine`: IRS pricing engine (net PV of two legs)
  - `PricingResult` with present value, accrued interest, clean price
- [ ] End-to-end integration test: bootstrap a EUR swap curve from deposit and swap rates,
  price a 5-year IRS, verify PV = 0 at fair market rate
- [ ] Regression test suite with QuantLib reference values for all instrument types

### Success Criteria

- Bond pricing matches QuantLib to within 0.01 bps on a set of 20 reference bonds.
- IRS pricing matches QuantLib to within 0.01 bps on a set of 10 reference swaps.
- Full bootstrap + pricing pipeline completes in ≤5 ms for a 20-tenor curve.

### Risks

- Day count conventions have many subtleties (EOM adjustment, 30/360 variants).
  *Mitigation: validate all day count implementations against QuantLib test suite.*
- Bootstrap convergence: near-flat or inverted curves may cause convergence issues.
  *Mitigation: bounded iteration with `Result` failure on non-convergence.*

### Dependencies

Phases 1 and 2 complete.

---

## Phase 4 — Risk, Simulation, and Calibration

**Status:** Planned

### Objective

Implement the analytics layer: finite difference Greeks, Monte Carlo simulation for
path-dependent products, and a general calibration framework.

### Rationale

Risk management is the primary use case for most institutional quant platforms. Phase 4
delivers the tools needed to compute sensitivities (delta, gamma, vega, DV01) and
simulate risk factor scenarios.

### Deliverables

- [ ] **Finite difference Greeks** in `risk`:
  - Bump-and-reprice framework: parallel bumping of all curve nodes
  - Delta (DV01), gamma, vega surface — returned as a structured `GreeksResult`
  - Bucketed PV01 with configurable tenor grid
- [ ] **Monte Carlo framework** in `simulation`:
  - `PathGenerator<RNG, Process>` concept-based path simulation
  - Geometric Brownian Motion process
  - Euler-Maruyama and Milstein discretization schemes
  - Variance reduction: antithetic variates, control variates
  - Path storage: full path, final value only, configurable memory/speed tradeoff
- [ ] **Option pricing engines** in `pricing`:
  - `BlackScholesEngine`: analytical pricing for European vanilla options
  - `MCEngine<Process, RNG>`: Monte Carlo pricing engine for path-dependent products
- [ ] **Calibration framework** in a new `calibration` module:
  - `Calibrator` concept: minimizes `||model_prices - market_prices||²`
  - Levenberg-Marquardt optimizer (via Eigen)
  - Black-Scholes implied vol solver (Newton-Raphson using Phase 2 root-finder)
  - Swaption volatility surface bootstrapping
- [ ] **Scenario analysis** in `risk`:
  - `ScenarioSet` builder: parallel shift, steepening, twist, historical, custom
  - Scenario PnL attribution
- [ ] Benchmarks: full Monte Carlo pricing run (100,000 paths, 252 steps) ≤2s
- [ ] GPU extension point documentation: algorithm interfaces designed for future
  CUDA/OpenCL backend substitution

### Success Criteria

- Implied vol inversion converges in ≤20 iterations for all standard option types.
- DV01 via bump-and-reprice matches analytical DV01 to within 0.1% for standard swaps.
- MC pricing of a path-dependent Asian option converges with standard 1/√N rate.

### Risks

- Monte Carlo variance requires careful numerical design.
  *Mitigation: systematic benchmarking of variance reduction techniques.*
- Calibration convergence is highly problem-dependent.
  *Mitigation: well-defined stopping criteria and convergence diagnostics in `Result`.*
- Threading the MC engine for performance requires careful task decomposition.
  *Mitigation: design path-level parallelism as opt-in via the `infra` task queue.*

### Dependencies

Phase 3 complete. `infra` task queue (for parallel MC) is a Phase 4 deliverable.

---

## Timeline

No fixed calendar dates are assigned. Phases are sequenced by readiness, not by calendar.
Each phase is complete when its Success Criteria are met, not when a deadline arrives.

The guiding principle: **ship correct code, not fast code.** A phase that takes longer
because of thorough testing is preferable to a phase rushed with known numerical
inaccuracies.
