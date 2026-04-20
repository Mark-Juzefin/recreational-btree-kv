#include "bytefile.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    const char *path = "/tmp/bytefile_smoke.bin";
    unlink(path);

    bytefile_t *f = NULL;
    kv_result_t rc_open = bytefile_open(path, &f);
    printf("open rc=%d (%s), handle=%p\n",
           rc_open, kv_strerror(rc_open), (void *)f);


    const char *words[] = {
        "slava", "ukraini", "heroyam", "slava",
        "hello", "world", "btree", "kv",
    };
    size_t n = sizeof(words) / sizeof(words[0]);

    typedef struct {
        uint64_t offset;
        size_t   len;
    } record_t;

    record_t records[sizeof(words) / sizeof(words[0])];

    uint64_t offset = 0;
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(words[i]);
        kv_result_t rc = bytefile_write_at(f, offset, words[i], len);
        printf("write[%zu] offset=%llu len=%zu rc=%d (%s)\n",
               i, (unsigned long long)offset, len, rc, kv_strerror(rc));

        records[i] = (record_t){ .offset = offset, .len = len };

        offset += len;
    }

    uint64_t size_on_disk = 0;
    kv_result_t rc_size = bytefile_size(f, &size_on_disk);
    printf("size rc=%d (%s): expected=%llu, on_disk=%llu\n",
           rc_size, kv_strerror(rc_size),
           (unsigned long long)offset,
           (unsigned long long)size_on_disk);

    for (size_t i = 0; i < n; i++) {
        char buf[32] = {0};
        kv_result_t rc = bytefile_read_at(f, records[i].offset, buf, records[i].len);
        printf("read[%zu] offset=%llu len=%zu rc=%d (%s) -> \"%.*s\"\n",
               i,
               (unsigned long long)records[i].offset,
               records[i].len,
               rc, kv_strerror(rc),
               (int)records[i].len, buf);
    }

    bytefile_close(f);
    printf("close ok\n");
    return 0;
}
