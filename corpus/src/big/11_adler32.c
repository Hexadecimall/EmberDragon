/*
 * adler32.c — Adler-32 checksum (the checksum used by zlib/zlib's stored data).
 *
 * Adler-32 maintains two running sums modulo the largest prime below 2^16
 * (65521): sum A accumulates the bytes themselves, while sum B accumulates the
 * running value of A. The 32-bit checksum packs B in the high half and A in the
 * low half. It is far cheaper than CRC-32 yet catches most accidental
 * corruption, which is why zlib uses it for its uncompressed-block integrity.
 */

#include <stdint.h>
#include <stddef.h>

/* Largest prime less than 2^16; the modulus for both running sums. */
#define ADLER_MOD 65521u

/* Number of bytes we can fold before sum B can possibly overflow 32 bits.
 * NMAX is the largest n such that 255*n*(n+1)/2 + (n+1)*(MOD-1) <= 2^32-1. */
#define ADLER_NMAX 5552

/*
 * Compute the Adler-32 checksum of a buffer in one shot.
 *
 * data: bytes to checksum (may be NULL only when len is 0).
 * len:  number of bytes.
 * Returns the 32-bit checksum. The defined seed for empty input is 1, because
 * sum A starts at 1 so that a leading run of zero bytes still changes the
 * checksum. O(len), no allocation.
 */
uint32_t adler32(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint32_t a = 1, b = 0; /* A seeded to 1 per the spec; B seeded to 0 */

    /* Process in blocks of at most NMAX bytes so we only need to take the
     * expensive modulo once per block instead of once per byte. */
    while (len > 0) {
        size_t block = (len < ADLER_NMAX) ? len : ADLER_NMAX;
        len -= block;
        for (size_t i = 0; i < block; i++) {
            a += p[i]; /* A grows by each byte */
            b += a;    /* B grows by the current A, weighting earlier bytes more */
        }
        p += block;
        a %= ADLER_MOD;
        b %= ADLER_MOD;
    }
    return (b << 16) | a; /* high half = B, low half = A */
}

/*
 * Resume an Adler-32 computation across multiple buffers.
 *
 * Pass 1 as the initial adler value to start a fresh checksum, then thread the
 * returned value into the next call. This matches zlib's adler32() contract and
 * lets you checksum a stream without buffering the whole thing.
 *
 * adler: the running checksum so far (use 1 to begin).
 * data:  next chunk of bytes.
 * len:   length of that chunk.
 * Returns the updated checksum. O(len).
 */
uint32_t adler32_update(uint32_t adler, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint32_t a = adler & 0xFFFF;        /* unpack the low 16 bits as A */
    uint32_t b = (adler >> 16) & 0xFFFF; /* unpack the high 16 bits as B */

    while (len > 0) {
        size_t block = (len < ADLER_NMAX) ? len : ADLER_NMAX;
        len -= block;
        for (size_t i = 0; i < block; i++) {
            a += p[i];
            b += a;
        }
        p += block;
        a %= ADLER_MOD;
        b %= ADLER_MOD;
    }
    return (b << 16) | a;
}

/*
 * Combine two Adler-32 checksums as if their underlying data were concatenated.
 *
 * Given adler1 over buffer X and adler2 over buffer Y (where len2 is the length
 * of Y), this returns the checksum of X followed by Y without re-reading X.
 * This is useful for parallel or hierarchical checksumming and mirrors zlib's
 * adler32_combine.
 *
 * adler1: checksum of the first buffer.
 * adler2: checksum of the second buffer.
 * len2:   length in bytes of the second buffer.
 * Returns the combined checksum. O(1).
 */
uint32_t adler32_combine(uint32_t adler1, uint32_t adler2, size_t len2) {
    uint32_t rem = (uint32_t)(len2 % ADLER_MOD); /* shift caused by len2 bytes */

    uint32_t a1 = adler1 & 0xFFFF;
    uint32_t b1 = (adler1 >> 16) & 0xFFFF;
    uint32_t a2 = adler2 & 0xFFFF;
    uint32_t b2 = (adler2 >> 16) & 0xFFFF;

    /* A of the concatenation is A1 + A2 - 1: the two A sums add, but the seed
     * value of 1 must not be double-counted. */
    uint32_t a = (a1 + a2 + ADLER_MOD - 1) % ADLER_MOD;

    /* B of the concatenation = B1 + B2 + rem*(A1-1), with the -1 again removing
     * the duplicated seed; everything is kept inside the modulus. */
    uint32_t b = (rem * (a1 + ADLER_MOD - 1)) % ADLER_MOD;
    b = (b + b1 + b2 + ADLER_MOD - 1) % ADLER_MOD;

    return (b << 16) | a;
}
