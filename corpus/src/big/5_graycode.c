/*
 * graycode.c — Reflected binary (Gray) code conversions and utilities.
 *
 * Gray code orders integers so that consecutive values differ in exactly one
 * bit, which is invaluable for rotary encoders and error-resilient counters.
 * This module converts between standard binary and Gray code and generates
 * the full Gray sequence for a given bit width.
 */

#include <stddef.h>
#include <stdint.h>

/*
 * Convert a standard binary value to its reflected Gray code. Each output bit
 * is the XOR of the input bit and the next-higher input bit, which is exactly
 * `value ^ (value >> 1)`. Returns the Gray-coded value. O(1).
 */
uint32_t binary_to_gray(uint32_t value) {
    return value ^ (value >> 1);
}

/*
 * Convert a reflected Gray code value back to standard binary. This is the
 * inverse of binary_to_gray: we accumulate a running XOR prefix as we walk
 * from the most significant bit downward. Returns the decoded binary value.
 * Runs in O(log width) — the loop halves the shift each iteration.
 */
uint32_t gray_to_binary(uint32_t gray) {
    /* Each fold XORs the value with itself shifted right by a power of two,
     * which is equivalent to XOR-ing all higher Gray bits into each bit. */
    gray ^= gray >> 16;
    gray ^= gray >> 8;
    gray ^= gray >> 4;
    gray ^= gray >> 2;
    gray ^= gray >> 1;
    return gray;
}

/*
 * Verify the defining property of Gray code for two consecutive integers:
 * the Gray encodings of `n` and `n + 1` must differ in exactly one bit.
 * Returns 1 if the property holds, 0 otherwise.
 */
int gray_adjacent_differ_by_one(uint32_t n) {
    uint32_t a = binary_to_gray(n);
    uint32_t b = binary_to_gray(n + 1);
    uint32_t diff = a ^ b;
    /* A power of two has exactly one set bit: diff && !(diff & (diff-1)). */
    return diff != 0 && (diff & (diff - 1)) == 0;
}

/*
 * Generate the Gray code sequence for `bits`-bit values into `out`, which
 * must hold at least 2^bits entries. Element i receives the Gray encoding of
 * i, so out[0]=0 and each adjacent pair differs by one bit. Returns the
 * number of entries written, or 0 if `bits` is 0 or exceeds 30 (to keep the
 * 2^bits count within a size_t comfortably).
 */
size_t gray_sequence(int bits, uint32_t *out) {
    if (bits <= 0 || bits > 30)
        return 0;
    size_t count = (size_t)1 << bits;
    for (size_t i = 0; i < count; i++)
        out[i] = binary_to_gray((uint32_t)i);
    return count;
}

/*
 * Return the bit position (0-based, from the least significant bit) that
 * flips when advancing from Gray(n) to Gray(n+1). This is determined purely
 * by the position of the lowest set bit of (n+1). Returns the bit index.
 */
int gray_changed_bit(uint32_t n) {
    uint32_t next = n + 1;
    /* Isolate the lowest set bit of `next` and find its index. */
    uint32_t lowest = next & (~next + 1); /* two's-complement: next & -next */
    int index = 0;
    while ((lowest & 1u) == 0) {
        lowest >>= 1;
        index++;
    }
    return index;
}
