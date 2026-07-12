//
// Created by Illia on 18.04.2026.
//

#include "bytefile.h"

#include <errno.h>        // errno після фейлу syscall'у
#include <fcntl.h>        // open() і його O_* флаги
#include <stdlib.h>       // malloc/free
#include <unistd.h>       // close()
#include <sys/types.h> // ssize_t
#include <sys/stat.h>    // дає struct stat і fstat()

// Повне визначення структури — тільки тут.
// Для всіх інших TU це залишиться "incomplete type".
struct bytefile {
    int fd; // POSIX file descriptor: int >= 0 при успіху
};

kv_result_t bytefile_open(const char *path, bytefile_t **out) {
    if (path == NULL || out == NULL) {
        return KV_ERR_ARG;
    }

    // O_RDWR   = open for read+write
    // O_CREAT  = create if missing
    // 0644     = mode (permissions) for newly-created file: rw-r--r--
    // Якщо файл існує, mode ігнорується. Якщо не існує і O_CREAT не заданий — fail.
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        return KV_ERR_IO; // errno скаже чому (EACCES, ENOSPC, ...)
    }

    bytefile_t *f = malloc(sizeof(*f));
    if (f == NULL) {
        close(fd); // не лишай fd висячим — resource leak
        return KV_ERR_NOMEM;
    }

    f->fd = fd;
    *out = f;
    return KV_OK;
}

void bytefile_close(bytefile_t *f) {
    if (f == NULL) return;
    close(f->fd);
    free(f);
}

kv_result_t bytefile_write_at(bytefile_t *f,
                              uint64_t offset,
                              const void *buf,
                              size_t len) {
    if (f == NULL || buf == NULL) {
        return KV_ERR_ARG;
    }

    const uint8_t *p = buf; // byte-wise указівник для арифметики
    size_t written = 0;

    while (written < len) {
        ssize_t n = pwrite(f->fd,
                           p + written,
                           len - written,
                           (off_t) (offset + written));
        if (n == -1) {
            if (errno == EINTR) continue; // перервано сигналом — просто повтори
            return KV_ERR_IO;
        }
        if (n == 0) {
            return KV_ERR_IO; // "неможливо писати більше" — disk full?
        }
        written += (size_t) n;
    }
    return KV_OK;
}

kv_result_t bytefile_read_at(bytefile_t *f,
                             uint64_t offset,
                             void *buf,
                             size_t len) {
    if (f == NULL || buf == NULL) {
        return KV_ERR_ARG;
    }

    uint8_t *p = buf;
    size_t done = 0;

    while (done < len) {
        ssize_t n = pread(f->fd,
                          p + done,
                          len - done,
                          (off_t) (offset + done));
        if (n == -1) {
            if (errno == EINTR) continue;
            return KV_ERR_IO;
        }
        if (n == 0) {
            return KV_ERR_IO; // EOF до того, як прочитали все потрібне
        }
        done += (size_t) n;
    }
    return KV_OK;
}


kv_result_t bytefile_size(bytefile_t *f, uint64_t *size_out) {
    if (f == NULL || size_out == NULL) {
        return KV_ERR_ARG;
    }

    struct stat st;
    if (fstat(f->fd, &st) == -1) {   // fstat — syscall "дай метадані цього FD";
        // &st = куди писати. Повертає 0 при успіху, -1 при фейлі.
        return KV_ERR_IO;
    }
    // st_size — off_t (signed); каст у беззнаковий безпечний,
    // бо розмір файла невід'ємний.
    *size_out = (uint64_t)st.st_size;
    return KV_OK;

}

kv_result_t bytefile_sync(bytefile_t *f) {
    if (f == NULL) {
        return KV_ERR_ARG;
    }

    if (fsync(f->fd) == -1) {
        return KV_ERR_IO;
    }

    return KV_OK;
}
