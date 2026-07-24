/*
 * rabin_karp.c
 *
 * Rabin-Karp substring search using a polynomial rolling hash. The pattern's
 * hash is compared against a window hash that slides across the text in O(1)
 * per step; only when the hashes collide is a full character comparison done
 * to rule out false positives. Average time is O(n + m); worst case (constant
 * hash collisions) degrades to O(n*m).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Rolling-hash parameters: a small prime base and a large prime modulus to
 * keep intermediate products inside a 64-bit integer without overflow. */
#define HASH_BASE    256u        /* one slot per possible byte value      */
#define HASH_MODULUS 1000000007u /* large prime to spread hashes out      */

/*
 * Compute the polynomial hash of the first `length` bytes of `data` from
 * scratch. The hash treats the bytes as digits in base HASH_BASE, reduced
 * modulo HASH_MODULUS. Returns the hash value. O(length).
 */
static uint64_t hashWindow(const char *data, int length) {
    uint64_t hash = 0;
    for (int i = 0; i < length; i++) {
        /* Horner's method: shift the accumulator up one digit and add the
         * next byte, reducing each step to stay within 64 bits. */
        hash = (hash * HASH_BASE + (unsigned char)data[i]) % HASH_MODULUS;
    }
    return hash;
}

/*
 * Compute HASH_BASE^(length-1) modulo HASH_MODULUS. This is the weight of the
 * highest-order digit, needed to subtract the outgoing character when the
 * window rolls forward. Returns 1 when `length` is 0 or 1. O(length).
 */
static uint64_t highOrderWeight(int length) {
    uint64_t weight = 1;
    for (int i = 0; i < length - 1; i++) {
        weight = (weight * HASH_BASE) % HASH_MODULUS;
    }
    return weight;
}

/*
 * Verify a candidate match by comparing `length` bytes of `text` starting at
 * `offset` against `pattern`. Returns 1 on an exact match, 0 otherwise. This
 * guards against hash collisions, which is what keeps Rabin-Karp correct.
 */
static int verifyMatch(const char *text, int offset, const char *pattern, int length) {
    for (int k = 0; k < length; k++) {
        if (text[offset + k] != pattern[k]) {
            return 0;
        }
    }
    return 1;
}

/*
 * Find the first occurrence of `pattern` in `text` with Rabin-Karp.
 * Returns the zero-based match index, or -1 if the pattern is absent. The
 * empty pattern matches at index 0. O(n + m) on average.
 */
int rabinKarpSearch(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0) {
        return 0;
    }
    if (m > n) {
        return -1;  /* pattern is longer than the text */
    }

    uint64_t patternHash = hashWindow(pattern, m);
    uint64_t windowHash  = hashWindow(text, m);   /* first window of the text */
    uint64_t weight      = highOrderWeight(m);

    for (int i = 0; i <= n - m; i++) {
        /* Cheap hash test first; only verify on a hash hit. */
        if (windowHash == patternHash && verifyMatch(text, i, pattern, m)) {
            return i;
        }

        /* Roll the window one position to the right, unless we are at the end. */
        if (i < n - m) {
            /* Remove the leftmost character's contribution... */
            uint64_t outgoing = ((unsigned char)text[i] * weight) % HASH_MODULUS;
            /* ...add HASH_MODULUS before subtracting to avoid unsigned wrap. */
            windowHash = (windowHash + HASH_MODULUS - outgoing) % HASH_MODULUS;
            /* ...shift up one digit and append the incoming character. */
            windowHash = (windowHash * HASH_BASE + (unsigned char)text[i + m]) % HASH_MODULUS;
        }
    }

    return -1;
}

/*
 * Count all (possibly overlapping) occurrences of `pattern` in `text` using
 * the same rolling hash. Returns the number of matches; an empty pattern
 * yields 0. O(n + m) average.
 */
int rabinKarpCount(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0 || m > n) {
        return 0;
    }

    uint64_t patternHash = hashWindow(pattern, m);
    uint64_t windowHash  = hashWindow(text, m);
    uint64_t weight      = highOrderWeight(m);
    int      matches     = 0;

    for (int i = 0; i <= n - m; i++) {
        if (windowHash == patternHash && verifyMatch(text, i, pattern, m)) {
            matches++;
        }
        if (i < n - m) {
            uint64_t outgoing = ((unsigned char)text[i] * weight) % HASH_MODULUS;
            windowHash = (windowHash + HASH_MODULUS - outgoing) % HASH_MODULUS;
            windowHash = (windowHash * HASH_BASE + (unsigned char)text[i + m]) % HASH_MODULUS;
        }
    }

    return matches;
}
