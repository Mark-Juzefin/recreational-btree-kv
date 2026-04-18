# Step 04 — Multi-file project & error handling

## Goal
Build a small module `person_t` — a struct that owns two heap strings
(`name`, `email`) — spread across proper `.h`/`.c` files, with a shared
`common.h` for error codes. The centrepiece is `person_create`: three
allocations in a row, any of which can fail. You'll use the `goto cleanup`
pattern to free what's already allocated and return a meaningful error.

## New concepts (3)

1. **`.h` / `.c` split** — headers declare the public API (types, function
   prototypes, constants). `.c` files hold the bodies. Include guards
   (`#ifndef ... #define ... #endif`) stop a header from being inlined twice
   into the same translation unit.
   - *Go analogue:* exported vs unexported identifiers within a package.
     Capital letter = exported, lowercase = package-private. In C you choose
     what to put in the `.h` — that's the "exported" surface.

2. **Error-code return pattern** — fallible functions return a small
   integer enum (`kv_result_t`) and deliver their result via an
   out-parameter. Zero means success; negatives mean specific errors. This
   makes every error path explicit — callers cannot forget to check.
   - *Go analogue:* `func f() (Result, error)`. C has no multi-return, so
     the result is written through a pointer (`out`) and the error becomes
     the function's return value.

3. **`goto cleanup` pattern** — when a function allocates multiple
   resources and any step can fail, you need to free what was already
   allocated before returning the error. C has no `defer`. The idiom is:
   one `goto fail_<N>` label per cumulative-cleanup level, ordered so that
   execution falls through, freeing everything backwards. It's the only
   place `goto` is idiomatic in modern C.
   - *Go analogue:* chained `defer` calls. Go runs them in LIFO order
     automatically; in C you stage the cleanup labels manually.

## Prerequisites
- Step 03 (heap allocation with `malloc` / `free`, ASan).

## Exercise

Build these files in the step folder:

- `common.h` — `kv_result_t` enum with at least `KV_OK`, `KV_ERR_NOMEM`,
  `KV_ERR_ARG`. Plus a small `kv_strerror(int)` helper.
- `person.h` — opaque-ish struct `person_t` (fields exposed for now; full
  opaque handles come later) and four prototypes:
  ```c
  kv_result_t person_create(const char *name, const char *email, person_t **out);
  void        person_free(person_t *p);
  kv_result_t person_rename(person_t *p, const char *new_name);
  const char *person_name(const person_t *p);
  ```
- `person.c` — implements them. `person_create` duplicates both input
  strings onto the heap using `strdup` (or `malloc`+`memcpy` if you want the
  lower-level version). Demonstrate `goto cleanup` for the three-step
  allocation.
- `person_test.c` — exercises success and failure paths.
- `Makefile` — compiles `person.c` + `person_test.c` into one binary,
  with `run` / `asan` / `leaks` / `check` targets analogous to step 03.

## Tasks

- [x] `common.h` with `kv_result_t` enum and `kv_strerror`
- [x] `person.h` with struct, prototypes, include guard
- [x] `person_create` with three-step alloc + `goto fail_N` cleanup
- [x] `person_free` (NULL-safe, frees both strings then struct)
- [x] `person_rename` (frees old name, dups new, error on NULL args)
- [x] `person_test.c` verifies create/rename/free and NULL-argument handling
- [x] `Makefile` compiles multi-file build
- [x] `make run` passes
- [x] `make check` (asan + leaks) passes clean

## Done when
- `make run` prints `all tests passed`
- `make check` passes with zero ASan reports and zero `leaks` output
- Zero compiler warnings
- `person.c` has no reachable path that leaks on error

## Key points to internalise
- A header declares **what exists**, a `.c` defines **how it works**.
  Rule of thumb: nothing goes in a header that isn't needed by other
  translation units.
- Error codes are boring and repetitive — that's the point. Every caller
  sees every possible failure mode in the signature.
- `goto` is banned in most style guides *except* for forward jumps to a
  cleanup label within the same function. Used this way, it's the
  clearest way to keep cleanup linear.

## References
- `man 3 strdup` — the string-duplication helper
- Linux kernel coding style, ch. 7 "Centralized exiting of functions"
  — the canonical explanation of `goto cleanup`
- [`.claude/rules/c-style.md`](../../../.claude/rules/c-style.md) — naming,
  headers, memory, error-handling conventions
- [`reference/src/common.h`](../../../reference/src/common.h) — similar
  error-enum pattern used in the final project (peek is OK here, the
  structure is what we're teaching)
