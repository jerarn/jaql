# Contributing to JAQL

Thank you for your interest in contributing. This file provides the essential quick reference for contributors. For comprehensive developer documentation, see [docs/contributing.md](docs/contributing.md).

---

## Development Setup

**Prerequisites:** CMake 3.25+, GCC 13+ or Clang 17+, Ninja (recommended), Conan 2.

```bash
# Install dependencies and configure
./scripts/bootstrap.sh

# Build and test
cmake --build --preset gcc-debug
./scripts/test.sh
```

---

## Branch Strategy

| Branch           | Purpose               | Merges Into |
|------------------|-----------------------|-------------|
| `main`          | Stable, always builds | —           |
| `feat/<name>`   | New features          | `main`      |
| `fix/<name>`    | Bug fixes             | `main`      |
| `perf/<name>`   | Performance work      | `main`      |
| `docs/<name>`   | Documentation         | `main`      |

Small changes can be committed directly to `main`. Use a branch for anything
non-trivial or experimental.

---

## Commit Conventions

This project uses [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <concise description>

[optional body]

[optional footer: BREAKING CHANGE, Closes #issue]
```

**Types:** `feat` · `fix` · `perf` · `refactor` · `test` · `docs` · `chore` · `ci` · `build`

**Scope:** module name — e.g. `core`, `math`, `pricing`, `cmake`, `ci`

**Examples:**

```
feat(core): add StrongType arithmetic comparison operators
fix(math): correct approx_equal behaviour for subnormal values
perf(simulation): replace std::vector reallocation with arena allocator
docs(roadmap): expand Phase 2 deliverables and risk section
test(core): prevent StrongType implicit conversion at compile time
```

---

## Pull Request Checklist

Before opening a PR, verify all of the following:

- [ ] All tests pass: `./scripts/test.sh`
- [ ] No formatting violations: `scripts/format.sh && git diff --exit-code`
- [ ] No clang-tidy warnings: `scripts/lint.sh`
- [ ] All public headers are self-contained: `scripts/check_headers.sh`
- [ ] API documentation passes: `./scripts/check_docs.sh` (requires `./scripts/bootstrap.sh --docs`)
- [ ] Commit messages follow Conventional Commits
- [ ] No new warnings with `-Wall -Wextra -Wpedantic`

---

## Code Style

Formatting is enforced by `clang-format`. Run `scripts/format.sh` before every commit, or configure your editor to format on save. See [docs/coding-standards.md](docs/coding-standards.md)
for the full style guide.

---

## Reporting Issues

Use the GitHub issue templates:
- **Bug reports:** include compiler version, platform, reproduction steps, and expected vs actual behaviour.
- **Feature requests:** describe the use case and financial context, not just the API you want.

---

## License

By contributing to JAQL, you agree that your contributions will be licensed under the MIT License. See [LICENSE](LICENSE) for terms.
