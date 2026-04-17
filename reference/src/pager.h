#ifndef KV_PAGER_H
#define KV_PAGER_H

#include "common.h"

typedef struct pager pager_t;

kv_result_t pager_open(const char *path, pager_t **out);
void        pager_close(pager_t *p);

kv_result_t pager_read(pager_t *p, uint32_t page_id, void *buf);
kv_result_t pager_write(pager_t *p, uint32_t page_id, const void *buf);
kv_result_t pager_alloc(pager_t *p, uint32_t *page_id_out);
kv_result_t pager_sync(pager_t *p);

uint32_t pager_root(const pager_t *p);
void     pager_set_root(pager_t *p, uint32_t root);

uint32_t pager_num_pages(const pager_t *p);

#endif
