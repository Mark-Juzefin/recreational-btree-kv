//
// Created by Illia on 18.04.2026.
//

#ifndef BTREE_KV_BYTEFILE_H
#define BTREE_KV_BYTEFILE_H

#include "common.h"

#include <stddef.h>                // size_t
#include <stdint.h>                // uint64_t

// Forward-declaration: структура існує, але тіло невідоме caller'у.
// Розмір невідомий → не можна інстанціювати на стеку (`bytefile_t f;` не скомпілюється),
// не можна читати поля (`f->fd` не скомпілюється).
// Можна тільки тримати вказівник і передавати його в API функції.
typedef struct bytefile bytefile_t;

kv_result_t bytefile_open(const char *path, bytefile_t **out);

void bytefile_close(bytefile_t *f);

kv_result_t bytefile_write_at(bytefile_t *f,
                              uint64_t offset,
                              const void *buf,
                              size_t len);

kv_result_t bytefile_read_at(bytefile_t *f,
                             uint64_t offset,
                             void *buf,
                             size_t len);

kv_result_t bytefile_size(bytefile_t *f, uint64_t *size_out);

kv_result_t bytefile_sync(bytefile_t *f);

#endif //BTREE_KV_BYTEFILE_H
