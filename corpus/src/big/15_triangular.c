/*
 * triangular.c — Triangular and other figurate number utilities.
 *
 * Triangular numbers (1, 3, 6, 10, ...) count dots in a triangle of rows.
 * This module computes the n-th value with a closed form, inverts it to
 * recover the row index, tests membership, and relates triangular numbers
 * to square and hexagonal figurate numbers. All integer arithmetic.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * Compute the n-th triangular number T(n) = n*(n+1)/2.
 *
 * The product n*(n+1) is always even, so the halving is exact. T(0) = 0.
 *
 * n:      row index.
 * return: the n-th triangular number. O(1) time. Overflows for very large
 *         n; valid while n*(n+1) fits in 64 bits.
 */
uint64_t triangular(uint64_t n) {
    return n * (n + 1) / 2;
}

/*
 * Sum the first n triangular numbers (a tetrahedral number).
 *
 * Equivalent to the closed form n*(n+1)*(n+2)/6, computed here by an
 * explicit loop to keep the relationship to triangular() obvious.
 *
 * n:      how many triangular numbers to add (T(1)..T(n)).
 * return: the n-th tetrahedral number. O(n) time.
 */
uint64_t tetrahedral(uint64_t n) {
    uint64_t total = 0;
    for (uint64_t k = 1; k <= n; k++) {
        total += triangular(k);
    }
    return total;
}

/*
 * Recover the row index of a triangular number, or report non-membership.
 *
 * Solves T(n) = x by integer-searching n. Because triangular numbers grow
 * quadratically, a linear walk from a rough estimate converges quickly; we
 * advance n while T(n) < x and then check for an exact hit.
 *
 * x:      candidate value.
 * return: n such that T(n) == x, or -1 (as int64_t) if x is not triangular.
 *         O(sqrt(x)) time.
 */
int64_t triangular_index(uint64_t x) {
    uint64_t n = 0;
    /* Walk upward until T(n) meets or exceeds x. */
    while (triangular(n) < x) {
        n++;
    }
    if (triangular(n) == x) {
        return (int64_t)n;
    }
    return -1; /* overshot without an exact match: x is not triangular */
}

/*
 * Test whether x is a triangular number.
 *
 * Thin predicate over triangular_index(); avoids exposing the sentinel.
 *
 * x:      candidate value.
 * return: 1 if x is triangular, 0 otherwise. O(sqrt(x)) time.
 */
int is_triangular(uint64_t x) {
    return triangular_index(x) >= 0;
}

/*
 * Compute the n-th square figurate number, n*n.
 *
 * Included so callers can explore the identity that every square number is
 * the sum of two consecutive triangular numbers: n*n = T(n) + T(n-1).
 *
 * n:      index.
 * return: n squared. O(1) time.
 */
uint64_t square_number(uint64_t n) {
    return n * n;
}

/*
 * Compute the n-th hexagonal number, n*(2n - 1).
 *
 * Every hexagonal number is also triangular (H(n) = T(2n-1)), a relation
 * this function makes easy to verify against triangular().
 *
 * n:      index, assumed >= 1 for the standard sequence.
 * return: the n-th hexagonal number. O(1) time.
 */
uint64_t hexagonal_number(uint64_t n) {
    return n * (2 * n - 1);
}

/*
 * Find the first triangular number that has more than `threshold` divisors.
 *
 * A nod to Project Euler problem 12. Divisors are counted by trial division
 * up to the square root, doubling for the paired divisor and correcting for
 * perfect squares.
 *
 * threshold: the divisor count to exceed.
 * return:    the smallest triangular number with strictly more than
 *            `threshold` divisors. O(answer * sqrt(answer)) worst case.
 */
uint64_t first_triangular_over_divisors(uint32_t threshold) {
    uint64_t n = 1;
    for (;;) {
        uint64_t value = triangular(n);
        uint32_t divisors = 0;
        for (uint64_t d = 1; d * d <= value; d++) {
            if ((value % d) == 0) {
                /* d and value/d are a divisor pair. */
                divisors += 2;
                if (d * d == value) {
                    /* Perfect square: the pair collapsed to one divisor. */
                    divisors -= 1;
                }
            }
        }
        if (divisors > threshold) {
            return value;
        }
        n++;
    }
}
