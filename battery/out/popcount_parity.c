#include <cstdio>

int popcount_kernighan(int a);
int popcount_swar(int a);
long parity(int a);


int main(int argc, char** argv) {
    int hash;
    int disagree;
    int sum;
    int i;
    int v24;
    int kernighan;
    int swar;
    hash = 0xc0ffee;
    disagree = 0;
    sum = 0;
    i = 0;
    while (i < 100) {
        hash = hash * 0x41c64e6d + 12345;
        v24 = hash ^ i << 19;
        kernighan = popcount_kernighan(v24);
        swar = popcount_swar(v24);
        if (kernighan != swar) {
            disagree++;
        }
        sum += parity(v24) ^ kernighan & 1;
        i++;
    }
    printf("disagree=%d parity_residual=%d\n", disagree, sum);
    return (disagree + sum);
}

int popcount_kernighan(unsigned int a) {
    int v12;
    int result;
    v12 = a;
    result = 0;
    while (v12 != 0) {
        v12 &= v12 - 1;
        result++;
    }
    return result;
}

int popcount_swar(unsigned int a) {
    int v12;
    v12 = a;
    v12 -= (unsigned)(v12) >> 1 & 0x55555555;
    v12 = (v12 & 0x33333333) + ((unsigned)(v12) >> 2 & 0x33333333);
    v12 = v12 + (v12 >> 4) & 0xf0f0f0f;
    return ((unsigned)(v12 * 0x1010101) >> 24);
}

long parity(unsigned int a) {
    int hash;
    hash = a;
    hash ^= hash >> 16;
    hash ^= hash >> 8;
    hash ^= hash >> 4;
    hash &= 15;
    return (27030 >> hash & 1);
}

