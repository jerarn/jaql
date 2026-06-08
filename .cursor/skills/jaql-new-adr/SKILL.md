---
name: jaql-new-adr
description: Create a new Architecture Decision Record (ADR) following the JAQL ADR template. Use when the user mentions ADR, architecture decision record, or wants to document a design decision.
disable-model-invocation: true
---

# JAQL New ADR

Create a new ADR for the decision title the user provides.

## Before Creating

1. Read [docs/adr/ADR-0001-conan2-package-manager.md](docs/adr/ADR-0001-conan2-package-manager.md) for the existing format.
2. List all files in `docs/adr/` to determine the next ADR number.

Save the new file as `docs/adr/ADR-<NNNN>-<kebab-case-title>.md`.

## Template

```markdown
# ADR-<NNNN>: <title>

| Field         | Value                              |
|---------------|------------------------------------|
| Status        | Proposed                           |
| Date          | <today's date, YYYY-MM-DD>         |
| Deciders      | <author name(s)>                   |
| Supersedes    | —                                  |
| Superseded by | —                                  |

---

## Context

<!-- What is the situation, constraint, or requirement that forces a decision?
     Be specific: what problem are we solving, for whom, and why now? -->

---

## Decision

<!-- State the decision in one or two sentences.
     "We will use X" or "We will not use Y because Z." -->

---

## Alternatives Considered

### Option A: <name>

**Pros:**
-

**Cons:**
-

### Option B: <name>

**Pros:**
-

**Cons:**
-

---

## Consequences

### Positive

-

### Negative / Trade-offs

-

### Neutral

-

---

## References

-
```

## Instructions

1. Determine the next ADR number (pad to 4 digits, e.g. `0002`).
2. Convert the title to `kebab-case` for the filename.
3. Fill in today's date in `YYYY-MM-DD` format.
4. Leave `Status` as `Proposed` — the user will change it to `Accepted` or `Rejected` after review.
5. Do not invent context or decisions. Fill only what can be inferred from the title;
   leave comment placeholders for sections the user must complete.
6. After creating the file, print the full path so the user can open it immediately.
