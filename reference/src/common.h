#ifndef KV_COMMON_H
#define KV_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define KV_PAGE_SIZE 4096

typedef enum {
    KV_OK            =  0,
    KV_ERR_NOT_FOUND = -1,
    KV_ERR_IO        = -2,
    KV_ERR_CORRUPT   = -3,
    KV_ERR_NOMEM     = -4,
    KV_ERR_ARG       = -5,
    KV_ERR_EXISTS    = -6,
    KV_ERR_EOF       = -7,
} kv_result_t;

static inline const char *kv_strerror(int code) {
    switch ((kv_result_t)code) {
        case KV_OK:            return "ok";
        case KV_ERR_NOT_FOUND: return "not found";
        case KV_ERR_IO:        return "io error";
        case KV_ERR_CORRUPT:   return "corrupt file";
        case KV_ERR_NOMEM:     return "out of memory";
        case KV_ERR_ARG:       return "bad argument";
        case KV_ERR_EXISTS:    return "already exists";
        case KV_ERR_EOF:       return "end of iteration";
    }
    return "unknown";
}

#endif
