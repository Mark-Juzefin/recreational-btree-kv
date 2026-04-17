---
description: Review the learner's code for the active step
---

# /review-step

Review the code in the active step folder.

## Actions

1. Read the active step's `README.md` to know the goal and constraints.
2. Read all source files the learner wrote (`*.c`, `*.h`) in that folder.
3. Run `make test` inside the step folder.
4. Report findings in three buckets:

### 🐞 Bugs (must fix)
Issues that make the code incorrect or unsafe. Cite file:line. Explain impact.

### 🎨 Style (should fix)
Non-idiomatic C or violations of `.claude/rules/c-style.md`. Cite file:line. Suggest
the fix but let the learner apply it.

### 💡 Alternatives (could fix)
Other ways to structure the solution. Explain the trade-off, not a verdict.

5. Highlight what's **correct and idiomatic** — don't only nitpick.
6. If tests fail: don't fix them. Describe what went wrong and point at the likely
   cause. Let the learner debug it.

## Rules

- No unsolicited refactoring. Describe, don't rewrite.
- Flag concepts that belong to a later step as "we'll revisit in step NN".
- If the code is premature-abstracted or overbuilt for the step, call it out — the
  goal is minimal code that works, not extensible code.
