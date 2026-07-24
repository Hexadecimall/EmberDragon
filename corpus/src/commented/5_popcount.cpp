/*
 * popcount.cpp - Hamming weight (population count) and related bit-distance
 * utilities implemented several ways: a parallel SWAR algorithm, a per-byte
 * table lookup, and Hamming distance between two values.
 *
 * Everything is integer-only and dependency-free; the class wraps a small
 * cached byte-count table to amortize repeated lookups.
 */

#include <cstdint>
#include <cstddef>

/*
 * Count the set bits in a 64-bit word using the classic SWAR (SIMD-within-a-
 * register) technique: sums are accumulated in parallel 2-, 4-, then 8-bit
 * fields before a final multiply collapses them. Returns the count in [0,64].
 * Runs in a fixed number of operations regardless of the bit pattern.
 */
unsigned int popcount64(uint64_t x) {
    /* Pair up adjacent bits: each 2-bit field now holds its own bit sum. */
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    /* Sum 2-bit fields into 4-bit fields. */
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    /* Sum 4-bit fields into 8-bit fields (max value 8 fits in a nibble). */
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    /* The multiply places the sum of all byte-sums into the top byte. */
    return (unsigned int)((x * 0x0101010101010101ULL) >> 56);
}

/*
 * Compute the Hamming distance between two 64-bit values: the number of bit
 * positions at which they differ. Returns a count in [0,64].
 */
unsigned int hamming_distance(uint64_t a, uint64_t b) {
    /* Differing bits are exactly the set bits of the XOR. */
    return popcount64(a ^ b);
}

/*
 * Return the position (0 = least significant) of the lowest set bit of 'x',
 * or -1 if 'x' is zero. Uses the isolate-lowest-bit trick (x & -x) followed
 * by a popcount of the mask below it.
 */
int lowest_set_bit(uint64_t x) {
    if (x == 0)
        return -1;
    /* (x & -x) isolates the lowest set bit; (isolated - 1) is a mask of all
     * lower positions, whose popcount is the bit's index. */
    uint64_t isolated = x & (~x + 1);
    return (int)popcount64(isolated - 1);
}

/*
 * A reusable population counter that precomputes the bit count of every byte
 * value, trading 256 bytes of memory for a table-driven count. Useful when
 * many counts are needed and a hardware popcount is unavailable.
 */
class PopCounter {
public:
    /*
     * Build the 256-entry byte-count table. Each slot 'i' is seeded from the
     * count of i>>1 plus i's own low bit, so the table fills in O(256).
     */
    PopCounter() {
        byteCounts[0] = 0;
        for (int i = 1; i < 256; i++)
            byteCounts[i] = (uint8_t)(byteCounts[i >> 1] + (i & 1));
    }

    /*
     * Count set bits in a 64-bit word by summing eight table lookups, one per
     * byte. Returns the count in [0,64].
     */
    unsigned int count(uint64_t x) const {
        unsigned int total = 0;
        /* Walk the eight bytes from least to most significant. */
        for (int shift = 0; shift < 64; shift += 8)
            total += byteCounts[(x >> shift) & 0xFF];
        return total;
    }

    /*
     * Count set bits across a contiguous block of 'length' bytes. Returns the
     * total over the whole buffer. O(length).
     */
    size_t countBuffer(const uint8_t *data, size_t length) const {
        size_t total = 0;
        for (size_t i = 0; i < length; i++)
            total += byteCounts[data[i]];
        return total;
    }

private:
    uint8_t byteCounts[256]; /* byteCounts[v] = number of set bits in byte v */
};
