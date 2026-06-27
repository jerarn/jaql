# Cursor Workflow

Cursor-specific configuration for JAQL lives alongside the VS Code workspace files in `.vscode/`.

For C++ extensions, install **`anysphere.cpptools`** when prompted — it auto-installs
clangd, CMake Tools, and CodeLLDB. You do not need to install those separately. Workspace
settings in `.vscode/settings.json` (clangd arguments, format-on-save, CMake presets) apply
to the auto-installed clangd extension. See [vscode-workflow.md](vscode-workflow.md) for
the full extension matrix and one-time setup steps.

## Configuration Layout

| Path | Purpose |
|------|---------|
| [AGENTS.md](../AGENTS.md) | Entry point for AI agents — architecture, commands, doc index |
| `.cursor/rules/` | Persistent coding standards (scoped by file type where possible) |
| `.cursor/skills/` | On-demand workflow skills |
| `.cursor/hooks.json` | Post-edit automation (clang-format on `.hpp`/`.cpp`) |
| `.cursor/mcp.json.example` | Example MCP config for GitHub + Atlassian Cloud |
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

## MCP Integrations (GitHub + Confluence)

Example configuration: [`.cursor/mcp.json.example`](../../.cursor/mcp.json.example)

**Do not commit tokens.** Copy the example to your personal config and fill in credentials:

```bash
cp .cursor/mcp.json.example ~/.cursor/mcp.json
# Edit ~/.cursor/mcp.json — replace YOUR_* placeholders
```

Restart Cursor and verify green status under **Settings → Tools & Integrations → MCP**.

| Server | Purpose | Prerequisites |
|--------|---------|---------------|
| `github` | Repos, PRs, issues, Actions | [GitHub PAT](https://github.com/settings/personal-access-tokens/new) with repo scope |
| `mcp-atlassian` | Confluence (and Jira if configured) | [Atlassian API token](https://id.atlassian.com/manage-profile/security/api-tokens), `uv`/`uvx` on PATH |

Cursor often launches MCP with a minimal `PATH`. If Atlassian shows `spawn uvx ENOENT`, use the
absolute path to `uvx` in `mcp.json` (run `command -v uvx` in a terminal — typically
`~/.local/bin/uvx`).

For Confluence-only use, omit the `JIRA_*` environment variables from the Atlassian block.

Example prompts once connected:

```
List open PRs on jaql and summarize failing CI checks
```

```
Search Confluence for JAQL architecture documentation
```

## Rules vs Skills

- **Rules** load automatically based on open files (C++ standards when editing `.hpp`, test rules in `tests/`, etc.).
- **Skills** load only when invoked — use them for repeatable workflows that need templates and checklists.
