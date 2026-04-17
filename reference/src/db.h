#ifndef KV_DB_H
#define KV_DB_H

#include "common.h"
#include "btree.h"

/* Top-level API.

   Uses two files on disk, both named from a common `prefix`:
     - `<prefix>.idx`:  paged B+ tree (index file)
     - `<prefix>.data`: append-only record log (table file)

   The index maps `int64 key -> int64 offset_into_data_file`; dereferencing
   the offset in the table yields the actual value. Splitting the two stores
   is overkill for fixed-size int values, but demonstrates how a real storage
   engine separates its index from its heap. */

typedef struct db db_t;

kv_result_t db_open(const char *prefix, db_t **out);
void        db_close(db_t *db);

kv_result_t db_put(db_t *db, int64_t key, int64_t value);
kv_result_t db_get(db_t *db, int64_t key, int64_t *value_out);
kv_result_t db_del(db_t *db, int64_t key);

typedef struct {
    db_t         *db;
    btree_iter_t  iter;
} db_iter_t;

kv_result_t db_scan_first(db_t *db, db_iter_t *it);
kv_result_t db_scan_seek(db_t *db, int64_t start_key, db_iter_t *it);
kv_result_t db_scan_next(db_iter_t *it, int64_t *key_out, int64_t *value_out);

void db_dump_tree(db_t *db, FILE *out);

#endif
