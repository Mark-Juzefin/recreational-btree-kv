#ifndef KV_BTREE_H
#define KV_BTREE_H

#include "common.h"
#include "pager.h"

/* Classic B+ tree over 4KB pages.

   - Leaves hold sorted (key -> value) pairs and form a singly-linked list
     via `next_leaf` for range scans.
   - Internal nodes hold (num_keys + 1) child page ids separated by `num_keys`
     routing keys. Routing rule: keys < K go left, keys >= K go right.
   - All keys are `int64_t`. Duplicates are rejected (KV_ERR_EXISTS).
   - Delete in this version does NOT rebalance; nodes may become under-filled.
     Correctness is preserved but the tree grows looser over time. */

#define BTREE_MAX_KEYS      15
#define BTREE_NODE_LEAF      1
#define BTREE_NODE_INTERNAL  2

typedef struct btree btree_t;

kv_result_t btree_open(pager_t *pager, btree_t **out);
void        btree_close(btree_t *bt);

kv_result_t btree_insert(btree_t *bt, int64_t key, int64_t value);
kv_result_t btree_get(btree_t *bt, int64_t key, int64_t *value_out);
kv_result_t btree_delete(btree_t *bt, int64_t key);

typedef struct {
    btree_t *bt;
    uint32_t leaf_id;  /* 0 = exhausted */
    uint16_t idx;
} btree_iter_t;

kv_result_t btree_iter_first(btree_t *bt, btree_iter_t *it);
kv_result_t btree_iter_seek(btree_t *bt, int64_t start_key, btree_iter_t *it);
kv_result_t btree_iter_next(btree_iter_t *it, int64_t *key_out, int64_t *value_out);

void btree_dump(btree_t *bt, FILE *out);

#endif
