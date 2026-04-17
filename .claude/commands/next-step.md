---
description: Advance to the next learning step
---

# /next-step

Mark the current step as done and set up the next one.

## Actions

1. Confirm the active step's tests pass (`make test` inside `docs/steps/NN-.../`).
2. Update `docs/roadmap.md`: set current step to 🟢 Done, next step to 🟡 In progress.
3. Update `CLAUDE.md`: change "Active step" link to the next step.
4. Create the next step folder `docs/steps/{NN+1}-{name}/` with:
   - `README.md` — exercise spec using the template below
   - `tests.c` — acceptance tests
   - `Makefile` — per-step build (if applicable)
5. Summarize what concepts the finished step exercised.
6. Show the new step's goal and first task.

## Step README Template

```markdown
# Step NN — {Title}

## Goal
One sentence. What the learner builds.

## New concepts (≤3)
- **Concept A** — what it is, why C needs it, Go analogue.
- **Concept B** — ...

## Prerequisites
What prior steps / concepts you should already know.

## Exercise
Describe what to build, in business terms. Include:
- Function signatures (or a rough API shape)
- Expected behavior
- What to put in which file

## Tasks
- [ ] Task 1 (specific, small, testable)
- [ ] Task 2
- [ ] Task 3
- [ ] All tests in `tests.c` pass

## Done when
- `make test` prints `all tests passed`
- Code compiles with no warnings under the project's strict flag set

## References
- `man` pages, stdlib headers, reference file snippets.
- Pointers into `reference/` for after-the-fact comparison only.
```

## Rules

- Don't skip steps even if the learner insists — gentle push-back with the reason.
- Don't write the solution code. Only the exercise spec and the tests.
- Keep new concepts ≤3 per step.
