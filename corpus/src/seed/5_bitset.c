#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BITSET_WORDS 16
#define BITS_PER_WORD 64

typedef struct BitSet {
    uint64_t words[BITSET_WORDS];
    int capacity;
} BitSet;

void bitset_init(BitSet *set) {
    int i;
    for (i = 0; i < BITSET_WORDS; i++) {
        set->words[i] = 0;
    }
    set->capacity = BITSET_WORDS * BITS_PER_WORD;
}

int bitset_set(BitSet *set, int index) {
    int word_index, bit_offset;
    if (index < 0 || index >= set->capacity) {
        return -1;
    }
    word_index = index / BITS_PER_WORD;
    bit_offset = index % BITS_PER_WORD;
    set->words[word_index] |= ((uint64_t)1 << bit_offset);
    return 0;
}

int bitset_clear(BitSet *set, int index) {
    int word_index, bit_offset;
    if (index < 0 || index >= set->capacity) {
        return -1;
    }
    word_index = index / BITS_PER_WORD;
    bit_offset = index % BITS_PER_WORD;
    set->words[word_index] &= ~((uint64_t)1 << bit_offset);
    return 0;
}

int bitset_test(const BitSet *set, int index) {
    int word_index, bit_offset;
    if (index < 0 || index >= set->capacity) {
        return 0;
    }
    word_index = index / BITS_PER_WORD;
    bit_offset = index % BITS_PER_WORD;
    return (set->words[word_index] >> bit_offset) & 1;
}

int bitset_count(const BitSet *set) {
    int i, total;
    uint64_t value;
    total = 0;
    for (i = 0; i < BITSET_WORDS; i++) {
        value = set->words[i];
        while (value != 0) {
            value &= (value - 1);
            total++;
        }
    }
    return total;
}

void bitset_union(BitSet *dest, const BitSet *other) {
    int i;
    for (i = 0; i < BITSET_WORDS; i++) {
        dest->words[i] |= other->words[i];
    }
}

void bitset_intersect(BitSet *dest, const BitSet *other) {
    int i;
    for (i = 0; i < BITSET_WORDS; i++) {
        dest->words[i] &= other->words[i];
    }
}

int bitset_find_first(const BitSet *set) {
    int i, bit;
    uint64_t value;
    for (i = 0; i < BITSET_WORDS; i++) {
        value = set->words[i];
        if (value != 0) {
            for (bit = 0; bit < BITS_PER_WORD; bit++) {
                if ((value >> bit) & 1) {
                    return i * BITS_PER_WORD + bit;
                }
            }
        }
    }
    return -1;
}
