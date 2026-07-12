# btree-kv

A small on-disk B+ tree key-value store in C, built from scratch.

## Goal

A CLI that stores `int64 -> int64` pairs in a single file, indexed by a
**paged on-disk B+ tree**, surviving process restarts — `put`, `get`, and
(as stretch) `del` / `scan`. The point is durable understanding of manual
memory management, pointer-heavy APIs, and on-disk layout.

## Current state

- **Done:** `bytefile_t` — positional byte I/O over a POSIX fd
  (`bytefile.{c,h}`, `common.h`, tests in `bytefile_test.c`). This is the base
  layer everything else builds on.
- **Next:** pager → node serialization → B+ tree → db + CLI. See
  [`ROADMAP.md`](ROADMAP.md) for the end result and the task list.

## Build

```sh
make run     # build + run tests
make check   # ASan + leaks
make clean
```

## Layout

- [`ROADMAP.md`](ROADMAP.md) — end result + task list.
- [`CLAUDE.md`](CLAUDE.md) — collaboration rules for Claude Code.
- `bytefile.{c,h}`, `common.h`, `bytefile_test.c` — the done base code.
