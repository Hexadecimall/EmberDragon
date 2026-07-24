/*
 * boyer_moore.c — Boyer-Moore string search with the bad-character heuristic.
 *
 * Boyer-Moore compares the pattern against the text right-to-left and, on a
 * mismatch, skips ahead using a precomputed table of the last position of each
 * byte in the pattern. This "bad character rule" lets the search jump over
 * large stretches of text, giving strong sub-linear behavior on typical input.
 */

#include <string.h>

/* The alphabet size for an 8-bit byte; one slot per possible character. */
#define ALPHABET_SIZE 256

/*
 * Compute the last-occurrence table for the bad-character rule.
 *
 * For every byte value, last[c] is set to the largest index in `pattern` at
 * which that byte appears, or -1 if it never appears. The -1 default is what
 * makes the rule shift the whole pattern past an absent character.
 *
 * Parameters:
 *   pattern - the needle.
 *   length  - number of bytes in `pattern`.
 *   last    - caller-provided array of ALPHABET_SIZE ints to fill.
 * Complexity: O(ALPHABET_SIZE + length).
 */
static void build_last_table(const char *pattern, int length, int *last) {
    for (int c = 0; c < ALPHABET_SIZE; c++) {
        last[c] = -1;        /* Assume the character is absent. */
    }
    /* Left-to-right fill means later indices overwrite earlier ones, so each
       slot ends up holding the rightmost occurrence — exactly what we want. */
    for (int i = 0; i < length; i++) {
        last[(unsigned char)pattern[i]] = i;
    }
}

/*
 * Find the first occurrence of `pattern` in `text` via Boyer-Moore.
 *
 * Returns the zero-based start index of the first match, or -1 if absent.
 * An empty pattern matches at index 0. Worst case is O(n*m) but typical and
 * best-case behavior is sub-linear thanks to the skip table.
 */
int boyer_moore_find(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0) return 0;
    if (m > n) return -1;

    int last[ALPHABET_SIZE];
    build_last_table(pattern, m, last);

    int shift = 0;           /* Alignment of the pattern's left edge in text. */
    while (shift <= n - m) {
        int j = m - 1;       /* Compare from the rightmost pattern character. */
        while (j >= 0 && pattern[j] == text[shift + j]) {
            j--;             /* Characters agree; walk leftward. */
        }

        if (j < 0) {
            return shift;    /* Walked off the left end: full match found. */
        }

        /*
         * Mismatch at text[shift + j]. The bad-character rule aligns the last
         * occurrence of that text byte in the pattern with this position. The
         * (j - last[...]) shift can be <= 0 when the character sits to the
         * right of j, so we clamp to a minimum advance of 1 to avoid looping.
         */
        int bad_char_index = last[(unsigned char)text[shift + j]];
        int skip = j - bad_char_index;
        shift += (skip > 1) ? skip : 1;
    }

    return -1;               /* No alignment produced a complete match. */
}

/*
 * Report whether `pattern` occurs anywhere in `text`.
 *
 * Returns 1 if present, 0 otherwise. Thin convenience wrapper over
 * boyer_moore_find that hides the index.
 */
int boyer_moore_contains(const char *text, const char *pattern) {
    return boyer_moore_find(text, pattern) >= 0;
}

/*
 * Find the last occurrence of `pattern` in `text`.
 *
 * Returns the zero-based start index of the rightmost match, or -1 if absent.
 * Implemented by scanning forward and remembering the most recent hit; this
 * keeps the code simple at the cost of not stopping early.
 */
int boyer_moore_find_last(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0) return n;    /* Empty needle "ends" at the text length. */
    if (m > n) return -1;

    int last[ALPHABET_SIZE];
    build_last_table(pattern, m, last);

    int best = -1;           /* Index of the rightmost match seen so far. */
    int shift = 0;
    while (shift <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[shift + j]) {
            j--;
        }
        if (j < 0) {
            best = shift;    /* Record but keep searching for a later match. */
            shift += 1;      /* Advance by one to test the next alignment. */
        } else {
            int skip = j - last[(unsigned char)text[shift + j]];
            shift += (skip > 1) ? skip : 1;
        }
    }
    return best;
}
