/*
 * bitset.c — A fixed-capacity bitset backed by an array of 64-bit words.
 *
 * Provides the usual set/clear/test operations plus union, intersection,
 * cardinality (popcount), and forward iteration over set bits. Bits are
 * packed little-end first within each 64-bit word; word index = bit >> 6,
 * in-word index = bit & 63. The capacity is fixed at construction time.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Number of bits stored per backing word. */
#define BITS_PER_WORD 64

/*
 * A bitset over a fixed range [0, capacity). The backing storage is a
 * heap-allocated array of `word_count` 64-bit words, owned by the bitset.
 */
typedef struct {
    uint64_t *words;   /* backing storage, word_count entries */
    size_t word_count; /* number of 64-bit words allocated */
    size_t capacity;   /* number of addressable bits */
} BitSet;

/*
 * Allocate a bitset capable of holding `capacity` bits, all initially clear.
 * Rounds the word count up so the final partial word is fully owned.
 * Returns NULL on allocation failure; caller frees with bitset_free().
 */
BitSet *bitset_create(size_t capacity) {
    BitSet *set = (BitSet *)malloc(sizeof(BitSet));
    if (!set) return NULL;
    /* Ceiling division: one extra word if capacity is not a multiple of 64. */
    set->word_count = (capacity + BITS_PER_WORD - 1) / BITS_PER_WORD;
    set->capacity = capacity;
    /* calloc zeroes the storage, so every bit starts cleared. */
    set->words = (uint64_t *)calloc(set->word_count, sizeof(uint64_t));
    if (!set->words) {
        free(set);
        return NULL;
    }
    return set;
}

/*
 * Release a bitset and its backing storage. Safe to call with NULL.
 */
void bitset_free(BitSet *set) {
    if (!set) return;
    free(set->words);
    free(set);
}

/*
 * Set the bit at `index` to 1. Out-of-range indices are ignored so callers
 * cannot corrupt memory by passing a stale index.
 */
void bitset_set(BitSet *set, size_t index) {
    if (index >= set->capacity) return;
    set->words[index >> 6] |= (uint64_t)1 << (index & 63);
}

/*
 * Clear the bit at `index` to 0. Out-of-range indices are ignored.
 */
void bitset_clear(BitSet *set, size_t index) {
    if (index >= set->capacity) return;
    set->words[index >> 6] &= ~((uint64_t)1 << (index & 63));
}

/*
 * Return 1 if the bit at `index` is set, 0 otherwise. Indices past the end
 * read as 0, mirroring the conceptual "infinite zero tail".
 */
int bitset_test(const BitSet *set, size_t index) {
    if (index >= set->capacity) return 0;
    return (set->words[index >> 6] >> (index & 63)) & 1u;
}

/*
 * Count the number of bits set in a single 64-bit word using the classic
 * SWAR (SIMD-within-a-register) popcount. Runs in a constant handful of
 * operations regardless of how many bits are set.
 */
static int word_popcount(uint64_t w) {
    w = w - ((w >> 1) & 0x5555555555555555ULL);
    w = (w & 0x3333333333333333ULL) + ((w >> 2) & 0x3333333333333333ULL);
    w = (w + (w >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    /* Sum all bytes into the top byte, then shift it down. */
    return (int)((w * 0x0101010101010101ULL) >> 56);
}

/*
 * Return the total number of bits currently set (the set's cardinality).
 * Runs in O(word_count) — one popcount per backing word.
 */
size_t bitset_count(const BitSet *set) {
    size_t total = 0;
    for (size_t i = 0; i < set->word_count; i++)
        total += (size_t)word_popcount(set->words[i]);
    return total;
}

/*
 * In-place union: dst |= src. Both sets must share the same word count;
 * if they differ, the operation is skipped to avoid an out-of-bounds write.
 */
void bitset_union(BitSet *dst, const BitSet *src) {
    if (dst->word_count != src->word_count) return;
    for (size_t i = 0; i < dst->word_count; i++)
        dst->words[i] |= src->words[i];
}

/*
 * In-place intersection: dst &= src. Same shape requirement as union.
 */
void bitset_intersect(BitSet *dst, const BitSet *src) {
    if (dst->word_count != src->word_count) return;
    for (size_t i = 0; i < dst->word_count; i++)
        dst->words[i] &= src->words[i];
}

/*
 * Find the index of the first set bit at or after `from`. Returns the
 * bitset's capacity (a one-past-the-end sentinel) if no such bit exists,
 * which lets callers loop with `i = next(i + 1)` until i == capacity.
 */
size_t bitset_next_set(const BitSet *set, size_t from) {
    for (size_t i = from; i < set->capacity; i++) {
        if ((set->words[i >> 6] >> (i & 63)) & 1u)
            return i;
    }
    return set->capacity;
}
