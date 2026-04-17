//
// Created by Illia on 17.04.2026.
//

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void upper_inplace(char *s) {
    for (; *s != '\0'; s++) {
        *s = (char)toupper((unsigned char)*s);
    }
}

int main(int argc, char **argv) {
    bool upper = false;

    /* First pass: detect flags. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--upper") == 0) {
            upper = true;
        }
    }

    /* Second pass: print non-flag arguments. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--upper") == 0) {
            continue;
        }
        if (upper) {
            upper_inplace(argv[i]);
        }
        size_t len = strlen(argv[i]);
        printf("%s (%zu)\n", argv[i], len);
    }
    return 0;
}
