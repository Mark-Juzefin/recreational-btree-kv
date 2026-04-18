//
// Created by Illia on 18.04.2026.
//
#include "intvec.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

intvec_t *intvec_new(size_t initial_cap) {
    intvec_t *v = malloc(sizeof(*v)); // sizeof(*v) = 24 - malloc(24)
    if (v == NULL) {
        return NULL;
    }
    v->data = malloc(initial_cap * sizeof(*v->data));
    if (v->data == NULL) {
        free(v);
        return NULL;
    }
    v->len = 0;
    v->cap = initial_cap;
    return v;
}

void intvec_push(intvec_t *v, int64_t x) {
    if (v->len == v->cap) {
        size_t new_cap = (v->cap == 0) ? 1 : v->cap * 2;

        int64_t *new_data = realloc(v->data, new_cap * sizeof(*new_data));
        assert(new_data != NULL);

        v->data = new_data;
        v->cap = new_cap;
    }

    v->data[v->len] = x;
    v->len++;
}

int64_t   intvec_get(const intvec_t *v, size_t i) {
    assert(i < v->len);
    return v->data[i];
}

void      intvec_free(intvec_t *v) {
    if (v == NULL) return;
    free(v->data);
    free(v);
}
