/*
 * bitset.c - A fixed-capacity bitset backed by an array of 64-bit words.
 *
 * Provides the usual set/clear/test operations plus population count and the
 * index of the first set bit. Bits are addressed [0, capacity) and stored
 * little-endian within each word (bit i lives in word i/64, position i%64).
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Number of bits packed into a single backing word. */
#define BITS_PER_WORD 64

/*
 * A bitset of fixed capacity. The 'words' array is heap-allocated and holds
 * ceil(capacity / 64) words; the caller owns it and must release it with
 * bitset_destroy.
 */
typedef struct {
    uint64_t *words;     /* backing storage, one bit per logical position    */
    size_t    capacity;  /* number of addressable bits                       */
    size_t    wordCount; /* number of 64-bit words in 'words'                */
} BitSet;

/*
 * Allocate a BitSet able to hold 'capacity' bits, all initially clear.
 * Returns NULL if capacity is zero or allocation fails. The caller owns the
 * returned pointer and must free it with bitset_destroy.
 */
BitSet *bitset_create(size_t capacity) {
    if (capacity == 0)
        return NULL;
    BitSet *set = (BitSet *)malloc(sizeof(BitSet));
    if (set == NULL)
        return NULL;
    /* Round capacity up to a whole number of words. */
    set->wordCount = (capacity + BITS_PER_WORD - 1) / BITS_PER_WORD;
    set->capacity = capacity;
    set->words = (uint64_t *)calloc(set->wordCount, sizeof(uint64_t));
    if (set->words == NULL) {
        free(set);
        return NULL;
    }
    return set;
}

/*
 * Release a BitSet and its backing storage. Safe to call with NULL.
 */
void bitset_destroy(BitSet *set) {
    if (set == NULL)
        return;
    free(set->words);
    free(set);
}

/*
 * Set the bit at 'index' to 1. Out-of-range indices are ignored so callers
 * cannot corrupt memory by passing a bad position.
 */
void bitset_set(BitSet *set, size_t index) {
    if (index >= set->capacity)
        return;
    set->words[index / BITS_PER_WORD] |= (uint64_t)1 << (index % BITS_PER_WORD);
}

/*
 * Clear the bit at 'index' to 0. Out-of-range indices are ignored.
 */
void bitset_clear(BitSet *set, size_t index) {
    if (index >= set->capacity)
        return;
    set->words[index / BITS_PER_WORD] &= ~((uint64_t)1 << (index % BITS_PER_WORD));
}

/*
 * Test whether the bit at 'index' is set. Returns 1 if set, 0 if clear, and
 * 0 for any out-of-range index.
 */
int bitset_test(const BitSet *set, size_t index) {
    if (index >= set->capacity)
        return 0;
    uint64_t word = set->words[index / BITS_PER_WORD];
    return (word >> (index % BITS_PER_WORD)) & 1u;
}

/*
 * Count the number of set bits across the whole set. Runs in O(wordCount)
 * using Kernighan's trick, which loops once per set bit within each word.
 */
size_t bitset_count(const BitSet *set) {
    size_t total = 0;
    for (size_t w = 0; w < set->wordCount; w++) {
        uint64_t bits = set->words[w];
        /* Each iteration clears the lowest set bit, so it runs exactly once
         * per 1-bit rather than 64 times per word. */
        while (bits != 0) {
            bits &= bits - 1;
            total++;
        }
    }
    return total;
}

/*
 * Return the index of the lowest set bit, or -1 (cast to size_t via the
 * return convention) if the set is entirely clear. We return a signed long
 * so the "not found" sentinel is unambiguous.
 */
long bitset_first_set(const BitSet *set) {
    for (size_t w = 0; w < set->wordCount; w++) {
        uint64_t bits = set->words[w];
        if (bits == 0)
            continue; /* skip whole empty words quickly */
        /* Find the lowest set bit position within this word. */
        for (int b = 0; b < BITS_PER_WORD; b++) {
            if ((bits >> b) & 1u) {
                size_t pos = w * BITS_PER_WORD + b;
                /* The final word may have padding bits beyond capacity. */
                if (pos >= set->capacity)
                    return -1;
                return (long)pos;
            }
        }
    }
    return -1;
}
