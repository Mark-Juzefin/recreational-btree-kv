#include "btree.h"

#include <stdlib.h>

struct btree {
    pager_t *pager;
};

/* In-memory view of one node. Both `values` and `children` are always
   allocated; only the one matching `type` is used. Fits easily in one cache
   line per node load. */
typedef struct {
    uint32_t page_id;
    uint8_t  type;
    uint16_t num_keys;
    uint32_t next_leaf;                       /* leaves only */
    int64_t  keys[BTREE_MAX_KEYS];
    int64_t  values[BTREE_MAX_KEYS];          /* leaves only */
    uint32_t children[BTREE_MAX_KEYS + 1];    /* internals only */
} node_t;

/* Explicit on-disk offsets. Layout is type-tag (1) + num_keys (u16) at 2 +
   next_leaf (u32) at 4 + keys at 16 + values/children at 136/256. */
#define OFF_TYPE      0
#define OFF_NUM_KEYS  2
#define OFF_NEXT_LEAF 4
#define OFF_KEYS      16
#define OFF_VALUES    136
#define OFF_CHILDREN  256

static void node_serialize(const node_t *n, uint8_t *buf) {
    memset(buf, 0, KV_PAGE_SIZE);
    buf[OFF_TYPE] = n->type;
    memcpy(buf + OFF_NUM_KEYS,  &n->num_keys,  2);
    memcpy(buf + OFF_NEXT_LEAF, &n->next_leaf, 4);
    memcpy(buf + OFF_KEYS,      n->keys,       sizeof(n->keys));
    if (n->type == BTREE_NODE_LEAF) {
        memcpy(buf + OFF_VALUES, n->values, sizeof(n->values));
    } else {
        memcpy(buf + OFF_CHILDREN, n->children, sizeof(n->children));
    }
}

static void node_deserialize(node_t *n, const uint8_t *buf, uint32_t page_id) {
    memset(n, 0, sizeof(*n));
    n->page_id = page_id;
    n->type = buf[OFF_TYPE];
    memcpy(&n->num_keys,  buf + OFF_NUM_KEYS,  2);
    memcpy(&n->next_leaf, buf + OFF_NEXT_LEAF, 4);
    memcpy(n->keys,       buf + OFF_KEYS,      sizeof(n->keys));
    if (n->type == BTREE_NODE_LEAF) {
        memcpy(n->values, buf + OFF_VALUES, sizeof(n->values));
    } else {
        memcpy(n->children, buf + OFF_CHILDREN, sizeof(n->children));
    }
}

static kv_result_t node_load(btree_t *bt, uint32_t page_id, node_t *n) {
    uint8_t buf[KV_PAGE_SIZE];
    kv_result_t r = pager_read(bt->pager, page_id, buf);
    if (r != KV_OK) return r;
    node_deserialize(n, buf, page_id);
    if (n->type != BTREE_NODE_LEAF && n->type != BTREE_NODE_INTERNAL)
        return KV_ERR_CORRUPT;
    if (n->num_keys > BTREE_MAX_KEYS) return KV_ERR_CORRUPT;
    return KV_OK;
}

static kv_result_t node_save(btree_t *bt, const node_t *n) {
    uint8_t buf[KV_PAGE_SIZE];
    node_serialize(n, buf);
    return pager_write(bt->pager, n->page_id, buf);
}

static kv_result_t node_alloc(btree_t *bt, uint8_t type, node_t *n) {
    uint32_t id;
    kv_result_t r = pager_alloc(bt->pager, &id);
    if (r != KV_OK) return r;
    memset(n, 0, sizeof(*n));
    n->page_id = id;
    n->type    = type;
    return KV_OK;
}

/* Binary search. Sets *found = 1 and returns the matching index if the key
   exists; otherwise returns the insertion position and *found = 0. */
static uint16_t node_find(const node_t *n, int64_t key, int *found) {
    *found = 0;
    int lo = 0, hi = (int)n->num_keys - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (n->keys[mid] == key) { *found = 1; return (uint16_t)mid; }
        if (n->keys[mid] <  key)  lo = mid + 1;
        else                      hi = mid - 1;
    }
    return (uint16_t)lo;
}

/* ---------- open / close / get ---------- */

kv_result_t btree_open(pager_t *pager, btree_t **out) {
    if (!pager || !out) return KV_ERR_ARG;
    btree_t *bt = calloc(1, sizeof(*bt));
    if (!bt) return KV_ERR_NOMEM;
    bt->pager = pager;
    *out = bt;
    return KV_OK;
}

void btree_close(btree_t *bt) { free(bt); }

kv_result_t btree_get(btree_t *bt, int64_t key, int64_t *value_out) {
    if (!bt || !value_out) return KV_ERR_ARG;
    uint32_t root = pager_root(bt->pager);
    if (root == 0) return KV_ERR_NOT_FOUND;

    uint32_t pid = root;
    node_t n;
    for (;;) {
        kv_result_t r = node_load(bt, pid, &n);
        if (r != KV_OK) return r;
        int found;
        uint16_t i = node_find(&n, key, &found);
        if (n.type == BTREE_NODE_LEAF) {
            if (found) { *value_out = n.values[i]; return KV_OK; }
            return KV_ERR_NOT_FOUND;
        }
        pid = n.children[found ? i + 1 : i];
    }
}

/* ---------- insert ---------- */

typedef struct {
    int      split;
    int64_t  key;
    uint32_t right;
} split_t;

static kv_result_t leaf_insert_nosplit(btree_t *bt, node_t *n, uint16_t i,
                                       int64_t key, int64_t value) {
    for (uint16_t j = n->num_keys; j > i; j--) {
        n->keys[j]   = n->keys[j - 1];
        n->values[j] = n->values[j - 1];
    }
    n->keys[i]   = key;
    n->values[i] = value;
    n->num_keys++;
    return node_save(bt, n);
}

static kv_result_t leaf_insert_split(btree_t *bt, node_t *n, uint16_t i,
                                     int64_t key, int64_t value, split_t *sp) {
    int64_t keys[BTREE_MAX_KEYS + 1];
    int64_t vals[BTREE_MAX_KEYS + 1];
    for (uint16_t j = 0; j < i; j++)            { keys[j] = n->keys[j];     vals[j] = n->values[j]; }
    keys[i] = key; vals[i] = value;
    for (uint16_t j = i; j < n->num_keys; j++)  { keys[j+1] = n->keys[j];   vals[j+1] = n->values[j]; }

    uint16_t total = (uint16_t)(n->num_keys + 1);
    uint16_t split = (uint16_t)(total / 2);      /* left [0,split), right [split,total) */

    node_t right;
    kv_result_t r = node_alloc(bt, BTREE_NODE_LEAF, &right);
    if (r != KV_OK) return r;

    right.num_keys  = (uint16_t)(total - split);
    right.next_leaf = n->next_leaf;
    for (uint16_t j = 0; j < right.num_keys; j++) {
        right.keys[j]   = keys[split + j];
        right.values[j] = vals[split + j];
    }

    n->num_keys  = split;
    n->next_leaf = right.page_id;
    for (uint16_t j = 0; j < split; j++) {
        n->keys[j]   = keys[j];
        n->values[j] = vals[j];
    }

    if ((r = node_save(bt, &right)) != KV_OK) return r;
    if ((r = node_save(bt, n))      != KV_OK) return r;

    sp->split = 1;
    sp->key   = right.keys[0];    /* B+ separator = first key of right leaf */
    sp->right = right.page_id;
    return KV_OK;
}

static kv_result_t internal_insert_nosplit(btree_t *bt, node_t *n, uint16_t i,
                                           int64_t key, uint32_t right_child) {
    for (uint16_t j = n->num_keys; j > i; j--)
        n->keys[j] = n->keys[j - 1];
    for (uint16_t j = (uint16_t)(n->num_keys + 1); j > i + 1; j--)
        n->children[j] = n->children[j - 1];
    n->keys[i]         = key;
    n->children[i + 1] = right_child;
    n->num_keys++;
    return node_save(bt, n);
}

static kv_result_t internal_insert_split(btree_t *bt, node_t *n, uint16_t i,
                                         int64_t key, uint32_t right_child,
                                         split_t *sp) {
    int64_t  keys[BTREE_MAX_KEYS + 1];
    uint32_t children[BTREE_MAX_KEYS + 2];
    for (uint16_t j = 0; j < i; j++)           keys[j] = n->keys[j];
    keys[i] = key;
    for (uint16_t j = i; j < n->num_keys; j++) keys[j + 1] = n->keys[j];
    for (uint16_t j = 0; j <= i; j++)          children[j] = n->children[j];
    children[i + 1] = right_child;
    for (uint16_t j = (uint16_t)(i + 1); j <= n->num_keys; j++)
        children[j + 1] = n->children[j];

    uint16_t total  = (uint16_t)(n->num_keys + 1);
    uint16_t mid    = (uint16_t)(total / 2);
    int64_t  up_key = keys[mid];

    node_t right;
    kv_result_t r = node_alloc(bt, BTREE_NODE_INTERNAL, &right);
    if (r != KV_OK) return r;

    right.num_keys = (uint16_t)(total - mid - 1);
    for (uint16_t j = 0; j < right.num_keys; j++)
        right.keys[j] = keys[mid + 1 + j];
    for (uint16_t j = 0; j <= right.num_keys; j++)
        right.children[j] = children[mid + 1 + j];

    n->num_keys = mid;
    for (uint16_t j = 0; j < mid; j++)  n->keys[j]     = keys[j];
    for (uint16_t j = 0; j <= mid; j++) n->children[j] = children[j];

    if ((r = node_save(bt, &right)) != KV_OK) return r;
    if ((r = node_save(bt, n))      != KV_OK) return r;

    sp->split = 1;
    sp->key   = up_key;
    sp->right = right.page_id;
    return KV_OK;
}

static kv_result_t insert_rec(btree_t *bt, uint32_t page_id,
                              int64_t key, int64_t value, split_t *sp) {
    sp->split = 0;
    node_t n;
    kv_result_t r = node_load(bt, page_id, &n);
    if (r != KV_OK) return r;

    int found;
    uint16_t i = node_find(&n, key, &found);

    if (n.type == BTREE_NODE_LEAF) {
        if (found) return KV_ERR_EXISTS;
        if (n.num_keys < BTREE_MAX_KEYS)
            return leaf_insert_nosplit(bt, &n, i, key, value);
        return leaf_insert_split(bt, &n, i, key, value, sp);
    }

    uint16_t ci = (uint16_t)(found ? i + 1 : i);
    split_t cs = {0};
    r = insert_rec(bt, n.children[ci], key, value, &cs);
    if (r != KV_OK) return r;
    if (!cs.split)  return KV_OK;

    if (n.num_keys < BTREE_MAX_KEYS)
        return internal_insert_nosplit(bt, &n, ci, cs.key, cs.right);
    return internal_insert_split(bt, &n, ci, cs.key, cs.right, sp);
}

kv_result_t btree_insert(btree_t *bt, int64_t key, int64_t value) {
    if (!bt) return KV_ERR_ARG;

    uint32_t root = pager_root(bt->pager);
    if (root == 0) {
        node_t leaf;
        kv_result_t r = node_alloc(bt, BTREE_NODE_LEAF, &leaf);
        if (r != KV_OK) return r;
        leaf.num_keys  = 1;
        leaf.keys[0]   = key;
        leaf.values[0] = value;
        if ((r = node_save(bt, &leaf)) != KV_OK) return r;
        pager_set_root(bt->pager, leaf.page_id);
        return pager_sync(bt->pager);
    }

    split_t sp = {0};
    kv_result_t r = insert_rec(bt, root, key, value, &sp);
    if (r != KV_OK) return r;

    if (sp.split) {
        node_t new_root;
        r = node_alloc(bt, BTREE_NODE_INTERNAL, &new_root);
        if (r != KV_OK) return r;
        new_root.num_keys    = 1;
        new_root.keys[0]     = sp.key;
        new_root.children[0] = root;
        new_root.children[1] = sp.right;
        if ((r = node_save(bt, &new_root)) != KV_OK) return r;
        pager_set_root(bt->pager, new_root.page_id);
    }
    return pager_sync(bt->pager);
}

/* ---------- delete (no rebalancing; see header for rationale) ---------- */

static kv_result_t delete_rec(btree_t *bt, uint32_t page_id, int64_t key) {
    node_t n;
    kv_result_t r = node_load(bt, page_id, &n);
    if (r != KV_OK) return r;

    int found;
    uint16_t i = node_find(&n, key, &found);

    if (n.type == BTREE_NODE_LEAF) {
        if (!found) return KV_ERR_NOT_FOUND;
        for (uint16_t j = i; j + 1 < n.num_keys; j++) {
            n.keys[j]   = n.keys[j + 1];
            n.values[j] = n.values[j + 1];
        }
        n.num_keys--;
        return node_save(bt, &n);
    }
    uint16_t ci = (uint16_t)(found ? i + 1 : i);
    return delete_rec(bt, n.children[ci], key);
}

kv_result_t btree_delete(btree_t *bt, int64_t key) {
    if (!bt) return KV_ERR_ARG;
    uint32_t root = pager_root(bt->pager);
    if (root == 0) return KV_ERR_NOT_FOUND;
    kv_result_t r = delete_rec(bt, root, key);
    if (r != KV_OK) return r;
    return pager_sync(bt->pager);
}

/* ---------- iterator ---------- */

kv_result_t btree_iter_first(btree_t *bt, btree_iter_t *it) {
    if (!bt || !it) return KV_ERR_ARG;
    it->bt = bt; it->leaf_id = 0; it->idx = 0;

    uint32_t root = pager_root(bt->pager);
    if (root == 0) return KV_OK;

    uint32_t pid = root;
    node_t n;
    for (;;) {
        kv_result_t r = node_load(bt, pid, &n);
        if (r != KV_OK) return r;
        if (n.type == BTREE_NODE_LEAF) { it->leaf_id = pid; return KV_OK; }
        pid = n.children[0];
    }
}

kv_result_t btree_iter_seek(btree_t *bt, int64_t start_key, btree_iter_t *it) {
    if (!bt || !it) return KV_ERR_ARG;
    it->bt = bt; it->leaf_id = 0; it->idx = 0;

    uint32_t root = pager_root(bt->pager);
    if (root == 0) return KV_OK;

    uint32_t pid = root;
    node_t n;
    for (;;) {
        kv_result_t r = node_load(bt, pid, &n);
        if (r != KV_OK) return r;
        int found;
        uint16_t i = node_find(&n, start_key, &found);
        if (n.type == BTREE_NODE_LEAF) {
            it->leaf_id = pid;
            it->idx     = i;
            return KV_OK;
        }
        pid = n.children[found ? i + 1 : i];
    }
}

kv_result_t btree_iter_next(btree_iter_t *it, int64_t *key_out, int64_t *value_out) {
    if (!it || !key_out || !value_out) return KV_ERR_ARG;
    if (it->leaf_id == 0) return KV_ERR_EOF;

    node_t n;
    kv_result_t r = node_load(it->bt, it->leaf_id, &n);
    if (r != KV_OK) return r;

    while (it->idx >= n.num_keys) {
        uint32_t next = n.next_leaf;
        if (next == 0) { it->leaf_id = 0; return KV_ERR_EOF; }
        it->leaf_id = next;
        it->idx     = 0;
        if ((r = node_load(it->bt, it->leaf_id, &n)) != KV_OK) return r;
    }

    *key_out   = n.keys[it->idx];
    *value_out = n.values[it->idx];
    it->idx++;
    return KV_OK;
}

/* ---------- debug dump ---------- */

static void dump_rec(btree_t *bt, uint32_t page_id, int depth, FILE *out) {
    node_t n;
    if (node_load(bt, page_id, &n) != KV_OK) return;
    for (int d = 0; d < depth; d++) fputs("  ", out);
    if (n.type == BTREE_NODE_LEAF) {
        fprintf(out, "Leaf  #%u (%u):", page_id, n.num_keys);
        for (uint16_t i = 0; i < n.num_keys; i++)
            fprintf(out, " %lld=%lld", (long long)n.keys[i], (long long)n.values[i]);
        fprintf(out, "  next=%u\n", n.next_leaf);
    } else {
        fprintf(out, "Node  #%u (%u):", page_id, n.num_keys);
        for (uint16_t i = 0; i < n.num_keys; i++)
            fprintf(out, " [%u] %lld", n.children[i], (long long)n.keys[i]);
        fprintf(out, " [%u]\n", n.children[n.num_keys]);
        for (uint16_t i = 0; i <= n.num_keys; i++)
            dump_rec(bt, n.children[i], depth + 1, out);
    }
}

void btree_dump(btree_t *bt, FILE *out) {
    uint32_t root = pager_root(bt->pager);
    if (root == 0) { fprintf(out, "(empty tree)\n"); return; }
    dump_rec(bt, root, 0, out);
}
