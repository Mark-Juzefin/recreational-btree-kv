# Step 01 — Pointers & arrays

Builds `main.c` with `increment` / `sum` / `reverse` over `int` arrays.

## Concepts

1. **Pointers** (`*T`, `&x`, `*p`) — a variable holding a memory address. C has
   **no pass-by-reference**: to mutate a caller's variable you pass its address
   and dereference. This isn't a style choice like in Go — it's the only
   mechanism, so pointers show up constantly.

2. **Array-to-pointer decay** — an array's name in most contexts (especially a
   function argument) silently becomes a pointer to its first element. There is
   no "array as a value" you can pass around, and **length does not travel with
   it** — that's why every array function also takes a `size_t n`.
   - Go's slice bundles pointer + len + cap; a C array is just the pointer.

3. **`size_t`** — platform-sized *unsigned* integer, the type of `sizeof`,
   `strlen`, `malloc`, and every index/count. Go's convention is signed `int`;
   C's is specifically unsigned `size_t`. (Consequence to remember: a reverse
   loop `for (size_t i = n-1; i >= 0; i--)` never ends — unsigned never goes
   negative.)
