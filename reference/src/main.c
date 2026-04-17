#include "db.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *f) {
    fputs(
        "usage: kvdb <prefix> <cmd> [args]\n"
        "\n"
        "commands:\n"
        "  put <key> <value>    insert key/value (both int64)\n"
        "  get <key>            print value for key\n"
        "  del <key>            delete key\n"
        "  scan [start]         print all (key, value) pairs, optionally from a start key\n"
        "  dump                 print raw B+ tree structure (debug)\n"
        "  help                 this message\n"
        "\n"
        "Files created: <prefix>.idx (B+ tree), <prefix>.data (records).\n",
        f);
}

static int parse_i64(const char *s, int64_t *out) {
    errno = 0;
    char *end = NULL;
    long long v = strtoll(s, &end, 10);
    if (errno != 0 || !end || *end != '\0' || end == s) return -1;
    *out = (int64_t)v;
    return 0;
}

static int die(const char *msg, int code) {
    fprintf(stderr, "error: %s (%s)\n", msg, kv_strerror(code));
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 3) { usage(stderr); return 2; }

    const char *prefix = argv[1];
    const char *cmd    = argv[2];

    if (strcmp(cmd, "help") == 0) { usage(stdout); return 0; }

    db_t *db;
    kv_result_t r = db_open(prefix, &db);
    if (r != KV_OK) return die("cannot open db", r);

    int rc = 0;

    if (strcmp(cmd, "put") == 0) {
        if (argc != 5) { usage(stderr); rc = 2; goto done; }
        int64_t k, v;
        if (parse_i64(argv[3], &k) || parse_i64(argv[4], &v)) {
            fputs("error: key/value must be int64\n", stderr);
            rc = 2; goto done;
        }
        r = db_put(db, k, v);
        if (r != KV_OK) { rc = die("put failed", r); goto done; }
        printf("ok\n");

    } else if (strcmp(cmd, "get") == 0) {
        if (argc != 4) { usage(stderr); rc = 2; goto done; }
        int64_t k, v;
        if (parse_i64(argv[3], &k)) {
            fputs("error: key must be int64\n", stderr);
            rc = 2; goto done;
        }
        r = db_get(db, k, &v);
        if (r == KV_ERR_NOT_FOUND) { printf("(not found)\n"); rc = 1; goto done; }
        if (r != KV_OK)            { rc = die("get failed", r); goto done; }
        printf("%lld\n", (long long)v);

    } else if (strcmp(cmd, "del") == 0) {
        if (argc != 4) { usage(stderr); rc = 2; goto done; }
        int64_t k;
        if (parse_i64(argv[3], &k)) {
            fputs("error: key must be int64\n", stderr);
            rc = 2; goto done;
        }
        r = db_del(db, k);
        if (r == KV_ERR_NOT_FOUND) { printf("(not found)\n"); rc = 1; goto done; }
        if (r != KV_OK)            { rc = die("del failed", r); goto done; }
        printf("ok\n");

    } else if (strcmp(cmd, "scan") == 0) {
        db_iter_t it;
        if (argc == 3) {
            r = db_scan_first(db, &it);
        } else if (argc == 4) {
            int64_t start;
            if (parse_i64(argv[3], &start)) {
                fputs("error: start must be int64\n", stderr);
                rc = 2; goto done;
            }
            r = db_scan_seek(db, start, &it);
        } else { usage(stderr); rc = 2; goto done; }
        if (r != KV_OK) { rc = die("scan init failed", r); goto done; }

        uint64_t n = 0;
        int64_t k, v;
        while ((r = db_scan_next(&it, &k, &v)) == KV_OK) {
            printf("%lld\t%lld\n", (long long)k, (long long)v);
            n++;
        }
        if (r != KV_ERR_EOF) { rc = die("scan failed", r); goto done; }
        fprintf(stderr, "(%llu %s)\n", (unsigned long long)n, n == 1 ? "row" : "rows");

    } else if (strcmp(cmd, "dump") == 0) {
        db_dump_tree(db, stdout);

    } else {
        fprintf(stderr, "error: unknown command '%s'\n", cmd);
        usage(stderr);
        rc = 2;
    }

done:
    db_close(db);
    return rc;
}
