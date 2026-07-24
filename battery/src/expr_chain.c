/* Integer expression chains: precedence, ternary, comma, side effects. */
#include <stdio.h>
#include <stdint.h>

static int64_t fold(int64_t a, int64_t b, int64_t c) {
    /* mixes shifts, mul/div/mod, ternary and short-circuit */
    int64_t t = (a * 3 + b) - (c << 2);
    t = t * t - (a ? (b % (c | 1)) : 0);
    int64_t u = ((a & b) | (b ^ c)) + (~a & 0xFF);
    int64_t v = (a > b ? a - b : b - a) + (c && a ? c / (a ? 1 : 0) : 7);
    return t / (u | 1) + (v << (a & 3)) - (b >> 1);
}

int main(int argc, char **argv) {
    int64_t acc = (int64_t)argc;
    for (int i = 0; i < 20; i++) {
        int64_t a = i - 7, b = (i * i) % 11, c = (i << 1) ^ 5;
        acc += fold(a, b, c);
        acc = (acc ^ (acc >> 13)) * 0x9E37 + i;   /* avalanche-ish chain */
        acc -= (i & 1) ? (a + c) : (b - a);
    }
    printf("acc=%lld\n", (long long)acc);
    return (int)(acc & 0x7F);
}
