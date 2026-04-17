# Roadmap

Progressive path from Hello-World-in-C to an on-disk B+ tree. Each step introduces
≤3 new concepts and produces working code that passes its own test suite.

**Legend:** 🟢 Done · 🟡 In progress · ⚪ Not started

## Steps

| #  | Step                           | New concepts                                           | Est.    | Status |
|----|--------------------------------|--------------------------------------------------------|---------|--------|
| 00 | [Hello, C](steps/00-hello/)    | `main`, `printf`, fixed-width types, clang, Makefile   | 30 min  | 🟡 |
| 01 | Pointers & arrays              | `*`, `&`, array decay, pass-by-pointer                 | 1 h     | ⚪ |
| 02 | Strings & argv                 | `char *`, `\0`, `strlen`/`strcmp`, `argv`              | 1 h     | ⚪ |
| 03 | Structs & heap memory          | `struct`, `typedef`, `malloc`/`free`, ASan             | 1.5 h   | ⚪ |
| 04 | Multi-file & error handling    | `.h`/`.c` split, headers, error codes, `goto cleanup` | 1 h     | ⚪ |
| 05 | POSIX file I/O                 | `open`/`read`/`write`/`pread`/`pwrite`, `fsync`        | 1.5 h   | ⚪ |
| 06 | Pager (4 KB page I/O)          | Page abstraction, on-disk header, explicit layout      | 1.5 h   | ⚪ |
| 07 | B+ tree in memory              | Recursion over trees, split propagation                | 2 h     | ⚪ |
| 08 | B+ tree on disk                | Combine pager + tree; serialization; reopen            | 2 h     | ⚪ |

**Total:** ~12 h of focused work, spread across sessions.

## Concept Dependency Graph

```
00 hello                   → knows: toolchain, int, printf
  └─▶ 01 pointers           → knows: *, &, arrays
        └─▶ 02 strings      → knows: char *, argv
              └─▶ 03 structs+mem  → knows: struct, malloc/free, ASan
                    └─▶ 04 multi-file → knows: .h/.c, goto cleanup
                          ├─▶ 05 file I/O    → knows: POSIX fd, pread/pwrite
                          │     └─▶ 06 pager  → knows: page-based storage
                          │           └─▶ 08 on-disk B+ tree
                          └─▶ 07 tree in memory → knows: B+ tree algorithm
                                └─▶ 08 on-disk B+ tree
```

Step 08 is the convergence — by that point all building blocks exist.

## How to Use This Roadmap

- **Active step** is tracked in [`CLAUDE.md`](../CLAUDE.md) and updated when a step completes.
- Each step folder is self-contained: you can `cd` into it, `make test`, and have a
  working (smaller) C project.
- Steps 00-04 give you the C language itself. Steps 05-08 apply it to build a real thing.
- If a step feels too big, tell Claude — we split it.
