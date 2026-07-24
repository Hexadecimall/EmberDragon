#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    char buf[101];
    int ptr;
    int v36;
    int i2;
    int i;
    memset(buf, 0, 101);
    ptr = 2;
    while (!((long)(ptr) * (long)(ptr) > 100)) {
        if (!(buf[ptr] != 0)) {
            v36 = ptr * ptr;
            while (v36 <= 100) {
                buf[v36] = 1;
                v36 += ptr;
            }
        }
        ptr++;
    }
    i2 = 0;
    i = 2;
    while (i <= 100) {
        if (!(buf[i] != 0)) {
            printf("%d ", i);
            i2++;
        }
        i++;
    }
    printf("\n%d primes up to %d\n", i2, 100);
    return 0;
}

