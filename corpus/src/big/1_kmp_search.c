/*
 * kmp_search.c — Knuth-Morris-Pratt substring search.
 *
 * Implements exact substring matching in O(n + m) time by precomputing a
 * "failure function" (longest proper prefix that is also a suffix) over the
 * pattern. On a mismatch the search slides the pattern forward by the failure
 * value instead of restarting at the next text position, so the text cursor
 * never moves backward and every text character is examined at most twice.
 */

#include <stdlib.h>
#include <string.h>

/*
 * Build the KMP failure table for `pattern`.
 *
 * table[i] holds the length of the longest proper prefix of pattern[0..i]
 * that is also a suffix of pattern[0..i]. This length doubles as the index
 * from which comparison resumes after a mismatch following a partial match.
 *
 * Parameters:
 *   pattern - NUL-terminated needle (length must be >= 1).
 *   length  - number of characters in `pattern`.
 *   table   - caller-provided array of at least `length` ints to fill.
 * Complexity: O(length) time, O(1) extra space.
 */
static void build_failure_table(const char *pattern, int length, int *table) {
    table[0] = 0;            /* A single character has no proper prefix/suffix. */
    int prefix_len = 0;      /* Length of the current candidate border. */

    /* `i` scans positions 1..length-1, extending or shrinking the border. */
    for (int i = 1; i < length; i++) {
        /*
         * While the next characters disagree, fall back to the border of the
         * border. This is the heart of KMP: prefix_len cannot decrease below
         * zero and the loop runs amortized O(1) times per `i`.
         */
        while (prefix_len > 0 && pattern[i] != pattern[prefix_len]) {
            prefix_len = table[prefix_len - 1];
        }
        if (pattern[i] == pattern[prefix_len]) {
            prefix_len++;    /* Extend the matched border by one character. */
        }
        table[i] = prefix_len;
    }
}

/*
 * Search `text` for the first occurrence of `pattern` using KMP.
 *
 * Returns the zero-based index of the first match, or -1 if the pattern does
 * not occur. An empty pattern matches at index 0 by convention.
 * Complexity: O(n + m) time where n = strlen(text), m = strlen(pattern).
 * The returned index never requires the caller to free anything.
 */
int kmp_find(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);

    if (m == 0) return 0;    /* Empty needle: match at the start. */
    if (m > n) return -1;    /* Needle longer than haystack cannot fit. */

    int *table = malloc((size_t)m * sizeof(int));
    if (!table) return -1;   /* Out of memory: report "not found" defensively. */
    build_failure_table(pattern, m, table);

    int matched = 0;         /* Count of pattern characters matched so far. */
    for (int i = 0; i < n; i++) {
        /* On mismatch, slide the pattern using the failure table. */
        while (matched > 0 && text[i] != pattern[matched]) {
            matched = table[matched - 1];
        }
        if (text[i] == pattern[matched]) {
            matched++;
        }
        if (matched == m) {
            free(table);
            /* Match ends at `i`, so it begins `m - 1` positions earlier. */
            return i - m + 1;
        }
    }

    free(table);
    return -1;               /* Exhausted the text with no full match. */
}

/*
 * Count all (possibly overlapping) occurrences of `pattern` in `text`.
 *
 * Returns the number of matches, 0 when none exist. Overlapping matches are
 * counted: searching "aa" in "aaaa" returns 3. Complexity O(n + m).
 */
int kmp_count(const char *text, const char *pattern) {
    int n = (int)strlen(text);
    int m = (int)strlen(pattern);
    if (m == 0 || m > n) return 0;

    int *table = malloc((size_t)m * sizeof(int));
    if (!table) return 0;
    build_failure_table(pattern, m, table);

    int matched = 0;
    int occurrences = 0;
    for (int i = 0; i < n; i++) {
        while (matched > 0 && text[i] != pattern[matched]) {
            matched = table[matched - 1];
        }
        if (text[i] == pattern[matched]) {
            matched++;
        }
        if (matched == m) {
            occurrences++;
            /* Reuse the failure table to permit overlapping matches. */
            matched = table[matched - 1];
        }
    }

    free(table);
    return occurrences;
}
