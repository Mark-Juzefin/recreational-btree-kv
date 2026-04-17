# btree-kv

A step-by-step C learning project that ends with a small on-disk B+ tree
key-value store.

## Goal

Pick up C by building something real, in small progressive steps. The end
artifact is a CLI (`kvdb`) that stores `int64 -> int64` pairs across two files
— a paged B+ tree index and an append-only record log — supporting `put`,
`get`, `del`, and ordered `scan`.

The finished implementation lives in [`reference/`](reference/) as the target
to converge on. The point isn't the binary — it's durable understanding of
manual memory management, pointer-heavy APIs, on-disk layout, and the things
higher-level languages hide.

## Idea

The path is split into **nine small steps**, each introducing ≤3 new concepts
and producing working, tested code.

- **Steps 00-04** — C fundamentals: toolchain, pointers, strings, structs,
  heap memory, multi-file builds.
- **Steps 05-08** — apply the fundamentals to build the storage engine:
  POSIX file I/O, a 4 KB pager, a B+ tree in memory, then on disk.

Every step folder is a **self-contained mini project** with its own `Makefile`,
source files, and acceptance tests.

## Layout

- [`CLAUDE.md`](CLAUDE.md) — collaboration rules for Claude Code.
- [`docs/roadmap.md`](docs/roadmap.md) — full nine-step plan with status.
- [`docs/steps/`](docs/steps/) — one folder per step. Start at `00-hello/`.
- [`reference/`](reference/) — completed solution; consult only after finishing
  the corresponding step.

## Workflow

1. Open the active step's `README.md`.
2. Write the code with Claude as a coach: tiered hints, review, explanations —
   not finished solutions.
3. Run `make test` inside the step folder.
4. When green, `/review-step`, apply feedback, then `/next-step`.
