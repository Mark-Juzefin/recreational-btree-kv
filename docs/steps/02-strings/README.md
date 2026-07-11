# Step 02 — Strings & argv

Builds a tiny `echoer` CLI (prints each arg + length, `--upper` uppercases ASCII).

## Concepts

1. **C strings = `char *` + `\0`** — there is no string type. A string is a
   pointer to bytes ending in a zero byte. Every stdlib string function trusts
   that sentinel is there; lose it and functions read off the end into garbage.
   - Go's `string` stores an explicit length, so it can hold `\0` and measuring
     is O(1). C measures by scanning to `\0` — smaller, but O(n) and fragile.

2. **`strlen` / `strcmp`** — `strlen` walks to the `\0` (O(n), not stored).
   `strcmp` returns `0` when **equal** — the inverted sense trips up everyone:
   `if (strcmp(a,b))` means "if *different*".

3. **`argc` / `argv`** — `int main(int argc, char **argv)`. `argv[0]` is the
   program name, real args start at `argv[1]`, and `argv[argc]` is `NULL`. You
   get `argc` separately precisely because the array can't carry its own length.
   - Go's `os.Args` is a slice, so no separate count.

Uppercasing is in-place (`char *s`) — the arg buffer is writable, so you mutate
bytes directly rather than allocating a new string.
