# Roadmap — on-disk key-value store

## End result

A small but real key-value database in C: `put key value` / `get key` from a CLI,
with the data persisted in a single file and surviving process restarts. The
index is a **B+ tree stored on disk as fixed-size pages**; lookups and inserts
touch only the pages along a root→leaf path, so it stays fast as the data grows.

Concretely, when finished:

- One database file holds a page-0 header (magic/version/root) plus B+ tree nodes,
  one node per 4 KB page.
- `put` inserts/overwrites a key, splitting nodes and growing the tree as needed,
  and writes the touched pages back to disk.
- `get` walks the tree from the on-disk root down to a leaf and returns the value.
- Close the process, reopen the file, and every key is still there.

## Foundation (done)

`bytefile.{c,h}` (repo root) — `bytefile_t`: a thin wrapper over a POSIX file
descriptor with positional byte I/O (`bytefile_read_at` / `bytefile_write_at`),
`bytefile_size`, and `bytefile_sync` for durability. Opaque handle, short-I/O
loops, error codes. **This is the base layer everything below builds on** — the
only thing that touches the raw file. Reuse it as-is; never open a second fd.

## Architecture (bottom → top)

```
CLI            put/get commands
  └─ db        open/close/put/get; owns the tree + header
       └─ btree  search / insert / split over nodes
            └─ node  (de)serialize a tree node ⇄ a 4 KB page
                 └─ pager  page-addressed I/O + file header
                      └─ bytefile  positional byte I/O  ← DONE
```

Each layer only calls the one below it.

## Tasks

Built from the end result, bottom-up. Order is dependency order, not fixed
"steps" — pull the next unblocked task. Each task: write the code + its own tests,
`make check` clean (ASan + leaks), zero warnings.

### Pager — pages over bytefile
- [ ] `PAGE_SIZE = 4096` constant; file length is always a whole number of pages.
- [ ] Page-0 header layout: pick byte offsets for `magic`, `version`,
      `page_count`, `root_page_no`. Write the layout as a comment table.
- [ ] Opaque `pager_t` API: open, close, read one page, write one page, allocate
      a new page, get/set the root page number, sync.
- [ ] Open logic: empty file → write a fresh header; existing file → read and
      validate magic + version, load `page_count` and `root_page_no`.
- [ ] Header encode/decode helpers (explicit per-offset, fixed-width types).
- [ ] `pager_alloc_page` bumps `page_count` and persists the header; data pages
      start at 1 (page 0 is metadata).

### Node — tree node ⇄ page bytes
- [ ] Node-on-page layout: offsets for `node_type` (leaf/internal), `key_count`,
      the keys array, and either values (leaf) or child page numbers (internal).
      A full node must fit in `PAGE_SIZE` — this fixes the tree order.
- [ ] `serialize(node, page_buf)` / `deserialize(page_buf, node)` — symmetric,
      field-by-field. Round-trip test them in isolation first.

### B+ tree — the algorithm
- [ ] Value type decision: start with `int64_t` values stored inline in leaves.
- [ ] `search(key)`: start at `root_page_no`, at each internal node pick the child
      by comparing separators, read that child's page, descend to a leaf.
- [ ] Leaf insert (sorted), with duplicate-key overwrite policy.
- [ ] Leaf split on overflow: split into two pages, copy a separator up.
- [ ] Internal insert + split: median moves up and leaves the node.
- [ ] Split propagation: a leaf split can cascade up to the root.
- [ ] Root split: allocate a new root page, update `root_page_no` in the header,
      persist it. Only place the tree grows in height.

### db + CLI — tie it together
- [ ] Opaque `db_t`: open (opens pager, reads root), close, put, get.
- [ ] Durability boundary: decide when to `sync` (per put = safe/slow, per batch
      = faster/weaker).
- [ ] CLI `main`: `put <key> <val>` / `get <key>` against a database file.
- [ ] End-to-end persistence test: put a batch across a multi-level tree, close,
      reopen, get every value back. Overwrite an existing key.

## Stretch (optional, after the core works)
- Append-only record log: leaves store an offset into a separate data file instead
  of an inline value — lets values be arbitrary bytes, not just `int64_t`.
- Delete + node rebalancing (merge underfull nodes).
- Range scans / iteration over the leaf level.

## Conventions
- Fixed-width types for everything on disk, or files won't port across machines.
- Explicit serialization only — never `memcpy` a whole struct to disk.
- See [`.claude/rules/c-style.md`](../.claude/rules/c-style.md).
