/*
 * rabin_karp.c — Rabin-Karp substring search via rolling polynomial hashing.
 *
 * Rabin-Karp hashes the pattern once, then slides a window across the text
 * maintaining the window's hash in O(1) per step. Only when hashes collide
 * does it verify with a direct character comparison, so genuine matches and
 * the rare hash collision are confirmed exactly while most windows are
 * rejected by a single integer comparison.
 */

#include <string.h>
#include <stdint.h>

/* Polynomial base: a small prime larger than the byte alphabet (256). */
#define RK_BASE 257u
/*
 * Modulus for the rolling hash. A large prime keeps collisions rare. All
 * arithmetic is done in uint64_t, and every product stays well under 2^64
 * because operands are reduced mod RK_MOD first.
 */
#define RK_MOD 1000000007u

/*
 * Compute the polynomial hash of the first `length` bytes of `data`.
 *
 * Returns hash = (data[0]*B^(L-1) + ... + data[L-1]) mod RK_MOD. Used to seed
 * both the pattern hash and the initial text window. Complexity: O(length).
 */
static uint64_t window_hash(const char *data, int length) {
    uint64_t hash = 0;
    for (int i = 0; i < length; i++) {
        /* Horner's method keeps every intermediate value reduced mod RK_MOD. */
        hash = (hash * RK_BASE + (unsigned char)data[i]) % RK_MOD;
    }
    return hash;
}

/*
 * Compute RK_BASE raised to `power`, modulo RK_MOD.
 *
 * Returns B^power mod RK_MOD. This is the weight of the leading character in a
 * window of size `power + 1`, needed to subtract it out when rolling forward.
 * Complexity: O(power) via repeated multiplication (no exponentiation tricks
 * to keep the integer logic explicit).
 */
static uint64_t base_power(int power) {
    uint64_t result = 1;
    for (int i = 0; i < power; i++) {
        result = (result * RK_BASE) % RK_MOD;
    }
    return result;
}

/*
 * Verify a candidate match character by character.
 *
 * Returns 1 if the `m` bytes of `pattern` equal the `m` bytes of text starting
 * at `text + offset`, else 0. This guards against the false positives that
 * equal hashes can produce. Complexity: O(m).
 */
static int verify_match(const char *text, int offset, const char *pattern,
                        int m) {
    for (int i = 0; i < m; i++) {
        if (text[offset + i] != pattern[i]) {
            return 0;        /* Hash collision, not a real match. */
        }
    }
    return 1;
}

/*
 * Find the first occurrence of `pattern` in `text` using Rabin-Karp.
 *
 * Returns the zero-based index of the first match, or -1 if absent. An empty
 * pattern matches at index 0. Expected time is O(n + m); worst case is O(n*m)
 * if every window collides, which the verification step still handles safely.
 */
int rabin_karp_find(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0) return 0;
    if (m > n) return -1;

    uint64_t pattern_hash = window_hash(pattern, m);
    uint64_t text_hash = window_hash(text, m);
    uint64_t lead_weight = base_power(m - 1); /* Weight of the dropped char. */

    for (int i = 0; i <= n - m; i++) {
        /* Cheap integer test first; only verify on a hash hit. */
        if (text_hash == pattern_hash && verify_match(text, i, pattern, m)) {
            return i;
        }
        if (i < n - m) {
            /*
             * Roll the window forward by one character:
             *   1. Remove the contribution of the leaving character.
             *   2. Shift remaining characters up one power of the base.
             *   3. Add the entering character.
             * The "+ RK_MOD" before subtracting keeps the value non-negative.
             */
            uint64_t leaving = ((unsigned char)text[i] * lead_weight) % RK_MOD;
            text_hash = (text_hash + RK_MOD - leaving) % RK_MOD;
            text_hash = (text_hash * RK_BASE +
                         (unsigned char)text[i + m]) % RK_MOD;
        }
    }
    return -1;               /* No window matched the pattern. */
}

/*
 * Count all (possibly overlapping) occurrences of `pattern` in `text`.
 *
 * Returns the number of matches, 0 when none. Shares the rolling-hash core
 * with rabin_karp_find but never stops early. Complexity: expected O(n + m).
 */
int rabin_karp_count(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0 || m > n) return 0;

    uint64_t pattern_hash = window_hash(pattern, m);
    uint64_t text_hash = window_hash(text, m);
    uint64_t lead_weight = base_power(m - 1);

    int occurrences = 0;
    for (int i = 0; i <= n - m; i++) {
        if (text_hash == pattern_hash && verify_match(text, i, pattern, m)) {
            occurrences++;
        }
        if (i < n - m) {
            uint64_t leaving = ((unsigned char)text[i] * lead_weight) % RK_MOD;
            text_hash = (text_hash + RK_MOD - leaving) % RK_MOD;
            text_hash = (text_hash * RK_BASE +
                         (unsigned char)text[i + m]) % RK_MOD;
        }
    }
    return occurrences;
}
