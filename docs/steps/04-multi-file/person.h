//
// Created by Illia on 18.04.2026.
//

#ifndef BTREE_KV_PERSON_H
#define BTREE_KV_PERSON_H

#include "common.h"

typedef struct person {
    char *name;
    char *email;
} person_t;

kv_result_t person_create(const char *name, const char *email, person_t **out);
void        person_free(person_t *p);
kv_result_t person_rename(person_t *p, const char *new_name);
const char *person_name(const person_t *p);

#endif //BTREE_KV_PERSON_H
