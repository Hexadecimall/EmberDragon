/*
 * collatz.c — Collatz (3n+1) sequence analysis.
 *
 * Tools for exploring the Collatz conjecture: stopping times, the peak
 * value a trajectory reaches, optional recording of the full orbit, and a
 * scan to find the most stubborn starting value in a range. Everything is
 * 64-bit unsigned because intermediate peaks can vastly exceed the start.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * Apply one Collatz step to n.
 *
 * Even numbers are halved; odd numbers map to 3n+1. The function is the
 * single source of truth for the transition rule used everywhere below.
 *
 * n:      current value, assumed >= 1.
 * return: the next value in the trajectory.
 */
uint64_t collatz_step(uint64_t n) {
    if ((n & 1u) == 0) {
        return n / 2;        /* even: shrink toward 1 */
    }
    return 3 * n + 1;        /* odd: the characteristic 3n+1 jump */
}

/*
 * Count how many steps it takes for n to reach 1.
 *
 * This is the "total stopping time". A start of 1 takes zero steps. The
 * Collatz conjecture asserts this always terminates, but it is unproven;
 * callers must trust that the trajectory is finite.
 *
 * start:  starting value, must be >= 1.
 * return: number of steps to reach 1, or 0 if start is 0 (invalid input).
 */
uint64_t collatz_steps(uint64_t start) {
    if (start == 0) {
        return 0;            /* 0 is outside the domain; report nothing */
    }
    uint64_t n = start;
    uint64_t steps = 0;
    while (n != 1) {
        n = collatz_step(n);
        steps++;
    }
    return steps;
}

/*
 * Find the highest value the trajectory of `start` ever reaches.
 *
 * Odd steps can push the value far above the starting point before it
 * eventually descends, so this peak is interesting for overflow analysis.
 *
 * start:  starting value, must be >= 1.
 * return: the maximum value seen across the whole trajectory (at least
 *         `start` itself). O(stopping time) iterations.
 */
uint64_t collatz_peak(uint64_t start) {
    uint64_t n = start;
    uint64_t peak = start;   /* the start is the initial best-known peak */
    while (n != 1) {
        n = collatz_step(n);
        if (n > peak) {
            peak = n;        /* record a new high-water mark */
        }
    }
    return peak;
}

/*
 * Record the full trajectory of `start` into a caller-supplied buffer.
 *
 * The sequence including both `start` and the terminating 1 is written in
 * order. Writing stops if the buffer fills, so a too-small buffer yields a
 * truncated prefix rather than an overrun.
 *
 * start:    starting value, must be >= 1.
 * out:      destination array.
 * capacity: number of slots available in `out`.
 * return:   number of values written (including start and the final 1
 *           unless truncated by capacity).
 */
uint32_t collatz_trajectory(uint64_t start, uint64_t *out, uint32_t capacity) {
    uint32_t length = 0;
    uint64_t n = start;
    for (;;) {
        if (length >= capacity) {
            break;           /* buffer exhausted; return the prefix */
        }
        out[length++] = n;
        if (n == 1) {
            break;           /* reached the fixed point; done */
        }
        n = collatz_step(n);
    }
    return length;
}

/*
 * Locate the value in [lo, hi] with the longest stopping time.
 *
 * Ties are broken toward the smallest such value, since the search keeps
 * the first maximum found while scanning upward.
 *
 * lo:        inclusive lower bound, must be >= 1.
 * hi:        inclusive upper bound.
 * out_steps: if non-NULL, receives the winning stopping time.
 * return:    the starting value with the maximal stopping time in range.
 *            O((hi-lo) * average stopping time).
 */
uint64_t collatz_longest_in_range(uint64_t lo, uint64_t hi, uint64_t *out_steps) {
    uint64_t best_start = lo;
    uint64_t best_steps = collatz_steps(lo);
    for (uint64_t candidate = lo + 1; candidate <= hi; candidate++) {
        uint64_t steps = collatz_steps(candidate);
        if (steps > best_steps) {
            best_steps = steps;
            best_start = candidate;
        }
    }
    if (out_steps != NULL) {
        *out_steps = best_steps;
    }
    return best_start;
}
