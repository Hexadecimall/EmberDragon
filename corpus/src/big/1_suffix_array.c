/*
 * suffix_array.c — Suffix array construction and pattern search.
 *
 * A suffix array is the sorted order of all suffixes of a string, stored as
 * their starting indices. Once built, it answers "does this pattern occur?"
 * in O(m log n) time via binary search. This implementation builds the array
 * with a simple comparison sort for clarity, then offers search utilities.
 */

#include <stdlib.h>
#include <string.h>

/*
 * Holds the text and its sorted suffix indices. `order[k]` is the start
 * position of the k-th smallest suffix. The struct borrows `text`; it does
 * not copy it, so the caller must keep the text alive for the array's life.
 */
typedef struct SuffixArray {
    const char *text;
    int length;
    int *order;
} SuffixArray;

/* Shared text pointer for the qsort comparator (qsort has no user-data arg). */
static const char *g_compare_text = NULL;

/*
 * qsort comparator that orders two suffixes lexicographically by their start
 * indices. Returns <0, 0, or >0 like strcmp. Relies on g_compare_text being
 * set to the owning text before sorting.
 */
static int compare_suffixes(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    /* strcmp on NUL-terminated tails gives correct suffix ordering because a
       shorter suffix that is a prefix of a longer one compares as smaller. */
    return strcmp(g_compare_text + ia, g_compare_text + ib);
}

/*
 * Build a suffix array for `text`.
 *
 * Returns a heap-allocated SuffixArray the caller must release with
 * suffix_array_free, or NULL on allocation failure. The construction cost is
 * O(n^2 log n) in the worst case due to character-wise comparisons inside the
 * O(n log n) sort; acceptable for moderate inputs and easy to reason about.
 */
SuffixArray *suffix_array_build(const char *text) {
    int n = (int)strlen(text);
    SuffixArray *sa = malloc(sizeof(SuffixArray));
    if (!sa) return NULL;

    sa->text = text;
    sa->length = n;
    sa->order = malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    if (!sa->order) {
        free(sa);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        sa->order[i] = i;    /* Start unsorted: identity permutation. */
    }

    g_compare_text = text;   /* Hand the text to the comparator. */
    qsort(sa->order, (size_t)n, sizeof(int), compare_suffixes);
    g_compare_text = NULL;   /* Avoid leaving a dangling global around. */

    return sa;
}

/*
 * Compare `pattern` against the suffix beginning at `suffix_start`, looking at
 * only the first strlen(pattern) characters.
 *
 * Returns <0 if the pattern sorts before the suffix, 0 if the pattern is a
 * prefix of the suffix, and >0 if it sorts after. This prefix-bounded compare
 * is what lets binary search locate occurrences rather than full matches.
 */
static int compare_pattern_prefix(const char *pattern, const char *suffix) {
    /* strncmp stops at the pattern length, so a pattern that is a prefix of
       the suffix yields 0 — exactly the "found an occurrence" signal. */
    return strncmp(pattern, suffix, strlen(pattern));
}

/*
 * Search the suffix array for any occurrence of `pattern`.
 *
 * Returns the starting index in the text of one occurrence (not necessarily
 * the first by position), or -1 if the pattern does not occur. An empty
 * pattern returns 0. Complexity: O(m log n) where m = strlen(pattern).
 */
int suffix_array_search(const SuffixArray *sa, const char *pattern) {
    if (pattern[0] == '\0') return 0;
    int lo = 0;
    int hi = sa->length - 1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;          /* Overflow-safe midpoint. */
        int suffix_start = sa->order[mid];
        int cmp = compare_pattern_prefix(pattern, sa->text + suffix_start);
        if (cmp == 0) {
            return suffix_start; /* Pattern is a prefix of this suffix. */
        } else if (cmp < 0) {
            hi = mid - 1;        /* Pattern sorts earlier; search left half. */
        } else {
            lo = mid + 1;        /* Pattern sorts later; search right half. */
        }
    }
    return -1;                   /* Binary search collapsed with no match. */
}

/*
 * Count how many suffixes start with `pattern`, i.e. its occurrence count.
 *
 * Returns the number of occurrences (0 if absent). Because all matching
 * suffixes form a contiguous block in the sorted order, this walks outward
 * from any found match. Complexity: O(m log n + occurrences).
 */
int suffix_array_count(const SuffixArray *sa, const char *pattern) {
    int hit = suffix_array_search(sa, pattern);
    if (hit < 0) return 0;

    /* Re-find the rank of the hit so we can scan its neighbors in `order`. */
    int rank = -1;
    for (int i = 0; i < sa->length; i++) {
        if (sa->order[i] == hit) { rank = i; break; }
    }

    int count = 0;
    /* Expand left while neighbors still begin with the pattern. */
    for (int i = rank; i >= 0; i--) {
        if (compare_pattern_prefix(pattern, sa->text + sa->order[i]) == 0) {
            count++;
        } else {
            break;           /* Block is contiguous; first miss ends it. */
        }
    }
    /* Expand right, skipping `rank` which we already counted above. */
    for (int i = rank + 1; i < sa->length; i++) {
        if (compare_pattern_prefix(pattern, sa->text + sa->order[i]) == 0) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

/*
 * Release a suffix array previously returned by suffix_array_build.
 *
 * Safe to call with NULL. Does not free the borrowed text. After this call
 * the pointer is dangling.
 */
void suffix_array_free(SuffixArray *sa) {
    if (!sa) return;
    free(sa->order);
    free(sa);
}
