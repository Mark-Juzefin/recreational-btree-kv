# C Style Rules

Conventions the learner's code should follow. Matches the style in `reference/`.

## Compiler & Flags

- Compiler: `clang`
- Standard: `-std=c11`
- Always-on warnings: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`
- Debug builds: `-g -O0`; release: `-O2`; memory-check: `-fsanitize=address,undefined`
- Treat missing `return` as error: `-Werror=return-type`

## Types

- Prefer fixed-width integer types from `<stdint.h>`: `int32_t`, `int64_t`, `uint8_t`,
  `uint16_t`, `uint32_t`, `uint64_t`.
- Use `size_t` for sizes and array indices, `ssize_t` for return values of `read`/`write`.
- Avoid bare `int`, `long`, `short` in data structures — their size is platform-dependent.
- Use `bool` from `<stdbool.h>` with `true`/`false` (not `0`/`1`) for flags.

## Headers

- Every `.h` file has an include guard:
  ```c
  #ifndef KV_MODULE_H
  #define KV_MODULE_H
  /* ... */
  #endif
  ```
- Headers declare; `.c` files define. No function bodies in headers except `static inline`
  for very small utilities.
- Include only what you use. Don't include one header from another unless types are
  needed in the public interface.

## Error Handling

- Functions that can fail return an `int` error code (`KV_OK`, `KV_ERR_*`). Results
  are returned via out-parameters:
  ```c
  kv_result_t db_get(db_t *db, int64_t key, int64_t *value_out);
  ```
- Check every return value. For system calls, check specifically, handle `errno` if needed.
- Multi-step allocation with cleanup: use `goto fail` pattern. Never leak on error paths.

## Memory

- Zero-initialize with `calloc` unless you immediately fill every field.
- Pair every `malloc`/`calloc` with a `free` on every path. Run ASan to confirm.
- Opaque handles: `typedef struct foo foo_t;` in header, full definition in `.c`.
  Users hold only a `foo_t *`; never expose fields.

## Serialization

- Do **not** `memcpy` whole structs to/from disk — padding and endianness break portability.
- Encode explicitly: pick offsets, use `memcpy(buf + OFFSET, &field, sizeof(field))`.
- Document the on-disk layout at the top of the file (offsets + field sizes).

## Naming

- Functions and variables: `snake_case`.
- Macros and compile-time constants: `UPPER_SNAKE_CASE`.
- Types: `snake_case_t` suffix (`pager_t`, `node_t`). Stick to one convention per project.
- Prefix public symbols with module name: `pager_open`, `btree_insert`.

## Comments

- Default: no comments — let the code speak.
- Write a comment only when the WHY is non-obvious: a constraint, a gotcha, a
  deliberate trade-off.
- Do not narrate the code ("loop over keys" — the `for` loop already says that).
- Top-of-file comment is OK for explaining module purpose or on-disk format.

## Structure

- No hidden globals. State lives in opaque structs passed explicitly.
- Prefer small translation units: one concern per `.c` file.
- Keep functions under ~60 lines. If a function grows, extract helpers.

## Tests

- Simple harness with `CHECK(cond)` macro. `assert.h` is also fine for quick checks.
- Tests are `int main()` programs that exit non-zero on failure.
- Each step's tests go in the step's `tests.c` file alongside its `Makefile`.
