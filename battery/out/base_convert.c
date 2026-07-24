#include <cstdio>
#include <cstring>

void to_base(int a, int b, long data);


static const int matrix_init[4] = {
    0, 5, 42, 255 
};
static const int k = 1024;

int main(int argc, char** argv) {
int matrix[4];
long ch;
int n;
int i;
char bin[64];
char oct[64];
char hex[64];
    // neon:  ldr    value0, [v297, #0x0]  // = 0x00000000050000002a000000ff000000
    memcpy(matrix, matrix_init, 16);  // data assembled on the stack via NEON
    ch = k;
    n = 6;
    i = 0;
    while (i < n) {
        to_base(matrix[i], 2, bin);
        to_base(matrix[i], 8, oct);
        to_base(matrix[i], 16, hex);
        printf("%-6u bin=%-16s oct=%-8s hex=%s\n", matrix[i], bin, oct, hex);
        i++;
    }
    return 0;
}

void to_base(int a, int b, long data) {
int v20;
int i;
char buf[64];
int j;
    v20 = a;
    i = 0;
    if (v20 == 0) {
        i++;
        buf[i] = 48;
    }
    while (v20 >= 0) {
        i++;
        buf[i] = "0123456789abcdef"[v20 - v20 / b * b];
        v20 /= b;
    }
    j = 0;
    while (i > 0) {
        i--;
        j++;
        data[j] = buf[i - 1];
    }
    data[j] = 0;
    return;
}

