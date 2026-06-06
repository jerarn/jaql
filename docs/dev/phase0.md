# Phase 0 Execution Plan

This document defines the implementation plan for Phase 0: Foundation Tooling and
Architecture. It is the working plan for finishing the engineering setup that later
phases depend on.

## Goal

Provide a reproducible Linux development workflow so a new developer can clone the
repository, install dependencies, build, and run tests in under 10 minutes.

## Scope Decisions

- CI is Linux-only in Phase 0.
- This phase covers developer tooling, workflow, and documentation.
- The Doxygen pipeline should be configured now, but full generated API documentation is
  deferred until Phase 1 introduces meaningful public APIs.

## Current State

The following foundation work is already done or started:

- CMake project and preset structure
- Conan dependency management
- Compiler warning configuration
- Sanitizer integration
- `clang-format` and `clang-tidy` setup
- Module scaffolding
- Developer scripts toolbelt created under `scripts/` (`bootstrap.sh`, `format.sh`,
  `lint.sh`, `test.sh`, `check_headers.sh`)
- GitHub Actions CI started in `.github/workflows/ci.yml` with one canonical Linux
  GCC 13 configuration
- Repository-level VS Code configuration is ready (`.vscode/settings.json`,
  `.vscode/extensions.json`, `.vscode/tasks.json`)
- `.vscode/launch.json` is in place with an initial C++ debug profile and
  `JAQL: Build` as pre-launch task
- GitHub Actions CI: matrix job covering GCC 13 × {Debug, Release} and
  Clang 17 × {Debug, Release}; separate `lint` job (clang-format check + clang-tidy)
- `docs/dev/` now contains focused guides for Linux setup, script/CI behavior,
  and VS Code workflow
- Documentation reconciliation is intentionally deferred until the end of Phase 0 so
  updates reflect the stabilized workflow

## Remaining Deliverables

### Documentation Suite

In progress. Remaining pass required:

- Reconcile `README.md`, `CONTRIBUTING.md`, and `docs/` with the actual repository state.
- Remove or implement any referenced commands, scripts, or workflows.
- Expand or refine `docs/dev/` guides as workflows evolve.
- Document how Doxygen is generated and where generated output belongs.

Requirements:

- Quick start steps must be executable as written.
- Contribution guidance must match the real validation workflow.
- Documentation should minimize setup guesswork.

## Next Focus Order

1. Execute the documentation reconciliation and expansion pass at end of Phase 0.

## Success Criteria

Phase 0 is complete when all of the following are true:

1. A new Linux developer can clone the repository, install dependencies, configure,
   build, and run tests in under 10 minutes using documented steps.
2. A pull request is gated by a working Linux CI build and test workflow.
3. Local `format`, `lint`, and `test` workflows are available through documented scripts
   and match CI behavior.
4. VS Code setup is repository-defined and usable without per-developer tribal knowledge.
5. Documentation references only tools and workflows that actually exist.
6. The Doxygen pipeline is configured and documented, even if the generated API surface
   is still small.

## Out of Scope

- Windows CI gating
- Release automation beyond current development needs
- Full generated API reference coverage for modules not yet implemented
- Performance regression automation
