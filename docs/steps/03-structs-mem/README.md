# Step 03 — Structs & heap memory

## Goal
Build a dynamic array of `int64_t` values (`intvec_t`) — essentially a hand-rolled
Go slice. Learn to define structs, allocate memory on the heap with `malloc`,
grow it with `realloc`, release it with `free`, and verify no leaks with
AddressSanitizer.

## New concepts (3)

1. **`struct` and `typedef`** — group related fields into a named type. Without
   `typedef` you write `struct foo x`; with it, you get a cleaner `foo_t x`.
   - *Go analogue:* `type Foo struct { ... }` — same idea, Go does the
     `typedef` implicitly. In C, the `typedef` is separate ceremony.

2. **Heap allocation: `malloc`, `realloc`, `free`** — `malloc(n)` returns a
   pointer to `n` bytes of uninitialized memory. `realloc(p, n)` grows/shrinks
   an existing allocation. `free(p)` releases it. Every `malloc` needs exactly
   one matching `free`, or you leak; calling `free` twice on the same pointer
   crashes (double-free).
   - *Go analogue:* `make([]T, n)` + GC. In C you track ownership manually.

3. **AddressSanitizer (ASan)** — a compiler + runtime tool that instruments
   your binary to catch heap bugs: leaks, double-frees, use-after-free, buffer
   overflows. Enabled with `-fsanitize=address`. Essentially "put a guard band
   around every allocation and track the state of every byte".
   - *Go analogue:* Go's race detector is conceptually similar (compile-time
     instrumentation). Go doesn't need ASan because GC handles free and
     out-of-bounds access panics immediately.

## Prerequisites
- Steps 00-02 (pointers, arrays, strings).

## Exercise

Build `intvec.c` + `intvec_test.c`:

- `intvec.c` — defines the struct and the four operations:
  ```c
  intvec_t *intvec_new(size_t initial_cap);
  void      intvec_free(intvec_t *v);
  void      intvec_push(intvec_t *v, int64_t x);
  int64_t   intvec_get(const intvec_t *v, size_t i);
  ```
- `intvec_test.c` — exercises them: create, push 1000 items, verify a couple
  indices, free.

The Makefile provides `make run` (normal build) and `make asan` (rebuild with
AddressSanitizer + run tests — a leak or misuse shows a detailed report).

## Tasks

- [x] Define `intvec_t` with `int64_t *data`, `size_t len`, `size_t cap`
- [x] Implement `intvec_new` (allocates struct + initial data array)
- [x] Implement `intvec_push` (growth via `realloc` when `len == cap`)
- [x] Implement `intvec_get` (bounds-check with `assert`)
- [x] Implement `intvec_free` (frees data array, then struct)
- [x] `intvec_test.c` creates, pushes 1000 items, verifies, frees
- [x] `make run` passes
- [x] `make asan` passes with zero ASan reports

## Done when
- `make run` prints `all tests passed`
- `make asan` runs the same tests under sanitizer with no errors
- Zero compiler warnings

## References
- `man 3 malloc`, `man 3 realloc`, `man 3 free`
- `<assert.h>` — `assert(expr)` aborts the program if `expr` is false
- [AddressSanitizer docs](https://clang.llvm.org/docs/AddressSanitizer.html)
- [`.claude/rules/c-style.md`](../../../.claude/rules/c-style.md)
