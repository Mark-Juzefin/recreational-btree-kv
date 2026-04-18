/*
 * Acceptance tests for step 05. Copy this into bytefile_test.c and
 * build. The `tests.c` file itself is the frozen specification of
 * the bytefile_t API — do not edit to make tests pass; fix the
 * implementation instead.
 */
#include "bytefile.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(cond) do {                                           \
    if (!(cond)) {                                                 \
        fprintf(stderr, "FAIL %s:%d: %s\n",                        \
                __FILE__, __LINE__, #cond);                        \
        exit(1);                                                   \
    }                                                              \
} while (0)

static const char *TEST_PATH = "/tmp/bytefile_test_step05.bin";

static void cleanup_file(void) {
    unlink(TEST_PATH);
}

static void test_open_creates_file(void) {
    cleanup_file();
    bytefile_t *f = NULL;
    CHECK(bytefile_open(TEST_PATH, &f) == KV_OK);
    CHECK(f != NULL);
    uint64_t size = 123;
    CHECK(bytefile_size(f, &size) == KV_OK);
    CHECK(size == 0);
    bytefile_close(f);
}

static void test_write_then_read(void) {
    cleanup_file();
    bytefile_t *f = NULL;
    CHECK(bytefile_open(TEST_PATH, &f) == KV_OK);

    const char payload[] = "hello world";
    CHECK(bytefile_write_at(f, 0, payload, sizeof(payload)) == KV_OK);

    char buf[32] = {0};
    CHECK(bytefile_read_at(f, 0, buf, sizeof(payload)) == KV_OK);
    CHECK(memcmp(buf, payload, sizeof(payload)) == 0);

    bytefile_close(f);
}

static void test_write_at_offset(void) {
    cleanup_file();
    bytefile_t *f = NULL;
    CHECK(bytefile_open(TEST_PATH, &f) == KV_OK);

    const char a[] = "AAAA";
    const char b[] = "BBBB";
    CHECK(bytefile_write_at(f, 0,    a, 4) == KV_OK);
    CHECK(bytefile_write_at(f, 1024, b, 4) == KV_OK);

    uint64_t size = 0;
    CHECK(bytefile_size(f, &size) == KV_OK);
    CHECK(size >= 1028);

    char buf[4];
    CHECK(bytefile_read_at(f, 0,    buf, 4) == KV_OK);
    CHECK(memcmp(buf, a, 4) == 0);
    CHECK(bytefile_read_at(f, 1024, buf, 4) == KV_OK);
    CHECK(memcmp(buf, b, 4) == 0);

    bytefile_close(f);
}

static void test_persistence_across_reopen(void) {
    cleanup_file();
    bytefile_t *f = NULL;
    CHECK(bytefile_open(TEST_PATH, &f) == KV_OK);
    const char msg[] = "persist-me";
    CHECK(bytefile_write_at(f, 0, msg, sizeof(msg)) == KV_OK);
    CHECK(bytefile_sync(f) == KV_OK);
    bytefile_close(f);

    bytefile_t *g = NULL;
    CHECK(bytefile_open(TEST_PATH, &g) == KV_OK);
    char buf[sizeof(msg)] = {0};
    CHECK(bytefile_read_at(g, 0, buf, sizeof(msg)) == KV_OK);
    CHECK(memcmp(buf, msg, sizeof(msg)) == 0);
    bytefile_close(g);
}

static void test_read_past_end_is_error(void) {
    cleanup_file();
    bytefile_t *f = NULL;
    CHECK(bytefile_open(TEST_PATH, &f) == KV_OK);
    const char msg[] = "short";
    CHECK(bytefile_write_at(f, 0, msg, sizeof(msg)) == KV_OK);

    char buf[1024];
    /* Asking for way more bytes than exist must not silently succeed. */
    CHECK(bytefile_read_at(f, 0, buf, sizeof(buf)) != KV_OK);
    bytefile_close(f);
}

static void test_close_null_is_safe(void) {
    bytefile_close(NULL);
}

static void test_open_rejects_null_args(void) {
    bytefile_t *f = NULL;
    CHECK(bytefile_open(NULL, &f)        == KV_ERR_ARG);
    CHECK(bytefile_open(TEST_PATH, NULL) == KV_ERR_ARG);
}

int main(void) {
    test_open_creates_file();
    test_write_then_read();
    test_write_at_offset();
    test_persistence_across_reopen();
    test_read_past_end_is_error();
    test_close_null_is_safe();
    test_open_rejects_null_args();
    cleanup_file();
    printf("all tests passed\n");
    return 0;
}
