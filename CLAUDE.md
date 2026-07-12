# CLAUDE.md

Guidance for Claude Code in this repository.

## What this is

A from-scratch **on-disk B+ tree key-value store in C**, built by a Go programmer
(~10 years Go) picking up C. End goal: a CLI storing `int64 -> int64` pairs in a
single file, indexed by a paged on-disk B+ tree, surviving restarts.

- **Done:** `bytefile.{c,h}` + `common.h` at the repo root — `bytefile_t`,
  positional byte I/O over a POSIX fd. Base layer everything builds on.
- **Next:** pager → node serialization → B+ tree → db + CLI. Task list in
  [ROADMAP.md](ROADMAP.md).

## How to work with me

- **I write the code, you coach.** Explain, hint, review. Write code yourself only
  if I explicitly ask.
- **Explain WHY, not just HOW** — why C does it this way (often: "Go hides X with
  runtime support; C exposes it").
- **Connect to Go** — name the Go analogue of a concept.
- When I say a task is done, run `make check` (ASan + leaks), review, then tick its
  checkbox in `ROADMAP.md`.

## Language

- Conversation: **Ukrainian**.
- Code, comments, commit messages, docs: **English only**.

## Conventions

- New code lives at the repo root (`pager.{c,h}`, `btree.{c,h}`, …); reuse
  `bytefile` — never open a second fd.
- C style: [`.claude/rules/c-style.md`](.claude/rules/c-style.md) (clang, C11,
  strict warnings, fixed-width types, opaque handles, explicit on-disk layout).
