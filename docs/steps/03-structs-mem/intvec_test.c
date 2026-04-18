 #include "intvec.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond) do {                                          \
      if (!(cond)) {                                                \
          fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
          exit(1);                                                  \
      }                                                             \
  } while (0)

int main(void) {
    intvec_t *v = intvec_new(4);
    CHECK(v != NULL);
    CHECK(v->len == 0);
    CHECK(v->cap == 4);

    for (int64_t i = 0; i < 1000; i++) {
        intvec_push(v, i * 10);
    }
    CHECK(v->len == 1000);
    CHECK(v->cap >= 1000);               // після growth'у cap буде 1024

    CHECK(intvec_get(v, 0)   == 0);
    CHECK(intvec_get(v, 42)  == 420);
    CHECK(intvec_get(v, 999) == 9990);

    intvec_free(v);
    intvec_free(NULL);                   // має тихо пройти

    printf("all tests passed\n");
    return 0;
}
