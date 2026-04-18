/*
 * Acceptance tests for step 04. This file is the specification of what
 * person.h / person.c must provide. Do not edit. When you're done,
 * copy its contents into person_test.c and adjust only if the header
 * names differ.
 */
#include "common.h"
#include "person.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond) do {                                           \
    if (!(cond)) {                                                 \
        fprintf(stderr, "FAIL %s:%d: %s\n",                        \
                __FILE__, __LINE__, #cond);                        \
        exit(1);                                                   \
    }                                                              \
} while (0)

static void test_create_success(void) {
    person_t *p = NULL;
    kv_result_t rc = person_create("Ada Lovelace", "ada@example.com", &p);
    CHECK(rc == KV_OK);
    CHECK(p != NULL);
    CHECK(strcmp(person_name(p), "Ada Lovelace") == 0);
    person_free(p);
}

static void test_create_rejects_null_args(void) {
    person_t *p = NULL;
    CHECK(person_create(NULL, "x@y", &p) == KV_ERR_ARG);
    CHECK(person_create("x", NULL, &p)   == KV_ERR_ARG);
    CHECK(person_create("x", "x@y", NULL) == KV_ERR_ARG);
}

static void test_rename_replaces_name(void) {
    person_t *p = NULL;
    CHECK(person_create("before", "a@b", &p) == KV_OK);
    CHECK(person_rename(p, "after") == KV_OK);
    CHECK(strcmp(person_name(p), "after") == 0);
    person_free(p);
}

static void test_rename_rejects_null(void) {
    person_t *p = NULL;
    CHECK(person_create("x", "y", &p) == KV_OK);
    CHECK(person_rename(p, NULL) == KV_ERR_ARG);
    CHECK(person_rename(NULL, "z") == KV_ERR_ARG);
    /* original name must be intact */
    CHECK(strcmp(person_name(p), "x") == 0);
    person_free(p);
}

static void test_free_null_is_safe(void) {
    person_free(NULL);
}

static void test_strerror_returns_strings(void) {
    CHECK(kv_strerror(KV_OK) != NULL);
    CHECK(kv_strerror(KV_ERR_NOMEM) != NULL);
    CHECK(kv_strerror(KV_ERR_ARG) != NULL);
}

int main(void) {
    test_create_success();
    test_create_rejects_null_args();
    test_rename_replaces_name();
    test_rename_rejects_null();
    test_free_null_is_safe();
    test_strerror_returns_strings();
    printf("all tests passed\n");
    return 0;
}
