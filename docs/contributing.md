# Developer Contribution Guide

This is the comprehensive guide for JAQL development workflow, versioning strategy,
and release procedures.

For a quick reference, see the root [CONTRIBUTING.md](../CONTRIBUTING.md).

---

## Development Environment

### Required Tools

| Tool            | Version   | Install                                        |
|-----------------|-----------|------------------------------------------------|
| GCC or Clang    | 13 / 17   | `apt install gcc-13` or `apt install clang-17` |
| CMake           | 3.25+     | `apt install cmake` or https://cmake.org       |
| Ninja           | 1.11+     | `apt install ninja-build`                      |
| Conan           | 2.4+      | `pip install conan`                            |
| clang-format    | 17+       | `apt install clang-format-17`                  |
| clang-tidy      | 17+       | `apt install clang-tidy-17`                    |

### First-Time Setup

```bash
# 1. Clone
git clone https://github.com/jerarn/jaql.git
cd jaql

# 2. Install Conan 2 and create a default profile
pip install conan
conan profile detect

# 3. Run the bootstrap script (installs deps, configures gcc-debug)
./scripts/bootstrap.sh

# 4. Build and test
cmake --build --preset gcc-debug
ctest --preset gcc-debug --output-on-failure

# 5. Create a symlink to compile_commands.json (for clangd/IDE)
ln -sf build/gcc-debug/compile_commands.json compile_commands.json
```

### Verifying the Setup

A successful setup produces:
- All CMake configure steps passing with no warnings.
- All targets building with 0 compiler warnings.
- `ctest` reporting all tests passed.
- The `hello_jaql` example printing `JAQL v0.1.0`.

---

## Git Workflow

### Branch Strategy

| Branch         | Purpose                          | Merges into |
|----------------|----------------------------------|-------------|
| `main`         | Stable, always builds            | —           |
| `feat/<name>`  | New features                     | `main`      |
| `fix/<name>`   | Bug fixes                        | `main`      |
| `perf/<name>`  | Performance work                 | `main`      |
| `docs/<name>`  | Documentation                    | `main`      |

Rules:
- Small, self-contained changes can be committed directly to `main`.
- Use a branch for anything non-trivial, experimental, or in-progress.
- Delete branches after merging.

---

## Commit Conventions

JAQL uses [Conventional Commits 1.0.0](https://www.conventionalcommits.org/en/v1.0.0/).

### Format

```
<type>(<scope>): <description>

[optional body — explain why, not what]

[optional footer]
BREAKING CHANGE: <description>
Closes #<issue-number>
```

### Types

| Type       | Meaning                                         |
|------------|-------------------------------------------------|
| `feat`     | New functionality                               |
| `fix`      | Bug fix                                         |
| `perf`     | Performance improvement (no behaviour change)   |
| `refactor` | Code restructuring (no behaviour change)        |
| `test`     | Adding or correcting tests                      |
| `docs`     | Documentation only                              |
| `build`    | CMake, Conan, build system changes              |
| `ci`       | GitHub Actions workflow changes                 |
| `chore`    | Tooling, dependency bumps                       |

### Scope

The scope is the module name or infrastructure area:

```
feat(core): add StrongType comparison operators
fix(math): correct approx_equal for subnormal values
perf(curves): cache log-discount factors on construction
test(instruments): add Cashflow schedule generation tests
docs(roadmap): expand Phase 3 curve bootstrapping deliverables
build(cmake): add JAQL_ENABLE_ASAN option
ci(workflows): add ubuntu-24.04 to CI matrix
```

### Commit Quality Rules

1. The description line is ≤72 characters.
2. Description is in the imperative mood: "add X", "fix Y", not "added X" or "fixes Y".
3. No "WIP", "temp", "fixup", or "misc" in merge-ready commits. Use interactive rebase
   to clean up before merging.
4. Each commit is self-contained: it builds and all tests pass at that commit.

---

## Pull Request Process

PRs are optional for small changes committed directly to `main`. Use a PR for anything
non-trivial to keep a review trail and ensure CI runs before merge.

### Before Opening a PR

Run the checklist in the root [CONTRIBUTING.md](../CONTRIBUTING.md#pull-request-checklist).

### Merging

- CI must pass.
- Merge strategy: **squash merge** to keep `main` history clean.
- Link related issues with `Closes #<issue>` or `Related to #<issue>` in the PR body.

---

## Semantic Versioning

JAQL follows [Semantic Versioning 2.0.0](https://semver.org/):

```
MAJOR.MINOR.PATCH  (e.g., 1.2.3)
```

| Version component | When to bump                                              |
|-------------------|-----------------------------------------------------------|
| MAJOR             | Breaking API change (`BREAKING CHANGE:` in commit footer) |
| MINOR             | New backward-compatible feature (`feat:` commit)          |
| PATCH             | Backward-compatible bug fix or performance improvement    |

The project is pre-1.0.0. MINOR bumps may contain breaking changes during this phase.

---

## Release Process

1. Update the version in `CMakeLists.txt`: `project(jaql VERSION X.Y.Z ...)`.
2. Commit: `chore(release): bump version to X.Y.Z`.
3. Tag `main`:
   ```bash
   git tag -a vX.Y.Z -m "Release X.Y.Z"
   git push origin vX.Y.Z
   ```
4. The `release` GitHub Actions workflow triggers automatically on the tag push and
   creates a GitHub Release.

---

## Dependency Management

### Adding a Dependency

1. Search for the package on [ConanCenter](https://conan.io/center).
2. Evaluate: license compatibility (MIT, Apache 2.0, BSD preferred), maintenance
   status, version stability.
3. Add to `conanfile.py` under `requires`.
4. Add to `cmake/Dependencies.cmake` with a `find_package()` call.
5. Document in [docs/tech-stack.md](tech-stack.md) with version, purpose, and
   alternatives considered.

### Upgrading a Dependency

1. Check the upstream changelog for breaking changes.
2. Bump the version in `conanfile.py`.
3. Run the full test suite: `ctest --preset ci-gcc-debug`.
4. Update the version in [docs/tech-stack.md](tech-stack.md).
5. Commit: `chore(<scope>): upgrade <dep> from X.Y to X.Z`.

---

## Performance Regression Policy

If a change touches a hot path (interpolation, pricing loop, path generation):

1. Run the relevant benchmarks before and after using the `benchmark` preset.
2. Include the benchmark output in the PR description or commit body.
3. A regression of more than 10% requires explicit justification.
