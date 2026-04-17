#include <stdio.h>
#include <stdint.h>

int main(void) {
    printf("Hello, C!\n");
    printf("sizeof(int)     = %zu\n", sizeof(int));
    printf("sizeof(long)    = %zu\n", sizeof(long));
    printf("sizeof(int64_t) = %zu\n", sizeof(int64_t));
    return 0;
}
