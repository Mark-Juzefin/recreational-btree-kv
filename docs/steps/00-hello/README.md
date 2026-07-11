# Step 00 — Hello, C

Prints `Hello, C!` plus `sizeof(int)` / `sizeof(long)` / `sizeof(int64_t)` — the
first program, and a look at how big C's integer types actually are.

## Concepts

1. **`int main(void)` + exit codes** — execution starts at `main`; its `int`
   return is the process exit code (0 = success, non-zero = failure). Go's
   runtime returns for you and you call `os.Exit`; in C you return it yourself.

2. **`#include` and the preprocessor** — `#include <stdio.h>` literally pastes
   the header's text into your file before compilation. C has no language-level
   modules — just files concatenated by the preprocessor. (This dumb pasting is
   why include guards exist; see step 04.) Go's `import` is semantic by contrast.

3. **Fixed-width types from `<stdint.h>`** — bare `int`/`long`/`short` have
   *platform-dependent* sizes: on this Mac `long` is 8 bytes, on 64-bit Windows
   it's 4. `int64_t` & friends are guaranteed exact widths. Anything that lands
   in a struct or on disk uses the fixed-width types, or the on-disk layout
   breaks across platforms. Go had these built in from day one; C bolted them on
   in C99 over the older, fuzzy `int`/`long`.

Use `%zu` in `printf` for the `size_t` that `sizeof` returns — not `%d`.
