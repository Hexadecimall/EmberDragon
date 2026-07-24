/*
 * regex_bitmap.cpp
 *
 * The Shift-And (bitap) algorithm for exact and approximate substring search.
 * It encodes the search state as a single machine word where bit j means
 * "the pattern prefix of length j+1 currently matches ending here". One bitwise
 * shift-and-OR per text byte advances all positions at once, giving O(text)
 * search for patterns up to 64 characters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Precomputed search tables for one pattern. 'mask[c]' has a 1 bit in every
 * position where byte 'c' occurs in the pattern; this is the per-character
 * "where could this byte extend a match" map the bitap loop needs. */
class BitapMatcher {
public:
    uint64_t mask[256];   /* per-byte occurrence bitmasks */
    int      length;      /* pattern length (must be <= 64) */

    /*
     * Build the occurrence tables for 'pattern'. Patterns longer than 64 bytes
     * are truncated to 64 because the state must fit in a uint64_t. After
     * construction the matcher is reusable across many texts.
     */
    BitapMatcher(const char *pattern) {
        for (int i = 0; i < 256; i++) mask[i] = 0;
        length = 0;
        for (const char *p = pattern; *p && length < 64; p++, length++) {
            /* Set the bit at this pattern position for this character. */
            mask[(unsigned char)*p] |= (uint64_t)1 << length;
        }
    }

    /*
     * Find the first exact occurrence of the pattern in 'text' (length 'n').
     * Returns the 0-based start index of the match, or -1 if absent.
     *
     * Invariant: after processing text[i], bit j of 'state' is set iff the
     * first j+1 pattern bytes match text ending at i. When the top bit
     * (position length-1) is set, a full match ends at i. O(n) time.
     */
    int findFirst(const char *text, int n) const {
        if (length == 0) return 0;          /* empty pattern matches at 0 */
        uint64_t state = 0;
        uint64_t accept = (uint64_t)1 << (length - 1);
        for (int i = 0; i < n; i++) {
            /* Shift in a fresh "length-1 prefix could start here" bit (the |1),
             * then keep only positions consistent with the current byte. */
            state = ((state << 1) | 1) & mask[(unsigned char)text[i]];
            if (state & accept)
                return i - length + 1;       /* match ends at i, started here */
        }
        return -1;
    }

    /*
     * Approximate search allowing up to 'k' edits (substitutions, insertions,
     * deletions) via the row-doubling bitap variant. Returns the end index of
     * the first match within 'k' edits, or -1 if none. Uses k+1 state words;
     * each text byte costs O(k) word operations, so total O(n*k).
     */
    int findApprox(const char *text, int n, int k) const {
        if (length == 0) return 0;
        if (k < 0) k = 0;
        /* state[d] tracks matches achievable with at most d edits. */
        uint64_t *state = (uint64_t *)malloc((size_t)(k + 1) * sizeof(uint64_t));
        for (int d = 0; d <= k; d++) {
            /* Pre-seed so the first characters may already be "deleted". */
            state[d] = ((uint64_t)1 << d) - 1;
        }
        uint64_t accept = (uint64_t)1 << (length - 1);
        int result = -1;

        for (int i = 0; i < n && result < 0; i++) {
            uint64_t prevDiag = state[0];   /* state[d-1] before this update */
            /* Exact row first: the standard shift-and step. */
            state[0] = ((state[0] << 1) | 1) & mask[(unsigned char)text[i]];

            for (int d = 1; d <= k; d++) {
                uint64_t cur = state[d];
                /* Combine the four ways to stay within d edits:
                 *  - match:       shift current exact-ish row, filter by mask
                 *  - substitution:advance the lower-edit diagonal
                 *  - insertion:   reuse the lower-edit row at this position
                 *  - deletion:    shift the lower-edit row */
                uint64_t match = ((cur << 1) | 1) & mask[(unsigned char)text[i]];
                uint64_t sub   = (prevDiag << 1) | 1;
                uint64_t ins   = prevDiag;
                uint64_t del   = (state[d - 1] << 1) | 1;
                prevDiag = cur;                 /* save before overwriting */
                state[d] = match | sub | ins | del;
            }

            if (state[k] & accept) result = i;  /* a <=k-edit match ends at i */
        }

        free(state);
        return result;
    }
};

/*
 * Count all non-overlapping exact occurrences of a pattern in a text.
 * After each match the scan resumes one byte past the match start to avoid
 * counting overlaps. Returns the number of occurrences found. O(n * matches)
 * in the worst case (re-running findFirst on suffixes).
 */
static int bitap_count(const BitapMatcher &m, const char *text, int n) {
    int count = 0;
    int offset = 0;
    while (offset < n) {
        int rel = m.findFirst(text + offset, n - offset);
        if (rel < 0) break;
        count++;
        /* Advance past this match's end to enforce non-overlap. */
        offset += rel + m.length;
    }
    return count;
}
