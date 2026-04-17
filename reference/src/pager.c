#define _POSIX_C_SOURCE 200809L
#include "pager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static const char  PAGER_MAGIC[8]    = {'K','V','I','D','X','0','0','1'};
static const uint32_t PAGER_VERSION  = 1;

struct pager {
    int      fd;
    uint32_t num_pages;
    uint32_t root_page;
    int      dirty_header;
};

/* Page 0 is the file header. Disk layout is explicit (not a cast of a struct)
   so the on-disk format is independent of compiler padding. */
static kv_result_t header_read(pager_t *p) {
    uint8_t buf[KV_PAGE_SIZE];
    ssize_t n = pread(p->fd, buf, KV_PAGE_SIZE, 0);
    if (n != (ssize_t)KV_PAGE_SIZE) return KV_ERR_IO;

    if (memcmp(buf, PAGER_MAGIC, 8) != 0) return KV_ERR_CORRUPT;

    uint32_t version, page_size, num_pages, root;
    memcpy(&version,   buf + 8,  4);
    memcpy(&page_size, buf + 12, 4);
    memcpy(&num_pages, buf + 16, 4);
    memcpy(&root,      buf + 20, 4);

    if (version != PAGER_VERSION)    return KV_ERR_CORRUPT;
    if (page_size != KV_PAGE_SIZE)   return KV_ERR_CORRUPT;
    if (num_pages < 1)               return KV_ERR_CORRUPT;

    p->num_pages = num_pages;
    p->root_page = root;
    return KV_OK;
}

static kv_result_t header_write(pager_t *p) {
    uint8_t buf[KV_PAGE_SIZE] = {0};
    memcpy(buf, PAGER_MAGIC, 8);
    memcpy(buf + 8,  &PAGER_VERSION, 4);
    uint32_t ps = KV_PAGE_SIZE;
    memcpy(buf + 12, &ps,             4);
    memcpy(buf + 16, &p->num_pages,   4);
    memcpy(buf + 20, &p->root_page,   4);

    ssize_t n = pwrite(p->fd, buf, KV_PAGE_SIZE, 0);
    if (n != (ssize_t)KV_PAGE_SIZE) return KV_ERR_IO;
    p->dirty_header = 0;
    return KV_OK;
}

kv_result_t pager_open(const char *path, pager_t **out) {
    if (!path || !out) return KV_ERR_ARG;

    pager_t *p = calloc(1, sizeof(*p));
    if (!p) return KV_ERR_NOMEM;

    p->fd = open(path, O_RDWR | O_CREAT, 0644);
    if (p->fd < 0) { free(p); return KV_ERR_IO; }

    struct stat st;
    if (fstat(p->fd, &st) != 0) { close(p->fd); free(p); return KV_ERR_IO; }

    if (st.st_size == 0) {
        p->num_pages = 1;  /* page 0 = header */
        p->root_page = 0;  /* 0 = empty tree */
        kv_result_t r = header_write(p);
        if (r != KV_OK) { close(p->fd); free(p); return r; }
    } else {
        if (st.st_size % KV_PAGE_SIZE != 0) {
            close(p->fd); free(p); return KV_ERR_CORRUPT;
        }
        kv_result_t r = header_read(p);
        if (r != KV_OK) { close(p->fd); free(p); return r; }
    }

    *out = p;
    return KV_OK;
}

void pager_close(pager_t *p) {
    if (!p) return;
    if (p->dirty_header) header_write(p);
    fsync(p->fd);
    close(p->fd);
    free(p);
}

kv_result_t pager_read(pager_t *p, uint32_t page_id, void *buf) {
    if (!p || !buf)              return KV_ERR_ARG;
    if (page_id == 0)            return KV_ERR_ARG;
    if (page_id >= p->num_pages) return KV_ERR_ARG;

    off_t off = (off_t)page_id * KV_PAGE_SIZE;
    ssize_t n = pread(p->fd, buf, KV_PAGE_SIZE, off);
    return n == (ssize_t)KV_PAGE_SIZE ? KV_OK : KV_ERR_IO;
}

kv_result_t pager_write(pager_t *p, uint32_t page_id, const void *buf) {
    if (!p || !buf)              return KV_ERR_ARG;
    if (page_id == 0)            return KV_ERR_ARG;
    if (page_id >= p->num_pages) return KV_ERR_ARG;

    off_t off = (off_t)page_id * KV_PAGE_SIZE;
    ssize_t n = pwrite(p->fd, buf, KV_PAGE_SIZE, off);
    return n == (ssize_t)KV_PAGE_SIZE ? KV_OK : KV_ERR_IO;
}

kv_result_t pager_alloc(pager_t *p, uint32_t *page_id_out) {
    if (!p || !page_id_out) return KV_ERR_ARG;

    uint32_t id = p->num_pages;
    uint8_t zero[KV_PAGE_SIZE] = {0};
    off_t off = (off_t)id * KV_PAGE_SIZE;
    ssize_t n = pwrite(p->fd, zero, KV_PAGE_SIZE, off);
    if (n != (ssize_t)KV_PAGE_SIZE) return KV_ERR_IO;

    p->num_pages++;
    p->dirty_header = 1;
    *page_id_out = id;
    return KV_OK;
}

kv_result_t pager_sync(pager_t *p) {
    if (!p) return KV_ERR_ARG;
    if (p->dirty_header) {
        kv_result_t r = header_write(p);
        if (r != KV_OK) return r;
    }
    return fsync(p->fd) == 0 ? KV_OK : KV_ERR_IO;
}

uint32_t pager_root(const pager_t *p)              { return p->root_page; }
uint32_t pager_num_pages(const pager_t *p)         { return p->num_pages; }

void pager_set_root(pager_t *p, uint32_t root) {
    p->root_page = root;
    p->dirty_header = 1;
}
