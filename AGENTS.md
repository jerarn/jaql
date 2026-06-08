# JAQL — Agent Instructions

JAQL (Just A Quant Library) is a modular C++23 quantitative finance library built for
correctness, explicitness, testability, and measurable performance.

## Architecture

Three layers with **strict one-way dependencies** (violations are build errors):

```
Foundation:  infra → core → math
Domain:      instruments, market, curves  (depend on Foundation)
Analytics:   pricing, simulation, risk, portfolio  (depend on Domain + Foundation)
```

Full dependency graph: [docs/architecture/module-dependencies.md](docs/architecture/module-dependencies.md)

| Path | Contents |
|------|----------|
| `include/jaql/<module>/` | Public headers |
| `src/<module>/` | Implementation |
| `tests/unit/<module>/` | Unit tests |
| `tests/integration/` | Integration tests |

## Daily Dev Loop

```bash
./scripts/bootstrap.sh                  # first-time / dependency setup
cmake --build --preset gcc-debug        # build
./scripts/test.sh                       # run tests
./scripts/format.sh --check             # validate formatting before commit
./scripts/lint.sh                       # clang-tidy (requires configured build)
```

## Cursor Configuration

| Location | Purpose |
|----------|---------|
| `.cursor/rules/` | Persistent coding standards (file-scoped where possible) |
| `.cursor/skills/` | Workflow skills — invoke with `@jaql-new-test`, `@jaql-commit-message`, `@jaql-new-adr`, `@jaql-new-module` |
| [docs/dev/cursor-workflow.md](docs/dev/cursor-workflow.md) | Cursor usage guide |

Human-readable quick-reference: [.github/copilot-instructions.md](.github/copilot-instructions.md)
(keep in sync with `.cursor/rules/` when standards change).

## Key Documentation

| Topic | File |
|-------|------|
| Coding standards | [docs/coding-standards.md](docs/coding-standards.md) |
| Architecture | [docs/architecture/overview.md](docs/architecture/overview.md) |
| Module dependencies | [docs/architecture/module-dependencies.md](docs/architecture/module-dependencies.md) |
| Build system | [docs/build-system.md](docs/build-system.md) |
| Testing strategy | [docs/testing-strategy.md](docs/testing-strategy.md) |
| Contributing | [docs/contributing.md](docs/contributing.md) |
