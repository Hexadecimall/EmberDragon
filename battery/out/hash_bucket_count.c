#include <cstdio>
#include <cstring>

int hash_key(int a);


static const int table_init[12] = {
    3, 14, 15, 92, 65, 35, 89, 79, 32, 38, 46, 26 
};

int main(int argc, char** argv) {
    int table[12];
    int n2;
    int j;
    int key;
    int n;
    int sum;
    int bucket;
    int i;
    int __addr0[16];
    memcpy(table, table_init, 48);
    n2 = 12;
    j = 0;
    while (j < n2) {
        key = hash_key(table[j]);
        __addr0[key] = __addr0[key] + 1;
        j++;
    }
    n = 0;
    sum = 0;
    bucket = 0;
    i = 0;
    while (i < 7) {
        sum += __addr0[i];
        if (!(__addr0[i] < n)) {
            n = __addr0[i];
            bucket = i;
        }
        printf("bucket[%d]=%u\n", i, __addr0[i]);
        i++;
    }
    printf("total=%u busiest=bucket[%d](%u)\n", sum, bucket, n);
    return 0;
}

int hash_key(int a) {
    int v8;
    v8 = a * 0x9e3779b1;
    v8 ^= v8 >> 15;
    return (v8 - v8 / 7 * 7);
}

