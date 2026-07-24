#include <cstdio>
#include <cstring>

long walk_sum(long a, int arr);
void reverse(long a, int arr);
void swap_ints(int* obj3, int* obj4);


static const int matrix_init[4] = {
    1, 2, 3, 4 
};
static const int k = 4;
static const char str[] = "\n";

int main(int argc, char** argv) {
    int matrix[4];
    int* obj;
    // neon:  ldr    value0, [v73, #0x0]  // = 0x01000000020000000300000004000000
    memcpy(matrix, matrix_init, 16);  // data assembled on the stack via NEON
    printf("sum=%ld\n", walk_sum(k[12], 7));
    reverse(matrix, 7);
    printf("reversed:");
    obj = matrix;
    while (obj < matrix + (7 << 2)) {
        printf(" %d", *obj);
        obj += 4;
    }
    printf(str);
    return 0;
}

long walk_sum(long a, int arr) {
    long sum;
    long n;
    sum = 0;
    n = a + (arr << 2);
    while (a < n) {
        sum += *a;
        a += 4;
    }
    return sum;
}

void reverse(long a, int arr) {
    long v8;
    long n;
    v8 = a;
    n = a + (arr << 2) - 4;
    while (v8 < n) {
        swap_ints(v8, n);
        v8 += 4;
        n -= 4;
    }
    return;
}

void swap_ints(int* obj3, int* obj4) {
    int v12;
    v12 = *obj3;
    *obj3 = *obj4;
    *obj4 = v12;
    return;
}

