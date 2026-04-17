# Step 02 — Strings & argv

## Goal
Learn how C represents strings (spoiler: as `char *` with a null terminator),
how to measure and compare them with the standard library, and how to read
command-line arguments from `main`.

## New concepts (3)

1. **C strings: `char *` + `\0`** — no built-in string type. A "string" is a
   pointer to the first character of a sequence of bytes, ending with a zero
   byte (`'\0'`). Every function that operates on strings relies on this sentinel.
   - *Go analogue:* Go's `string` carries a pointer + length. C carries only the
     pointer; length is implied by scanning until the `\0`. Slower to measure
     but smaller in memory.

2. **`<string.h>`: `strlen`, `strcmp`** — `strlen(s)` returns the number of
   bytes before the `\0`; `strcmp(a, b)` returns `0` if strings are equal,
   negative/positive otherwise (lexicographic order).
   - *Go analogue:* `len(s)` is O(1) in Go (stored length). `strlen` in C is
     O(n) — it walks until `\0`. `strcmp` ≈ `strings.Compare`.

3. **`argc` / `argv` in `main`** — `int main(int argc, char **argv)`. `argc` is
   the count of arguments (including the program name). `argv` is an array of
   C strings. `argv[0]` is the program name; user arguments start at `argv[1]`.
   - *Go analogue:* `os.Args`. Same shape: slice of strings; `os.Args[0]` is
     the program. In C you always get `argc` separately because arrays don't
     carry length.

## Prerequisites
- Step 01 (pointers, `*T`, `size_t`).

## Exercise

Build a tiny CLI `echoer` that prints each argument on its own line, along
with its length. Example:

```
$ ./echoer hello world
hello (5)
world (5)
```

If passed the flag `--upper`, convert each argument to uppercase before
printing (ASCII only — don't worry about UTF-8).

```
$ ./echoer --upper hello world
HELLO (5)
WORLD (5)
```

## Tasks

- [x] `int main(int argc, char **argv)` signature and a loop over `argv[1..argc)`
- [x] Use `strlen` to print each arg's length
- [x] Use `strcmp` to detect the `--upper` flag (skip it when printing)
- [x] Implement an `upper_inplace(char *s)` helper that uppercases ASCII in place
- [x] `make run-tests` passes all cases
- [x] Zero warnings

## Done when
- `make run ARGS='hello world'` prints:
  ```
  hello (5)
  world (5)
  ```
- `make run ARGS='--upper hi there'` prints:
  ```
  HI (2)
  THERE (5)
  ```
- `make run-tests` is green.

## References
- `man 3 strlen`, `man 3 strcmp`, `man 3 toupper`
- `<ctype.h>` — `toupper(int c)`, `islower(int c)`
- [`.claude/rules/c-style.md`](../../../.claude/rules/c-style.md)
