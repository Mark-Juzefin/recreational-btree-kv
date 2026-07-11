# Step 03 — Structs & heap memory

Builds `intvec_t` — a hand-rolled dynamic array of `int64_t` (a Go slice by hand).

## Concepts

1. **`struct` + `typedef`** — a struct groups fields; the `typedef` is what lets
   you write `intvec_t x` instead of `struct intvec x`. Go folds both into one
   `type Foo struct{}`; in C they're two separate steps.

2. **`malloc` / `realloc` / `free`** — manual heap ownership, no GC.
   - `malloc(n)` gives `n` bytes of **uninitialised** memory (contains garbage —
     `calloc` zeroes instead).
   - `realloc(p, n)` may **move** the block and return a *new* pointer; the old
     one is then dangling. Always write `p = realloc(p, n)`, never keep the old.
   - Every `malloc` needs exactly one `free`. Zero → leak; twice → double-free
     crash. This manual tracking is the whole reason C feels heavier than Go.

3. **AddressSanitizer** (`-fsanitize=address`) — instruments the binary to catch
   leaks, double-free, use-after-free, and out-of-bounds. It's your GC-less
   safety net; treat a clean ASan run as part of "done", like Go's race detector.

Growth strategy worth remembering: double `cap` when `len == cap`, so N pushes
cost O(N) amortised, not O(N²). `intvec_get` bounds-checks with `assert` — a
programmer-error guard, compiled out under `-DNDEBUG`.
