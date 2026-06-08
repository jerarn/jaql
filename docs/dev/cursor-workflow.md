# Cursor Workflow

Cursor-specific configuration for JAQL lives alongside the VS Code workspace files in `.vscode/`.

## Configuration Layout

| Path | Purpose |
|------|---------|
| [AGENTS.md](../AGENTS.md) | Entry point for AI agents — architecture, commands, doc index |
| `.cursor/rules/` | Persistent coding standards (scoped by file type where possible) |
| `.cursor/skills/` | On-demand workflow skills |
| `.cursor/hooks.json` | Post-edit automation (clang-format on `.hpp`/`.cpp`) |
| `.cursorignore` | Excludes build artifacts from agent indexing |

Human-readable quick-reference: [.github/copilot-instructions.md](../.github/copilot-instructions.md)

## Workflow Skills

Invoke skills in Cursor chat with `@` or by describing the task:

| Skill | Use for |
|-------|---------|
| `@jaql-new-test` | Generate a GoogleTest file for a public header |
| `@jaql-commit-message` | Conventional Commit message from staged changes |
| `@jaql-new-adr` | Scaffold a new Architecture Decision Record |
| `@jaql-new-module` | Scaffold headers, sources, tests, and CMake for a module |

### Example Prompts

```
@jaql-new-test Generate tests for include/jaql/infra/logger.hpp
```

```
@jaql-commit-message
```

```
@jaql-new-adr Use Conan for Eigen dependency management
```

```
@jaql-new-module Scaffold the curves module in the domain layer
```

## Daily Loop

Same as the [VS Code workflow](vscode-workflow.md):

1. Edit code
2. `cmake --build --preset gcc-debug` (or task **JAQL: Build**)
3. `./scripts/test.sh` (or task **JAQL: Test**)
4. `./scripts/format.sh --check` and `./scripts/lint.sh` before committing

Agent edits to `.hpp` and `.cpp` files are auto-formatted via the `afterFileEdit` hook.

## Rules vs Skills

- **Rules** load automatically based on open files (C++ standards when editing `.hpp`, test rules in `tests/`, etc.).
- **Skills** load only when invoked — use them for repeatable workflows that need templates and checklists.
