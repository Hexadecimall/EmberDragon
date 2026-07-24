/*
 * popcount.c — Population-count (Hamming weight) and bit-scanning routines.
 *
 * A small library of branch-light bit-counting primitives: number of set
 * bits, trailing/leading zero counts, Hamming distance between two values,
 * parity, and isolating or clearing the lowest set bit. All operate on
 * 64-bit unsigned words with pure integer arithmetic.
 */

#include <stddef.h>
#include <stdint.h>

/*
 * Count the set bits in a 64-bit word via the SWAR algorithm: pairwise sums
 * are accumulated in-place into wider and wider fields, and a final multiply
 * gathers all byte sums into the top byte. Returns the population count in
 * [0, 64]. Constant time — no loop over the bits.
 */
int popcount64(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555ULL);            /* sum bit pairs */
    x = (x & 0x3333333333333333ULL) +
        ((x >> 2) & 0x3333333333333333ULL);                 /* sum nibbles */
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;             /* sum bytes */
    return (int)((x * 0x0101010101010101ULL) >> 56);       /* gather totals */
}

/*
 * Count trailing zero bits — the number of low-order 0 bits before the first
 * set bit. Returns 64 for an all-zero input, since there is no set bit to
 * stop at. Uses the isolate-lowest-set-bit trick then counts via popcount.
 */
int count_trailing_zeros(uint64_t x) {
    if (x == 0)
        return 64;
    /* (x & -x) keeps only the lowest set bit; subtracting 1 turns every bit
     * below it into ones, and those ones are exactly the trailing zeros. */
    return popcount64((x & (~x + 1)) - 1);
}

/*
 * Count leading zero bits — the number of high-order 0 bits above the most
 * significant set bit. Returns 64 for zero. We "smear" the highest set bit
 * downward to fill all lower positions, then count the resulting ones and
 * subtract from 64.
 */
int count_leading_zeros(uint64_t x) {
    if (x == 0)
        return 64;
    /* Propagate the top set bit into every lower bit position. */
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return 64 - popcount64(x);
}

/*
 * Compute the Hamming distance between two words: the number of bit positions
 * at which they differ. Equal to the population count of their XOR. Returns a
 * value in [0, 64].
 */
int hamming_distance(uint64_t a, uint64_t b) {
    return popcount64(a ^ b);
}

/*
 * Return the parity of a word: 1 if it has an odd number of set bits, 0 if
 * even. Computed by folding the word onto itself and reading the low bit.
 */
int parity64(uint64_t x) {
    x ^= x >> 32;
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (int)(x & 1u);
}

/*
 * Return a word with only the lowest set bit of `x` retained (all others
 * cleared). Returns 0 if `x` is 0. This is the standard `x & -x` idiom.
 */
uint64_t isolate_lowest_set(uint64_t x) {
    return x & (~x + 1);
}

/*
 * Return `x` with its lowest set bit cleared. Returns 0 if `x` is 0. Useful
 * for iterating over set bits one at a time: clear, process, repeat.
 */
uint64_t clear_lowest_set(uint64_t x) {
    return x & (x - 1);
}

/*
 * Collect the indices of all set bits of `x` into `out` (which must hold at
 * least 64 entries), from least to most significant. Returns the number of
 * indices written, i.e. the population count of `x`. O(popcount) iterations.
 */
size_t set_bit_indices(uint64_t x, int *out) {
    size_t n = 0;
    while (x != 0) {
        out[n++] = count_trailing_zeros(x);
        x = clear_lowest_set(x); /* drop the bit we just recorded */
    }
    return n;
}
