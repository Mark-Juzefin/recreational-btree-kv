#ifndef KV_TABLE_H
#define KV_TABLE_H

#include "common.h"

/* Append-only fixed-size record store.

   Each record: [uint8 flags][int64 key][int64 value] = 17 bytes.
   The table is addressed by absolute byte offset, which is stored in the
   B-tree leaf as the "value" for a given key. */

#define TABLE_RECORD_SIZE 17

typedef struct table table_t;

typedef struct {
    int64_t key;
    int64_t value;
    uint8_t deleted;
} table_record_t;

kv_result_t table_open(const char *path, table_t **out);
void        table_close(table_t *t);

kv_result_t table_append(table_t *t, int64_t key, int64_t value, uint64_t *offset_out);
kv_result_t table_read(const table_t *t, uint64_t offset, table_record_t *rec);
kv_result_t table_mark_deleted(table_t *t, uint64_t offset);
kv_result_t table_sync(table_t *t);

uint64_t table_size(const table_t *t);

#endif
