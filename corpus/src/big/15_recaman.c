/*
 * recaman.c — Recaman's sequence and related step-walk sequences.
 *
 * Recaman's sequence wanders the number line by subtracting the step index
 * when the result is positive and not yet visited, otherwise adding it.
 * This module generates the sequence, tracks visited values with a bitset,
 * and answers questions about first appearances. Integer and bit logic only.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * A compact set of non-negative integers backed by a bit array.
 *
 * Each value v occupies one bit at word `v/64`, bit `v%64`. Capacity is
 * fixed at construction; values outside [0, capacity) are treated as absent.
 */
typedef struct {
    uint64_t *words;  /* bit storage, one bit per candidate value */
    uint64_t capacity; /* number of distinct values representable */
} VisitedSet;

/*
 * Create a VisitedSet able to track values in [0, capacity).
 *
 * capacity: the exclusive upper bound on storable values.
 * return:   an initialized set (all bits clear), or a set with NULL words
 *           on allocation failure. Caller calls visited_free().
 */
VisitedSet visited_make(uint64_t capacity) {
    VisitedSet set;
    set.capacity = capacity;
    uint64_t word_count = capacity / 64 + 1; /* round up to whole words */
    set.words = (uint64_t *)calloc((size_t)word_count, sizeof(uint64_t));
    return set;
}

/*
 * Release a VisitedSet's storage.
 *
 * set:    pointer to the set; safe to call on a failed allocation.
 */
void visited_free(VisitedSet *set) {
    free(set->words);
    set->words = NULL;
    set->capacity = 0;
}

/*
 * Test whether a value is present in the set.
 *
 * Out-of-range values report absent, which lets callers probe freely.
 *
 * set:    the set to query.
 * value:  the value to look up.
 * return: 1 if present, 0 otherwise. O(1).
 */
int visited_contains(const VisitedSet *set, uint64_t value) {
    if (value >= set->capacity) {
        return 0;
    }
    uint64_t word = value >> 6;        /* value / 64 */
    uint64_t bit = value & 63u;        /* value % 64 */
    return (set->words[word] >> bit) & 1u;
}

/*
 * Mark a value as present.
 *
 * Out-of-range values are ignored rather than overrunning the bitset.
 *
 * set:    the set to mutate.
 * value:  the value to insert.
 */
void visited_add(VisitedSet *set, uint64_t value) {
    if (value >= set->capacity) {
        return;
    }
    uint64_t word = value >> 6;
    uint64_t bit = value & 63u;
    set->words[word] |= (uint64_t)1 << bit;
}

/*
 * Generate the first `count` terms of Recaman's sequence.
 *
 * Rule from a(0) = 0: at step n, let candidate = a(n-1) - n. Use it if it is
 * positive and unvisited; otherwise use a(n-1) + n. Every produced term is
 * recorded so later steps can test for prior visits.
 *
 * out:    destination array, at least `count` elements.
 * count:  how many terms to generate.
 * return: the number of terms written. Returns fewer than `count` only if
 *         the visited-set allocation fails. O(count) time.
 */
uint32_t recaman_sequence(uint64_t *out, uint32_t count) {
    if (count == 0) {
        return 0;
    }
    /* Terms can grow but stay well under count*count; size the set to give
     * the back-step branch room to find unvisited values. */
    VisitedSet seen = visited_make((uint64_t)count * count + 1);
    if (seen.words == NULL) {
        return 0;
    }
    out[0] = 0;
    visited_add(&seen, 0);
    for (uint32_t n = 1; n < count; n++) {
        uint64_t prev = out[n - 1];
        uint64_t back = prev - n; /* may underflow; guard with prev > n */
        uint64_t value;
        if (prev > n && !visited_contains(&seen, back)) {
            /* Step backward: result is positive and previously unseen. */
            value = back;
        } else {
            /* Otherwise step forward by n. */
            value = prev + n;
        }
        out[n] = value;
        visited_add(&seen, value);
    }
    visited_free(&seen);
    return count;
}

/*
 * Find the index of the first occurrence of `target` in Recaman's sequence.
 *
 * Generates terms up to `limit` and returns the position of the first hit.
 * Because the sequence revisits and skips values irregularly, a value may
 * appear late or not within the searched window.
 *
 * target: the value to locate.
 * limit:  maximum number of terms to examine.
 * return: the zero-based index of the first occurrence, or -1 (as int64_t)
 *         if `target` does not appear within `limit` terms.
 */
int64_t recaman_first_index(uint64_t target, uint32_t limit) {
    if (limit == 0) {
        return -1;
    }
    uint64_t *terms = (uint64_t *)malloc((size_t)limit * sizeof(uint64_t));
    if (terms == NULL) {
        return -1;
    }
    uint32_t produced = recaman_sequence(terms, limit);
    int64_t found = -1;
    for (uint32_t i = 0; i < produced; i++) {
        if (terms[i] == target) {
            found = (int64_t)i;
            break; /* first occurrence wins */
        }
    }
    free(terms);
    return found;
}
