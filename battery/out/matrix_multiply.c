#include <cstdio>
#include <cstring>

void matmul(long a, long b, long c);


static const int matrix_init[9] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9 
};
static const int matrix2[9] = {
    9, 8, 7, 6, 5, 4, 3, 2, 1 
};
static const char str[] = "\n";

int main(int argc, char** argv) {
    int matrix[9];
    char buf2[64];
    char buf[64];
    int j;
    int i;
    memcpy(matrix, matrix_init, 36);
    memcpy(buf2, matrix2, 36);
    matmul(matrix, buf2, buf);
    j = 0;
    while (j < 3) {
        i = 0;
        while (i < 3) {
            printf("%4d", (buf + (long)(j) * 12)[i << 2]);
            i++;
        }
        printf(str);
        j++;
    }
    return 0;
}

void matmul(int const (*) [3] a, int const (*) [3] b, int (*) [3] c) {
    int j;
    int i;
    int sum;
    int k;
    j = 0;
    while (j < 3) {
        i = 0;
        while (i < 3) {
            sum = 0;
            k = 0;
            while (k < 3) {
                sum += (a + (long)(j) * 12)[k << 2] * (b + (long)(k) * 12)[i << 2];
                k++;
            }
            (c + (long)(j) * 12)[i << 2] = sum;
            i++;
        }
        j++;
    }
    return;
}

