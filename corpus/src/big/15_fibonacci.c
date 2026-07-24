/*
 * fibonacci.c — Fibonacci number computation.
 *
 * Provides several ways to obtain Fibonacci numbers: a fast iterative
 * single-value lookup, bulk table generation into a caller-supplied
 * buffer, and an exact fast-doubling routine that reaches large indices
 * in logarithmic time. All values use 64-bit unsigned integers, so
 * results are correct up to F(93); beyond that the true value overflows.
 */

#include <stdint.h>
#include <stdlib.h>

/* Largest index whose Fibonacci value still fits in a uint64_t. */
#define FIB_MAX_EXACT_INDEX 93

/*
 * Compute the n-th Fibonacci number iteratively.
 *
 * Uses the standard two-variable rolling pair so only constant memory is
 * needed. F(0) = 0 and F(1) = 1 by convention.
 *
 * n:      index of the desired Fibonacci number.
 * return: F(n), or 0 if n exceeds FIB_MAX_EXACT_INDEX (the result would
 *         no longer be exact). Runs in O(n) time, O(1) space.
 */
uint64_t fibonacci(uint32_t n) {
    if (n > FIB_MAX_EXACT_INDEX) {
        /* Refuse to return a silently-wrapped, incorrect value. */
        return 0;
    }
    uint64_t previous = 0; /* holds F(i-1) at the top of each step */
    uint64_t current = 1;  /* holds F(i)   at the top of each step */
    for (uint32_t i = 0; i < n; i++) {
        /* Advance the pair (F(i-1), F(i)) to (F(i), F(i+1)). */
        uint64_t next = previous + current;
        previous = current;
        current = next;
    }
    return previous;
}

/*
 * Fill a buffer with the first `count` Fibonacci numbers.
 *
 * out:    destination array; must hold at least `count` elements.
 * count:  how many values to write, starting at F(0).
 * return: the number of values actually written. Values past
 *         FIB_MAX_EXACT_INDEX would overflow, so generation stops early
 *         and the returned count reflects the truncation.
 */
uint32_t fibonacci_table(uint64_t *out, uint32_t count) {
    uint32_t written = 0;
    uint64_t previous = 0;
    uint64_t current = 1;
    for (uint32_t i = 0; i < count; i++) {
        if (i > FIB_MAX_EXACT_INDEX) {
            break; /* stop before producing an inexact entry */
        }
        out[written++] = previous;
        uint64_t next = previous + current;
        previous = current;
        current = next;
    }
    return written;
}

/*
 * Compute F(n) in logarithmic time using the fast-doubling identities:
 *   F(2k)   = F(k) * (2*F(k+1) - F(k))
 *   F(2k+1) = F(k)^2 + F(k+1)^2
 *
 * This walks the bits of n from most to least significant, doubling the
 * index each step and conditionally advancing by one. Far faster than the
 * linear method for large n, though here it shares the same exactness bound.
 *
 * n:      index of the desired Fibonacci number.
 * return: F(n), or 0 if n exceeds FIB_MAX_EXACT_INDEX. O(log n) time.
 */
uint64_t fibonacci_fast(uint32_t n) {
    if (n > FIB_MAX_EXACT_INDEX) {
        return 0;
    }
    uint64_t a = 0; /* F(k)   for the current prefix of n's bits */
    uint64_t b = 1; /* F(k+1) for the current prefix of n's bits */
    /* Process bits from bit 31 down to bit 0. */
    for (int bit = 31; bit >= 0; bit--) {
        /* Doubling step: derive (F(2k), F(2k+1)) from (F(k), F(k+1)). */
        uint64_t two_b_minus_a = 2 * b - a;
        uint64_t d = a * two_b_minus_a; /* F(2k) */
        uint64_t e = a * a + b * b;     /* F(2k+1) */
        a = d;
        b = e;
        /* If this bit of n is set, advance the index by one. */
        if ((n >> bit) & 1u) {
            uint64_t sum = a + b;
            a = b;
            b = sum;
        }
    }
    return a;
}

/*
 * Test whether x is a Fibonacci number.
 *
 * Generates Fibonacci numbers until reaching or passing x; an exact match
 * means x belongs to the sequence. Note 0 and 1 are both Fibonacci numbers.
 *
 * x:      candidate value.
 * return: 1 if x is in the Fibonacci sequence, 0 otherwise. O(log x) time
 *         since Fibonacci numbers grow exponentially.
 */
int is_fibonacci(uint64_t x) {
    uint64_t previous = 0;
    uint64_t current = 1;
    while (previous < x) {
        uint64_t next = previous + current;
        previous = current;
        current = next;
    }
    return previous == x;
}
