/* Tiny CLI argument parser: --verbose, -n COUNT, -o NAME, positional file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int verbose = 0;
    int count = 1;
    const char *out = "stdout";
    const char *file = NULL;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "--verbose") == 0 || strcmp(a, "-v") == 0) {
            verbose = 1;
        } else if (strcmp(a, "-n") == 0 && i + 1 < argc) {
            count = atoi(argv[++i]);
            if (count < 0) count = 0;
        } else if (strcmp(a, "-o") == 0 && i + 1 < argc) {
            out = argv[++i];
        } else if (a[0] != '-') {
            file = a;
        } else {
            fprintf(stderr, "unknown flag: %s\n", a);
            return 2;
        }
    }

    if (file == NULL) file = "(none)";
    if (verbose) {
        printf("verbose mode on\n");
    }
    printf("file=%s out=%s count=%d\n", file, out, count);
    for (int i = 0; i < count; i++) {
        printf("[%d] processing %s\n", i, file);
    }
    return 0;
}
