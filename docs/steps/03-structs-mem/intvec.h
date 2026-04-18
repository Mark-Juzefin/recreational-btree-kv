//
// Created by Illia on 18.04.2026.
//

#ifndef BTREE_KV_INTVEC_H
#define BTREE_KV_INTVEC_H

#include<stddef.h>
#include<stdint.h>

typedef struct intvec {
    int64_t *data; // 8 bytes
    size_t len; // 8 bytes
    size_t cap; // 8 bytes
} intvec_t; // 24 bytes

intvec_t *intvec_new(size_t initial_cap);
void      intvec_free(intvec_t *v);
void      intvec_push(intvec_t *v, int64_t x);
int64_t   intvec_get(const intvec_t *v, size_t i);


#endif //BTREE_KV_INTVEC_H
