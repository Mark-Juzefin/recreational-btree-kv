# Step 00 — Hello, C

## Goal
Write your first C program, compile it with `clang`, and run it. See that the
toolchain works end-to-end.

## New concepts (3)

1. **`int main(void)` and exit codes** — Every C program starts at `main`. Its
   return value is the process exit code (0 = success).
   - *Go analogue:* `func main()`. In C you return an int explicitly; in Go the
     runtime handles it and you call `os.Exit(code)` for non-zero.

2. **`#include` and the preprocessor** — `#include <stdio.h>` textually splices
   the header's declarations into your file before compilation. C has no modules
   at the language level — only files that get concatenated by the preprocessor.
   - *Go analogue:* `import "fmt"`. But Go imports are semantic; C `#include` is
     literal text substitution. This is why header guards exist (step 04).

3. **Fixed-width types from `<stdint.h>`** — `int32_t`, `int64_t`, `uint8_t`, etc.
   Bare `int` / `long` sizes vary by platform (!), so data structures use the
   fixed-width types. You'll use these everywhere going forward.
   - *Go analogue:* Go already has `int32`, `int64`, `uint8` built-in. In C you
     must `#include <stdint.h>` to get them.

## Prerequisites
- `clang` is installed: `clang --version` should print something.

## Exercise

Create `hello.c` in this folder. Your program must:

1. Print `Hello, C!` on its own line.
2. Print three lines showing the size of different types on your platform, like:
   ```
   sizeof(int)     = 4
   sizeof(long)    = 8
   sizeof(int64_t) = 8
   ```
   Use `printf` with `%zu` for `size_t` values returned by `sizeof`.
3. Return `0` from `main`.

You'll also use the provided [`Makefile`](Makefile). It defines:
- `make hello` — compile `hello.c` to an executable named `hello`
- `make run` — build and run it
- `make clean` — remove the executable

## Tasks

- [ ] `hello.c` exists with an `int main(void)` that returns `0`
- [ ] Output includes `Hello, C!` on its own line
- [ ] Output shows sizes of `int`, `long`, and `int64_t` (all via `sizeof`)
- [ ] `make run` prints the expected output
- [ ] No compiler warnings with the project's strict flag set

## Done when
- `make run` succeeds and the output is correct.
- The file compiles with zero warnings under `-Wall -Wextra -Wpedantic`.

## References
- `man 3 printf` — format specifiers (use `%d` for `int`, `%zu` for `size_t`)
- `<stdio.h>` — declares `printf`
- `<stdint.h>` — declares `int32_t`, `int64_t`, etc.

When you're done, ask Claude `/review-step`. Don't peek at `reference/` —
there's nothing there for this step anyway.

## Stuck?

Ask Claude. Hints come in tiers (name the concept → describe the approach →
code sketch), so you learn the pattern instead of just copying a fix.
