#include <cstdio>
#include <cstring>

int crc32(long data, long n2);
long mod_pow(long x, long a, long b);


int main(int argc, char** argv) {
    int crc322;
    long sum;
    long i;
    crc322 = crc32("The quick brown fox jumps over the lazy dog", strlen("The quick brown fox jumps over the lazy dog"));
    sum = 0;
    i = 2;
    while (i < 10) {
        sum += (mod_pow(i, 0x3b9aca07 - 1, 0x3b9aca07) != 1 ? 0 : 1) & 1;
        i++;
    }
    printf("crc32=%08X fermat_pass=%llu\n", crc322, sum);
    return (crc322 & 127 ^ sum);
}

int crc32(long data, long n2) {
    long n;
    int v28;
    long j;
    int i;
    int v8;
    n = n2;
    v28 = -1;
    j = 0;
    while (j < n) {
        v28 ^= data[j];
        i = 0;
        while (i < 8) {
            v8 = 0 - (v28 & 1);
            v28 = 0xedb88320 & v8 ^ v28 >> 1;
            i++;
        }
        j++;
    }
    return (~v28);
}

long mod_pow(long x, long a, long b) {
    long ptr;
    long v16;
    long result;
    ptr = x;
    v16 = a;
    result = 1 - 1 / b * b;
    ptr -= ptr / b * b;
    while (v16 >= 0) {
        if (v16) {
            result = result * ptr - result * ptr / b * b;
        }
        ptr = ptr * ptr - ptr * ptr / b * b;
        v16 = (unsigned long)(v16) >> 1;
    }
    return result;
}

