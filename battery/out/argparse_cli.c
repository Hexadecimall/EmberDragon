#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
int result;
int n2;
int v76;
int n;
long out;
long file;
int j;
char* obj;
int i;
    result = 0;
    n2 = argc;
    v76 = 0;
    n = 1;
    out = "stdout";
    file = 0;
    j = 1;
    while (j < n2) {
        obj = argv[j];
        if (!(strcmp(obj, "--verbose") != 0 && strcmp(obj, "-v") != 0)) {
            v76 = 1;
            goto L4;
        }
        if (strcmp(obj, "-n") == 0) {
            if (!(j + 1 >= n2)) {
                j++;
                n = atoi(argv[j + 1]);
                if (!((n & 1<<31) == 0)) {
                    n = 0;
                }
                goto L3;
            }
        } else {
            if (strcmp(obj, "-o") == 0) {
                if (!(j + 1 >= n2)) {
                    j++;
                    out = argv[j + 1];
                    goto L2;
                }
            } else {
                if (*obj != 45) {
                    file = obj;
                } else {
                    fprintf(stderr, "unknown flag: %s\n", obj);
                    return 2;
                }
            }
            L2:
        }
        L3:
        L4:
        j++;
    }
    if (file == 0) {
        file = "(none)";
    }
    if (v76 != 0) {
        printf("verbose mode on\n");
    }
    printf("file=%s out=%s count=%d\n", file, out, n);
    i = 0;
    while (i < n) {
        printf("[%d] processing %s\n", i, file);
        i++;
    }
    return 0;
}

