# Roadmap

This document defines the phased development plan for JAQL. Each phase represents a
coherent engineering milestone that delivers standalone value and creates the foundation
for the next phase.

The plan is deliberately **bottom-up**: infrastructure and correctness come before
functionality, and functionality comes before optimization. No phase is skipped.

Detailed deliverables, tasks, and progress tracking are managed via
[GitHub Issues](https://github.com/jerarn/jaql/issues) and the
[JAQL Project Board](https://github.com/jerarn/jaql/projects).

---

## Phase 0 — Foundation Tooling and Architecture

**Status:** In Progress

Establish the engineering foundation that all future work depends on. A developer should
be able to clone the repository, install dependencies, build, and run tests in under
10 minutes on a supported platform.

---

## Phase 1 — Core Utilities

**Status:** Planned

Implement the foundational C++ utilities that all financial domain code will rely on:
a complete date/time system, a complete error handling framework, and a validated
`StrongType` library.

**Depends on:** Phase 0

---

## Phase 2 — Mathematical Foundations

**Status:** Planned

Build the numerical infrastructure that the pricing and simulation layers depend on:
linear algebra, interpolation, root-finding, numerical integration, and random number
generation.

**Depends on:** Phase 1

---

## Phase 3 — Market Primitives and Pricing Framework

**Status:** Planned

Implement the domain layer: instrument abstractions, market data framework, yield curve
bootstrapping, and the first working pricing engines (bond and vanilla interest rate
derivatives).

**Depends on:** Phases 1 and 2

---

## Phase 4 — Risk, Simulation, and Calibration

**Status:** Planned

Implement the analytics layer: finite difference Greeks, Monte Carlo simulation for
path-dependent products, and a general calibration framework.

**Depends on:** Phase 3

---

## Timeline

No fixed calendar dates are assigned. Phases are sequenced by readiness, not by calendar.
Each phase is complete when its Success Criteria are met, not when a deadline arrives.

The guiding principle: **ship correct code, not fast code.** A phase that takes longer
because of thorough testing is preferable to a phase rushed with known numerical
inaccuracies.
