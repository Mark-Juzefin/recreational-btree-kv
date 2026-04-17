# Step 01 — Pointers & arrays

## Goal
Understand how C moves data into and out of functions when the caller wants to
see changes. Practice pointer syntax (`*`, `&`) and the way arrays decay into
pointers when passed around.

## New concepts (3)

1. **Pointers** (`*T`, `&x`, `*p`) — a variable holding a memory address. C has
   no pass-by-reference; to mutate a caller's variable, pass its address.
   - *Go analogue:* `*T`, `&x`, `*p` — same spelling, same idea. In Go you use
     them explicitly when you need mutation; in C you use them constantly
     because there's no alternative.

2. **Array-to-pointer decay** — using an array's name in most contexts
   (including function arguments) automatically yields a pointer to its first
   element. There is no "the array itself" as a value type.
   - *Go analogue:* slices carry a pointer + length + cap. C arrays carry only
     the pointer; length travels separately.

3. **`size_t` for sizes and indices** — a platform-sized unsigned integer
   (`unsigned long` on 64-bit macOS/Linux). Use it for anything that counts
   elements or bytes. Matches `sizeof`, `strlen`, `malloc`.
   - *Go analogue:* Go's `int` (platform-sized signed). In C, the convention
     for sizes is specifically `size_t`, unsigned.

## Prerequisites
- Step 00 (toolchain, `printf`, types).

## Exercise

Build `main.c` in this folder. You'll write three small functions that exercise
each concept, plus a `main` that calls them and prints results.

1. `void increment(int *x)` — adds 1 to the value `x` points at.
2. `int sum(int *arr, size_t n)` — returns the sum of `n` elements in `arr`.
3. `void reverse(int *arr, size_t n)` — reverses the array in place.

## Tasks

- [x] `increment` modifies the caller's variable via pointer
- [x] `sum` iterates an array passed as `int *` + length
- [x] `reverse` swaps elements in place using a temporary variable
- [x] `main` calls all three and prints results so they can be visually verified
- [x] Compiles cleanly under `make run`

## Done when
- `make run` prints the expected output:
  ```
  increment: 6
  sum: 150
  reverse: 5 4 3 2 1
  ```
- Zero warnings under `-Wall -Wextra -Wpedantic -Wconversion`.

## References
- `man 3 printf` — format specifiers
- [`.claude/rules/c-style.md`](../../../.claude/rules/c-style.md) — project conventions
