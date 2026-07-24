/*
 * rabinkarp.c — Rabin-Karp rolling hash and substring search.
 *
 * A rolling hash lets you slide a fixed-width window across a string and update
 * the window's hash in O(1) per step instead of rehashing the whole window.
 * This module implements the classic polynomial rolling hash over a prime
 * modulus and uses it to drive Rabin-Karp substring search, which finds a
 * pattern in expected O(n + m) time.
 */

#include <stdint.h>
#include <stddef.h>

/* Base used to treat the window as digits of a number in this radix. 256 means
 * "one byte is one digit", which is natural for arbitrary byte data. */
#define RK_BASE 256u

/* A large prime modulus keeps hash values bounded and collisions rare. It is
 * chosen so that RK_BASE * RK_MOD still fits comfortably inside 64 bits. */
#define RK_MOD 1000000007ULL

/* Opaque-ish state for an incremental rolling hash over a window of bytes. */
typedef struct RollingHash {
    uint64_t hash;       /* current hash of the bytes in the window */
    uint64_t pow_high;   /* RK_BASE^(window_len-1) mod RK_MOD, for byte removal */
    size_t   window_len; /* fixed number of bytes the window holds */
} RollingHash;

/*
 * Compute RK_BASE^exp modulo RK_MOD using fast exponentiation.
 *
 * exp: the exponent. Returns base^exp mod RK_MOD in O(log exp). Used to derive
 * the multiplier that the leaving byte was scaled by.
 */
static uint64_t rk_pow(uint64_t exp) {
    uint64_t result = 1, base = RK_BASE % RK_MOD;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % RK_MOD; /* fold this bit */
        base = (base * base) % RK_MOD;                  /* square for next bit */
        exp >>= 1;
    }
    return result;
}

/*
 * Initialize a rolling hash over the first window_len bytes of data.
 *
 * rh:         state to fill in.
 * data:       buffer holding at least window_len bytes.
 * window_len: fixed window width; must be >= 1.
 * After this call rh->hash is the hash of data[0..window_len). O(window_len).
 */
void rolling_hash_init(RollingHash *rh, const uint8_t *data, size_t window_len) {
    rh->window_len = window_len;
    rh->hash = 0;
    for (size_t i = 0; i < window_len; i++) {
        /* Horner's method: hash = hash*BASE + byte, reduced each step. */
        rh->hash = (rh->hash * RK_BASE + data[i]) % RK_MOD;
    }
    /* Precompute the weight of the highest digit so we can subtract it when the
     * window slides forward. */
    rh->pow_high = rk_pow((uint64_t)window_len - 1);
}

/*
 * Slide the window one byte to the right.
 *
 * Removes old_byte (the byte leaving on the left) and appends new_byte (the
 * byte entering on the right), updating the hash in O(1).
 *
 * rh:       state previously initialized with rolling_hash_init.
 * old_byte: the byte that is leaving the window.
 * new_byte: the byte that is entering the window.
 */
void rolling_hash_roll(RollingHash *rh, uint8_t old_byte, uint8_t new_byte) {
    /* Subtract the contribution of the leaving byte, which sat in the high
     * position. Add RK_MOD before subtracting to avoid unsigned underflow. */
    uint64_t high = (rh->pow_high * old_byte) % RK_MOD;
    rh->hash = (rh->hash + RK_MOD - high) % RK_MOD;
    /* Shift remaining digits up by one place and drop the new byte in low. */
    rh->hash = (rh->hash * RK_BASE + new_byte) % RK_MOD;
}

/*
 * Hash an arbitrary byte range with the same polynomial scheme.
 *
 * data: pointer to the first byte. len: number of bytes. Returns the hash so a
 * caller can compare against a RollingHash value over an equal-length window.
 * O(len).
 */
uint64_t rolling_hash_of(const uint8_t *data, size_t len) {
    uint64_t h = 0;
    for (size_t i = 0; i < len; i++) {
        h = (h * RK_BASE + data[i]) % RK_MOD;
    }
    return h;
}

/*
 * Find the first occurrence of pattern within text using Rabin-Karp.
 *
 * text:    haystack bytes.
 * text_len: length of the haystack.
 * pattern: needle bytes.
 * pat_len: length of the needle.
 * Returns the index of the first match, or -1 if the pattern does not occur or
 * is longer than the text. Expected O(text_len + pat_len); worst case
 * O(text_len * pat_len) when many hashes collide, so equal hashes are verified
 * byte-by-byte before being accepted.
 */
long rabin_karp_search(const uint8_t *text, size_t text_len,
                       const uint8_t *pattern, size_t pat_len) {
    if (pat_len == 0) return 0;          /* empty pattern matches at the start */
    if (pat_len > text_len) return -1;   /* needle cannot fit in haystack */

    uint64_t pat_hash = rolling_hash_of(pattern, pat_len);

    RollingHash rh;
    rolling_hash_init(&rh, text, pat_len);

    for (size_t i = 0; ; i++) {
        if (rh.hash == pat_hash) {
            /* Hashes agree; confirm with a real comparison to rule out a
             * collision (a false positive in the hash). */
            size_t j = 0;
            while (j < pat_len && text[i + j] == pattern[j]) j++;
            if (j == pat_len) return (long)i; /* genuine match */
        }
        if (i + pat_len >= text_len) break;   /* window is at the far right */
        rolling_hash_roll(&rh, text[i], text[i + pat_len]);
    }
    return -1;
}
