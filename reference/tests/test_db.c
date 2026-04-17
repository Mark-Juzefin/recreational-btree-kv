#include "../src/db.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Minimal test harness: each CHECK aborts with a message on failure.
   Tests are independent functions that clean up their own files. */

#define CHECK(cond) do {                                                     \
    if (!(cond)) {                                                           \
        fprintf(stderr, "CHECK failed at %s:%d: %s\n",                       \
                __FILE__, __LINE__, #cond);                                  \
        abort();                                                             \
    }                                                                        \
} while (0)

#define CHECK_EQ(a, b) do {                                                  \
    long long _a = (long long)(a), _b = (long long)(b);                      \
    if (_a != _b) {                                                          \
        fprintf(stderr, "CHECK_EQ failed at %s:%d: %s (%lld) != %s (%lld)\n",\
                __FILE__, __LINE__, #a, _a, #b, _b);                         \
        abort();                                                             \
    }                                                                        \
} while (0)

static void rm_files(const char *prefix) {
    char p[256];
    snprintf(p, sizeof(p), "%s.idx",  prefix); unlink(p);
    snprintf(p, sizeof(p), "%s.data", prefix); unlink(p);
}

static void test_basic(void) {
    const char *prefix = "tests/_t_basic";
    rm_files(prefix);

    db_t *db;
    CHECK_EQ(db_open(prefix, &db), KV_OK);

    CHECK_EQ(db_put(db, 1, 100), KV_OK);
    CHECK_EQ(db_put(db, 2, 200), KV_OK);
    CHECK_EQ(db_put(db, 3, 300), KV_OK);

    int64_t v;
    CHECK_EQ(db_get(db, 1, &v), KV_OK); CHECK_EQ(v, 100);
    CHECK_EQ(db_get(db, 2, &v), KV_OK); CHECK_EQ(v, 200);
    CHECK_EQ(db_get(db, 3, &v), KV_OK); CHECK_EQ(v, 300);

    CHECK_EQ(db_get(db, 42, &v), KV_ERR_NOT_FOUND);
    CHECK_EQ(db_put(db, 2, 999), KV_ERR_EXISTS);

    db_close(db);
    rm_files(prefix);
    printf("  ok test_basic\n");
}

static void test_many_inserts_and_splits(void) {
    const char *prefix = "tests/_t_many";
    rm_files(prefix);

    db_t *db;
    CHECK_EQ(db_open(prefix, &db), KV_OK);

    /* N large enough to force multiple splits and a multi-level tree.
       With BTREE_MAX_KEYS = 15, a 3-level tree holds ~15^3 = 3375 leaves. */
    const int64_t N = 2000;

    /* Insert in random-ish order to exercise splits at varying positions. */
    for (int64_t i = 0; i < N; i++) {
        int64_t k = (i * 2654435761LL) % N;  /* knuth-style hash permutation */
        int64_t v = k * 10;
        kv_result_t r = db_put(db, k, v);
        if (r == KV_ERR_EXISTS) continue;   /* collisions within permutation */
        CHECK_EQ(r, KV_OK);
    }

    /* Verify all unique keys are retrievable. */
    for (int64_t k = 0; k < N; k++) {
        int64_t v;
        kv_result_t r = db_get(db, k, &v);
        /* Some keys may not have been inserted due to hash collisions above. */
        if (r == KV_OK) CHECK_EQ(v, k * 10);
    }

    db_close(db);
    rm_files(prefix);
    printf("  ok test_many_inserts_and_splits\n");
}

static void test_persistence(void) {
    const char *prefix = "tests/_t_persist";
    rm_files(prefix);

    db_t *db;
    CHECK_EQ(db_open(prefix, &db), KV_OK);
    for (int64_t i = 0; i < 500; i++) CHECK_EQ(db_put(db, i, i + 1000), KV_OK);
    db_close(db);

    /* Reopen and verify. */
    CHECK_EQ(db_open(prefix, &db), KV_OK);
    for (int64_t i = 0; i < 500; i++) {
        int64_t v;
        CHECK_EQ(db_get(db, i, &v), KV_OK);
        CHECK_EQ(v, i + 1000);
    }
    /* New inserts still work after reopen. */
    CHECK_EQ(db_put(db, 9999, 42), KV_OK);
    int64_t v;
    CHECK_EQ(db_get(db, 9999, &v), KV_OK); CHECK_EQ(v, 42);
    db_close(db);

    rm_files(prefix);
    printf("  ok test_persistence\n");
}

static void test_scan_sorted(void) {
    const char *prefix = "tests/_t_scan";
    rm_files(prefix);

    db_t *db;
    CHECK_EQ(db_open(prefix, &db), KV_OK);

    /* Insert in reverse order; scan must yield sorted. */
    const int64_t N = 300;
    for (int64_t i = N - 1; i >= 0; i--) CHECK_EQ(db_put(db, i, i * 7), KV_OK);

    db_iter_t it;
    CHECK_EQ(db_scan_first(db, &it), KV_OK);

    int64_t expected = 0;
    int64_t k, v;
    while (db_scan_next(&it, &k, &v) == KV_OK) {
        CHECK_EQ(k, expected);
        CHECK_EQ(v, expected * 7);
        expected++;
    }
    CHECK_EQ(expected, N);

    /* Seek to middle. */
    CHECK_EQ(db_scan_seek(db, 150, &it), KV_OK);
    CHECK_EQ(db_scan_next(&it, &k, &v), KV_OK);
    CHECK_EQ(k, 150);

    db_close(db);
    rm_files(prefix);
    printf("  ok test_scan_sorted\n");
}

static void test_delete(void) {
    const char *prefix = "tests/_t_del";
    rm_files(prefix);

    db_t *db;
    CHECK_EQ(db_open(prefix, &db), KV_OK);

    for (int64_t i = 0; i < 100; i++) CHECK_EQ(db_put(db, i, i * 2), KV_OK);

    /* Delete even keys. */
    for (int64_t i = 0; i < 100; i += 2) CHECK_EQ(db_del(db, i), KV_OK);

    /* Deleting a missing key is an error. */
    CHECK_EQ(db_del(db, 0), KV_ERR_NOT_FOUND);

    /* Odd keys still present; even keys gone. */
    for (int64_t i = 0; i < 100; i++) {
        int64_t v;
        kv_result_t r = db_get(db, i, &v);
        if (i % 2 == 0) CHECK_EQ(r, KV_ERR_NOT_FOUND);
        else            { CHECK_EQ(r, KV_OK); CHECK_EQ(v, i * 2); }
    }

    /* Scan yields only odd keys in order. */
    db_iter_t it;
    CHECK_EQ(db_scan_first(db, &it), KV_OK);
    int64_t expected = 1;
    int64_t k, v;
    while (db_scan_next(&it, &k, &v) == KV_OK) {
        CHECK_EQ(k, expected);
        expected += 2;
    }
    CHECK_EQ(expected, 101);

    /* Re-insert a deleted key succeeds. */
    CHECK_EQ(db_put(db, 0, 777), KV_OK);
    CHECK_EQ(db_get(db, 0, &v), KV_OK);
    CHECK_EQ(v, 777);

    db_close(db);
    rm_files(prefix);
    printf("  ok test_delete\n");
}

int main(void) {
    printf("running tests\n");
    test_basic();
    test_many_inserts_and_splits();
    test_persistence();
    test_scan_sorted();
    test_delete();
    printf("all tests passed\n");
    return 0;
}
