# Step 05 — POSIX file I/O

## Goal
Build `bytefile_t` — a wrapper around a POSIX file descriptor that lets
you read/write bytes at arbitrary offsets and `fsync` to disk. This is
the primitive that the next step (pager) will build on. Along the way
you'll meet file descriptors, positional I/O, short reads/writes, and
the **opaque handle** pattern.

## New concepts (3)

1. **File descriptors, `open` / `close`** — an `int` handle the kernel
   gives you when you `open(path, flags, mode)`. Flags choose behaviour
   (`O_RDONLY` / `O_RDWR`, `O_CREAT`, `O_APPEND`, `O_TRUNC`); mode is
   the permission bits on creation (`0644`). On failure, `open` returns
   `-1` and sets `errno`. Descriptors are a resource — every successful
   `open` needs a matching `close`, same as `malloc`/`free`.
   - *Go analogue:* `os.OpenFile(path, flag, perm)` → `*os.File`. Go wraps
     the fd in a struct with a GC finalizer; in C you own the `int`.

2. **Positional I/O: `pread` / `pwrite` / `fsync`** — `pread(fd, buf, n,
   off)` reads `n` bytes from the file at offset `off` **without moving
   the file pointer**; `pwrite` writes the same way. Safe for concurrent
   reads on the same fd and natural for random-access formats like B+
   trees. `fsync(fd)` asks the kernel to push all buffered writes to
   actual disk — essential for durability.
   - *Go analogue:* `file.ReadAt` / `file.WriteAt` / `file.Sync`. Same
     semantics, different names.

3. **Opaque handle pattern** — declare the struct name in the header but
   leave the body in the `.c`. Callers hold `bytefile_t *` but cannot
   reach inside — only the API functions can. This is how "hide the
   representation" works in C; it's the closest analogue to Go's
   lowercase-field encapsulation.
   - *Go analogue:* a struct type exposed only via its exported
     methods, with all fields unexported.

## Prerequisites
- Step 04 (error codes, `goto cleanup`, `.h`/`.c` split).
- A little comfort with `man` pages — you'll consult `man 2 open` etc.

## Exercise

Build a POSIX file wrapper across:

- `common.h` — add `KV_ERR_IO` (read/write/open/sync failed).
- `bytefile.h` — **opaque** `bytefile_t` typedef and the public API:
  ```c
  kv_result_t bytefile_open(const char *path, bytefile_t **out);
  void        bytefile_close(bytefile_t *f);

  kv_result_t bytefile_write_at(bytefile_t *f,
                                uint64_t offset,
                                const void *buf,
                                size_t len);

  kv_result_t bytefile_read_at (bytefile_t *f,
                                uint64_t offset,
                                void *buf,
                                size_t len);

  kv_result_t bytefile_size(bytefile_t *f, uint64_t *size_out);
  kv_result_t bytefile_sync(bytefile_t *f);
  ```
- `bytefile.c` — defines `struct bytefile { int fd; }` and implements
  the API with `open`, `pread`, `pwrite`, `fsync`, `fstat`, `close`.
  Handle **short reads/writes** by looping until the full `len` has
  been transferred or an error occurs.
- `bytefile_test.c` — write, read, size, sync, reopen, read again,
  verify persistence across close/reopen.
- `Makefile` — same shape as step 04 (`run` / `asan` / `leaks` / `check`).

## Tasks

- [x] Extend `common.h` with `KV_ERR_IO` and update `kv_strerror`
- [x] `bytefile.h` — opaque typedef + function prototypes + include guard
- [x] `bytefile.c` — `struct bytefile` definition and `bytefile_open`
      (creates file if missing, opens O_RDWR, stores `fd`)
- [x] `bytefile_close` closes fd and frees struct (NULL-safe)
- [x] `bytefile_write_at` loops over `pwrite` to handle short writes
- [x] `bytefile_read_at` loops over `pread`; reports EOF on premature end
- [x] `bytefile_size` uses `fstat`
- [x] `bytefile_sync` uses `fsync`
- [ ] `bytefile_test.c` — write, read, size, sync, reopen, re-read
- [ ] `Makefile` builds it, `make check` passes (ASan + leaks)

## Done when
- `make run` prints `all tests passed`
- `make check` is clean (no ASan or leaks output)
- Zero compiler warnings
- Any `open` that succeeded has a matching `close` on every path

## Key points to internalise
- **A file descriptor is a resource like a `malloc` block.** If you
  forget `close`, you leak kernel resources (and eventually hit
  `EMFILE`). Use the `goto fail` pattern if multiple resources are
  in play.
- **Short reads/writes are real.** `pread`/`pwrite` can transfer fewer
  bytes than you asked for — especially with signals or large buffers.
  Always loop until `len` is consumed or an error is returned.
- **Check `errno` only after a failed call.** If `pread` returns `-1`,
  `errno` tells you why (`EIO`, `EINTR`, ...). If it returned a valid
  count, don't touch `errno`.
- **`fsync` is not free.** It forces a disk flush. Do it when you need
  durability (after a transaction); don't do it per-byte.

## References
- `man 2 open` / `man 2 close` / `man 2 pread` / `man 2 pwrite`
- `man 2 fsync` / `man 2 fstat`
- `<fcntl.h>` for flags, `<unistd.h>` for the syscall prototypes,
  `<sys/stat.h>` for `fstat` and permission macros
- [`reference/src/pager.c`](../../../reference/src/pager.c) — the next
  layer you'll build in step 06; peek at its file-open idioms but not
  its pager logic
- [`.claude/rules/c-style.md`](../../../.claude/rules/c-style.md)
