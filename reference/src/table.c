#define _POSIX_C_SOURCE 200809L
#include "table.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

struct table {
    int      fd;
    uint64_t size;
};

static void encode_record(uint8_t buf[TABLE_RECORD_SIZE], const table_record_t *r) {
    buf[0] = r->deleted;
    memcpy(buf + 1, &r->key,   8);
    memcpy(buf + 9, &r->value, 8);
}

static void decode_record(const uint8_t buf[TABLE_RECORD_SIZE], table_record_t *r) {
    r->deleted = buf[0];
    memcpy(&r->key,   buf + 1, 8);
    memcpy(&r->value, buf + 9, 8);
}

kv_result_t table_open(const char *path, table_t **out) {
    if (!path || !out) return KV_ERR_ARG;

    table_t *t = calloc(1, sizeof(*t));
    if (!t) return KV_ERR_NOMEM;

    t->fd = open(path, O_RDWR | O_CREAT, 0644);
    if (t->fd < 0) { free(t); return KV_ERR_IO; }

    struct stat st;
    if (fstat(t->fd, &st) != 0) { close(t->fd); free(t); return KV_ERR_IO; }

    t->size = (uint64_t)st.st_size;
    *out = t;
    return KV_OK;
}

void table_close(table_t *t) {
    if (!t) return;
    fsync(t->fd);
    close(t->fd);
    free(t);
}

kv_result_t table_append(table_t *t, int64_t key, int64_t value, uint64_t *offset_out) {
    if (!t || !offset_out) return KV_ERR_ARG;

    table_record_t r = { .key = key, .value = value, .deleted = 0 };
    uint8_t buf[TABLE_RECORD_SIZE];
    encode_record(buf, &r);

    ssize_t n = pwrite(t->fd, buf, TABLE_RECORD_SIZE, (off_t)t->size);
    if (n != TABLE_RECORD_SIZE) return KV_ERR_IO;

    *offset_out = t->size;
    t->size += TABLE_RECORD_SIZE;
    return KV_OK;
}

kv_result_t table_read(const table_t *t, uint64_t offset, table_record_t *rec) {
    if (!t || !rec)                           return KV_ERR_ARG;
    if (offset + TABLE_RECORD_SIZE > t->size) return KV_ERR_ARG;

    uint8_t buf[TABLE_RECORD_SIZE];
    ssize_t n = pread(t->fd, buf, TABLE_RECORD_SIZE, (off_t)offset);
    if (n != TABLE_RECORD_SIZE) return KV_ERR_IO;

    decode_record(buf, rec);
    return KV_OK;
}

kv_result_t table_mark_deleted(table_t *t, uint64_t offset) {
    if (!t)                                   return KV_ERR_ARG;
    if (offset + TABLE_RECORD_SIZE > t->size) return KV_ERR_ARG;

    uint8_t flag = 1;
    ssize_t n = pwrite(t->fd, &flag, 1, (off_t)offset);
    return n == 1 ? KV_OK : KV_ERR_IO;
}

kv_result_t table_sync(table_t *t) {
    return t && fsync(t->fd) == 0 ? KV_OK : KV_ERR_IO;
}

uint64_t table_size(const table_t *t) { return t ? t->size : 0; }
