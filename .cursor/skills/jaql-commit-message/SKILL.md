---
name: jaql-commit-message
description: Generate a Conventional Commit message from staged git changes for JAQL. Use when the user asks for a commit message, conventional commit, or help with staged diff.
disable-model-invocation: true
---

# JAQL Commit Message

Inspect the staged changes with `git diff --staged` and generate a commit message
that conforms to **Conventional Commits 1.0.0** as used in JAQL.

## Format

```
<type>(<scope>): <description>

[optional body — explain WHY, not what]

[optional footer]
BREAKING CHANGE: <description>
Closes #<issue-number>
```

## Types

| Type       | Use when                                                  |
|------------|-----------------------------------------------------------|
| `feat`     | New functionality                                         |
| `fix`      | Bug fix                                                   |
| `perf`     | Performance improvement, no behaviour change              |
| `refactor` | Code restructuring, no behaviour or interface change      |
| `test`     | Adding or correcting tests only                           |
| `docs`     | Documentation only                                        |
| `build`    | CMake, Conan, build system changes                        |
| `ci`       | GitHub Actions workflow changes                           |
| `chore`    | Tooling, dependency bumps, non-functional maintenance     |

## Scope

The scope is the **module name** or **infrastructure area** — pick the narrowest one
that accurately describes what changed:

```
core · math · infra · instruments · market · curves
pricing · simulation · risk · portfolio
cmake · conan · ci · docs · scripts
```

## Rules

1. Description line: **≤ 72 characters**, imperative mood ("add X", not "added X").
2. No "WIP", "temp", "fixup", or "misc" in a merge-ready commit.
3. Each commit must be self-contained: it builds and all tests pass at that commit.
4. If the change introduces a public API break, add `BREAKING CHANGE:` in the footer.
5. If the change closes a GitHub issue, add `Closes #<n>` in the footer.

## Output

Print **only** the final commit message inside a fenced code block — no preamble,
no explanation, no alternatives. The user will copy-paste it directly into the terminal.

If the staged diff is empty, tell the user there is nothing staged and stop.
