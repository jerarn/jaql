# VS Code Workflow

Repository-level VS Code configuration lives in `.vscode/`.

## Included Workspace Files

- `settings.json`: editor and clangd defaults
- `extensions.json`: recommended extensions
- `tasks.json`: bootstrap/configure/build/test/format/lint tasks
- `launch.json`: C++ debug configurations

## Recommended Extensions

`.vscode/extensions.json` is written for both Cursor and VS Code. Each editor installs
only the IDs it recognizes.

| Editor | Install | Notes |
|--------|---------|-------|
| **Cursor** | `anysphere.cpptools` | Pulls in clangd, CMake Tools, and CodeLLDB automatically. Microsoft IntelliSense is not used; language features come from clangd via `compile_commands.json`. |
| **VS Code** | `llvm-vs-code-extensions.vscode-clangd`, `ms-vscode.cmake-tools`, `ms-vscode.cpptools` | clangd handles completion and diagnostics; cpptools provides the `cppdbg` debugger used by `launch.json`. IntelliSense is disabled in workspace settings to avoid conflicting with clangd. |

After bootstrap, clangd picks up `build/gcc-debug/compile_commands.json` (configured in
`settings.json`). Until then, red squiggles and missing go-to-definition are expected.

Cursor-specific hooks and agent config are documented in [cursor-workflow.md](cursor-workflow.md).

## One-Time Setup

1. Install recommended extensions when prompted.
2. Install Conan — either `pip install --user 'conan>=2.4,<3'` or create a project
   `.venv` and `pip install 'conan>=2.4,<3'` there (see [setup.md](setup.md)). Workspace
   settings prepend `.venv/bin` to `PATH` in integrated terminals on Linux so bootstrap
   tasks work without manual activation.
3. Run task: `JAQL: Bootstrap`.
4. Run task: `JAQL: Build`.
5. Run task: `JAQL: Test`.

## Debugging

Launch configuration:

- Name: `JAQL: Debug test_infra_logger`
- Program: `build/gcc-debug/tests/unit/infra/test_infra_logger`
- Pre-launch task: `JAQL: Build`
- Prompt input: `gtestFilter` (default `*`)

Use a filter such as:

- `*` (all tests)
- `Logger*` (tests with matching suite/case prefix)

## Daily Loop

1. Edit code
2. Run `JAQL: Build`
3. Run `JAQL: Test`
4. Debug failing tests via launch profile
5. Run `JAQL: Format` and `JAQL: Lint` before committing

> **Lint prerequisite:** `JAQL: Lint` (`./scripts/lint.sh`) runs clang-tidy against the
> preset's compile database, so it needs a configured build first. Run `JAQL: Bootstrap`
> (or `JAQL: Build`) at least once so `build/<preset>/compile_commands.json` exists;
> otherwise the script exits early asking you to bootstrap. clang-tidy findings are
> reported as errors by design (`.clang-tidy` sets `WarningsAsErrors: "*"`) — a non-zero
> exit means there are diagnostics to review, not that the task is broken.
