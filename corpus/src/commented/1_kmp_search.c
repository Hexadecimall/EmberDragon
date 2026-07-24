/*
 * kmp_search.c
 *
 * Substring search using the Knuth-Morris-Pratt algorithm. The pattern is
 * preprocessed once into a "longest proper prefix that is also a suffix"
 * (LPS) table so the text is scanned in a single left-to-right pass without
 * ever backing up, giving O(n + m) worst-case time. A naive O(n*m) reference
 * implementation is included for comparison.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Build the failure (LPS) table for `pattern` into the caller-provided `lps`
 * array, which must hold at least `length` entries. lps[i] is the length of
 * the longest proper prefix of pattern[0..i] that is also a suffix of it.
 * This value tells the matcher how far the pattern can be shifted after a
 * mismatch without missing any occurrence. Runs in O(length).
 */
static void buildLpsTable(const char *pattern, int length, int *lps) {
    lps[0] = 0;          /* a single character has no proper prefix/suffix    */
    int prefixLen = 0;   /* length of the current candidate prefix-suffix     */
    int i = 1;

    while (i < length) {
        if (pattern[i] == pattern[prefixLen]) {
            /* Extend the current prefix-suffix by one character. */
            prefixLen++;
            lps[i] = prefixLen;
            i++;
        } else if (prefixLen != 0) {
            /* Fall back to the previous best prefix-suffix and retry without
             * advancing i; this is the heart of KMP's no-backtrack guarantee. */
            prefixLen = lps[prefixLen - 1];
        } else {
            /* No prefix-suffix possible ending here. */
            lps[i] = 0;
            i++;
        }
    }
}

/*
 * Find the first occurrence of `pattern` inside `text` using KMP.
 * Returns the zero-based index of the match, or -1 if `pattern` does not
 * occur. An empty pattern matches at index 0 by convention. Allocates a
 * temporary LPS table on the heap; returns -1 if that allocation fails.
 * Time O(n + m), extra space O(m).
 */
int kmpSearch(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0) {
        return 0;  /* the empty string is a prefix of everything */
    }
    if (m > n) {
        return -1; /* pattern cannot fit inside a shorter text */
    }

    int *lps = malloc((size_t)m * sizeof(int));
    if (lps == NULL) {
        return -1;
    }
    buildLpsTable(pattern, m, lps);

    int i = 0;  /* index into text    */
    int j = 0;  /* index into pattern */
    int result = -1;

    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
            if (j == m) {
                /* Matched the whole pattern; report where it started. */
                result = i - m;
                break;
            }
        } else if (j != 0) {
            /* Reuse the table to skip characters already known to match. */
            j = lps[j - 1];
        } else {
            /* Mismatch on the first pattern character: advance the text. */
            i++;
        }
    }

    free(lps);
    return result;
}

/*
 * Reference brute-force search: try the pattern at every alignment of `text`.
 * Returns the first match index, or -1 if absent. Empty pattern matches at 0.
 * Time O(n*m); useful as an oracle when validating kmpSearch.
 */
int naiveSearch(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0) {
        return 0;
    }

    for (int start = 0; start + m <= n; start++) {
        int k = 0;
        while (k < m && text[start + k] == pattern[k]) {
            k++;
        }
        if (k == m) {
            return start;  /* every pattern character lined up */
        }
    }
    return -1;
}

/*
 * Count every (possibly overlapping) occurrence of `pattern` in `text` by
 * repeatedly resuming a KMP-style scan one position past each hit. Returns
 * the number of matches. O(n + m) overall because the LPS table is reused.
 */
int countOccurrences(const char *text, const char *pattern) {
    int m = (int)strlen(pattern);
    if (m == 0) {
        return 0;
    }

    int *lps = malloc((size_t)m * sizeof(int));
    if (lps == NULL) {
        return 0;
    }
    buildLpsTable(pattern, m, lps);

    int n = (int)strlen(text);
    int i = 0, j = 0, total = 0;

    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
            if (j == m) {
                total++;
                j = lps[j - 1];  /* allow overlapping matches to continue */
            }
        } else if (j != 0) {
            j = lps[j - 1];
        } else {
            i++;
        }
    }

    free(lps);
    return total;
}
