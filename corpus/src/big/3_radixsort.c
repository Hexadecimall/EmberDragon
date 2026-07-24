/*
 * radixsort.c — LSD radix sort for unsigned 32-bit integers.
 *
 * Sorts an array of uint32_t values ascending using least-significant-digit
 * radix sort with a base-256 (one byte per pass) counting-sort kernel. Four
 * stable passes — one per byte — produce a fully sorted array in O(n) time
 * for fixed-width keys, independent of the value distribution.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RADIX_BITS  8                 /* Process 8 bits (one byte) per pass. */
#define BUCKET_COUNT (1 << RADIX_BITS) /* 256 buckets for a byte digit.      */
#define KEY_BYTES   (32 / RADIX_BITS)  /* Four passes cover a 32-bit key.    */

/*
 * countingPass — One stable counting sort keyed on a single byte.
 *
 * Distributes `source` into `dest` ordered by the byte at the given shift.
 * Within each bucket the original relative order is preserved (stability),
 * which is what allows the four LSD passes to compose into a full sort.
 *
 * @param source     Input array for this pass.
 * @param dest       Output array (must differ from source).
 * @param count      Number of elements.
 * @param byteShift  Bit offset of the digit to sort on (0, 8, 16, or 24).
 */
static void countingPass(const uint32_t *source, uint32_t *dest,
                         int count, int byteShift) {
    int histogram[BUCKET_COUNT];
    memset(histogram, 0, sizeof(histogram));

    /* Tally how many keys fall into each of the 256 buckets. */
    for (int i = 0; i < count; i++) {
        uint32_t digit = (source[i] >> byteShift) & 0xFF;
        histogram[digit]++;
    }

    /* Prefix-sum the histogram into starting offsets per bucket. */
    int offset = 0;
    for (int b = 0; b < BUCKET_COUNT; b++) {
        int bucketSize = histogram[b];
        histogram[b] = offset; /* histogram now holds write positions. */
        offset += bucketSize;
    }

    /* Scatter each key to its bucket's next free slot (stable, left-to-right). */
    for (int i = 0; i < count; i++) {
        uint32_t digit = (source[i] >> byteShift) & 0xFF;
        dest[histogram[digit]] = source[i];
        histogram[digit]++;
    }
}

/*
 * radixSortU32 — Sort an array of unsigned 32-bit integers ascending.
 *
 * Performs KEY_BYTES stable counting passes, ping-ponging between the
 * caller's array and a temporary buffer. After an even number of passes
 * the data lands back in the original array. Runs in O(n) time and O(n)
 * extra space.
 *
 * @param array  Pointer to the uint32_t array.
 * @param count  Number of elements.
 * @return       0 on success, -1 if the scratch buffer allocation failed.
 */
int radixSortU32(uint32_t *array, int count) {
    if (array == 0 || count < 2) {
        return 0; /* Empty or single-element arrays are already sorted. */
    }

    uint32_t *buffer = (uint32_t *)malloc((size_t)count * sizeof(uint32_t));
    if (buffer == 0) {
        return -1; /* Allocation failed; leave the input unchanged. */
    }

    uint32_t *from = array;
    uint32_t *to = buffer;

    /* One pass per byte, from least to most significant. */
    for (int pass = 0; pass < KEY_BYTES; pass++) {
        countingPass(from, to, count, pass * RADIX_BITS);

        /* Swap roles: this pass's output is next pass's input. */
        uint32_t *swap = from;
        from = to;
        to = swap;
    }

    /* KEY_BYTES is even (4), so the sorted data sits in `array` already.
     * If it ever became odd we would need to copy `from` back here. */
    if (from != array) {
        memcpy(array, from, (size_t)count * sizeof(uint32_t));
    }

    free(buffer);
    return 0;
}
