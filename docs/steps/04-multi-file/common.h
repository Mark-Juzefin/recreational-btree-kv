//
// Created by Illia on 18.04.2026.
//

#ifndef BTREE_KV_COMMON_H
#define BTREE_KV_COMMON_H

typedef enum {
    KV_OK         =  0,    // успіх
    KV_ERR_NOMEM  = -1,    // malloc/strdup/realloc повернув NULL
    KV_ERR_ARG    = -2,    // невалідний аргумент (NULL, від'ємне, тощо)
} kv_result_t;

static inline const char *kv_strerror(int code) {
    switch ((kv_result_t)code) {
        case KV_OK:        return "ok";
        case KV_ERR_NOMEM: return "out of memory";
        case KV_ERR_ARG:   return "bad argument";
    }
    return "unknown";
}

#endif //BTREE_KV_COMMON_H
