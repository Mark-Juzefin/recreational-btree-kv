#include "db.h"

#include "pager.h"
#include "table.h"

#include <stdlib.h>

struct db {
    pager_t *pager;
    btree_t *btree;
    table_t *table;
};

static char *join_suffix(const char *prefix, const char *suffix) {
    size_t lp = strlen(prefix), ls = strlen(suffix);
    char *p = malloc(lp + ls + 1);
    if (!p) return NULL;
    memcpy(p,       prefix, lp);
    memcpy(p + lp,  suffix, ls + 1);
    return p;
}

kv_result_t db_open(const char *prefix, db_t **out) {
    if (!prefix || !out) return KV_ERR_ARG;

    db_t *db = calloc(1, sizeof(*db));
    if (!db) return KV_ERR_NOMEM;

    char *idx_path  = join_suffix(prefix, ".idx");
    char *data_path = join_suffix(prefix, ".data");
    if (!idx_path || !data_path) {
        free(idx_path); free(data_path); free(db);
        return KV_ERR_NOMEM;
    }

    kv_result_t r;
    if ((r = pager_open(idx_path,  &db->pager)) != KV_OK) goto fail;
    if ((r = btree_open(db->pager, &db->btree)) != KV_OK) goto fail;
    if ((r = table_open(data_path, &db->table)) != KV_OK) goto fail;

    free(idx_path); free(data_path);
    *out = db;
    return KV_OK;

fail:
    if (db->btree) btree_close(db->btree);
    if (db->pager) pager_close(db->pager);
    if (db->table) table_close(db->table);
    free(idx_path); free(data_path); free(db);
    return r;
}

void db_close(db_t *db) {
    if (!db) return;
    if (db->btree) btree_close(db->btree);
    if (db->pager) pager_close(db->pager);
    if (db->table) table_close(db->table);
    free(db);
}

kv_result_t db_put(db_t *db, int64_t key, int64_t value) {
    if (!db) return KV_ERR_ARG;

    /* Writing to the table first means a crash between steps leaves an
       orphan record — wasted space but no incorrect reads. Index-first would
       allow a read to find a key pointing at a record that doesn't exist. */
    uint64_t off;
    kv_result_t r = table_append(db->table, key, value, &off);
    if (r != KV_OK) return r;

    r = btree_insert(db->btree, key, (int64_t)off);
    if (r != KV_OK) {
        table_mark_deleted(db->table, off);
        return r;
    }
    return table_sync(db->table);
}

kv_result_t db_get(db_t *db, int64_t key, int64_t *value_out) {
    if (!db || !value_out) return KV_ERR_ARG;

    int64_t off;
    kv_result_t r = btree_get(db->btree, key, &off);
    if (r != KV_OK) return r;

    table_record_t rec;
    if ((r = table_read(db->table, (uint64_t)off, &rec)) != KV_OK) return r;
    if (rec.deleted) return KV_ERR_NOT_FOUND;

    *value_out = rec.value;
    return KV_OK;
}

kv_result_t db_del(db_t *db, int64_t key) {
    if (!db) return KV_ERR_ARG;

    int64_t off;
    kv_result_t r = btree_get(db->btree, key, &off);
    if (r != KV_OK) return r;

    if ((r = table_mark_deleted(db->table, (uint64_t)off)) != KV_OK) return r;
    if ((r = btree_delete(db->btree, key))                 != KV_OK) return r;
    return table_sync(db->table);
}

kv_result_t db_scan_first(db_t *db, db_iter_t *it) {
    if (!db || !it) return KV_ERR_ARG;
    it->db = db;
    return btree_iter_first(db->btree, &it->iter);
}

kv_result_t db_scan_seek(db_t *db, int64_t start_key, db_iter_t *it) {
    if (!db || !it) return KV_ERR_ARG;
    it->db = db;
    return btree_iter_seek(db->btree, start_key, &it->iter);
}

kv_result_t db_scan_next(db_iter_t *it, int64_t *key_out, int64_t *value_out) {
    if (!it || !key_out || !value_out) return KV_ERR_ARG;

    for (;;) {
        int64_t k, off;
        kv_result_t r = btree_iter_next(&it->iter, &k, &off);
        if (r != KV_OK) return r;   /* KV_ERR_EOF ends iteration */

        table_record_t rec;
        r = table_read(it->db->table, (uint64_t)off, &rec);
        if (r != KV_OK) return r;
        if (rec.deleted) continue;  /* skip tombstones */

        *key_out   = k;
        *value_out = rec.value;
        return KV_OK;
    }
}

void db_dump_tree(db_t *db, FILE *out) {
    if (!db) return;
    btree_dump(db->btree, out);
}
