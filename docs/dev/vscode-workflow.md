# VS Code Workflow

Repository-level VS Code configuration lives in `.vscode/`.

## Included Workspace Files

- `settings.json`: editor and clangd defaults
- `extensions.json`: recommended extensions
- `tasks.json`: bootstrap/configure/build/test/format/lint tasks
- `launch.json`: C++ debug configurations

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
