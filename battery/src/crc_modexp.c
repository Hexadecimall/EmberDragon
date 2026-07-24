/* CRC-32 (bit-at-a-time) over a buffer + modular exponentiation. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static uint32_t crc32(const uint8_t *buf, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int b = 0; b < 8; b++) {
            uint32_t mask = -(crc & 1u);          /* 0xFFFFFFFF or 0 */
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1 % mod;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;                                /* square-and-multiply */
    }
    return result;
}

int main(void) {
    const char *msg = "The quick brown fox jumps over the lazy dog";
    uint32_t c = crc32((const uint8_t *)msg, strlen(msg));

    /* Fermat-style check: a^(p-1) mod p == 1 for prime p */
    uint64_t p = 1000000007ULL, ok = 0;
    for (uint64_t a = 2; a < 10; a++)
        ok += (mod_pow(a, p - 1, p) == 1);

    printf("crc32=%08X fermat_pass=%llu\n", c, (unsigned long long)ok);
    return (int)(c & 0x7F) ^ (int)ok;
}
