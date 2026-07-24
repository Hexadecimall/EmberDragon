// Hand-rolled popcount (Kernighan + SWAR) and parity, compared two ways.
#include <cstdio>
#include <cstdint>

static int popcount_kernighan(uint32_t x) {
    int n = 0;
    while (x) { x &= (x - 1); n++; }   // clears lowest set bit each pass
    return n;
}

static int popcount_swar(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    return (int)((x * 0x01010101u) >> 24);
}

static int parity(uint32_t x) {
    x ^= x >> 16; x ^= x >> 8; x ^= x >> 4;
    x &= 0xF;
    return (0x6996 >> x) & 1;            // 16-entry parity LUT in a constant
}

int main() {
    uint32_t seed = 0xC0FFEEu;
    int disagree = 0, parity_sum = 0;
    for (int i = 0; i < 100; i++) {
        seed = seed * 1103515245u + 12345u;   // LCG to vary the input
        uint32_t x = seed ^ ((uint32_t)i << 19);
        int a = popcount_kernighan(x);
        int b = popcount_swar(x);
        if (a != b) disagree++;
        parity_sum += parity(x) ^ (a & 1);     // must always be 0 (consistent)
    }
    printf("disagree=%d parity_residual=%d\n", disagree, parity_sum);
    return disagree + parity_sum;
}
