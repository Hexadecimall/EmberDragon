#include <cstdio>
#include <cstring>

long trace(long a);
long fold_bytes(long a, long b);


static const int table_init[12] = {
    1, 2, 3, 4, 10, 20, 30, 40, 100, 200, 300, 400 
};

struct Pair {
    int value;
    char _pad4[4];
    int data;
};
int main(int argc, char** argv) {
    int table[12];
    int i;
    char buf[64];
    struct Pair* obj;
    char buf2[64];
    memcpy(table, table_init, 48);
    printf("trace = %d\n", trace(table));
    i = 0;
    while (i < 16) {
        buf[i] = i * 7 + 1;
        i++;
    }
    printf("fold = 0x%08X\n", fold_bytes(buf, 16));
    obj = buf2;
    printf("p[0]=%d *(p+2)=%d\n", obj->value, obj->data);
    return 0;
}

long trace(long a) {
    int sum;
    int i;
    sum = 0;
    i = 0;
    while (i < 3) {
        sum += (a + (i << 4))[i - i / 4 * 4 << 2];
        i++;
    }
    return sum;
}

long fold_bytes(long a, long b) {
int result;
long i;
    result = 0;
    i = 0;
    while (i < b / 4) {
        result ^= a[i] + i;
        i++;
    }
    return result;
}

