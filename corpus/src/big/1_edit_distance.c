/*
 * edit_distance.c — Levenshtein edit distance and related string metrics.
 *
 * Computes the minimum number of single-character insertions, deletions, and
 * substitutions needed to transform one string into another. Uses the classic
 * dynamic-programming recurrence but stores only two rows at a time, reducing
 * memory from O(n*m) to O(min(n, m)) while keeping O(n*m) time.
 */

#include <stdlib.h>
#include <string.h>

/* Return the smaller of two integers. */
static int min_int(int a, int b) {
    return a < b ? a : b;
}

/* Return the smallest of three integers; used for the DP transition. */
static int min3(int a, int b, int c) {
    return min_int(min_int(a, b), c);
}

/*
 * Compute the Levenshtein distance between `source` and `target`.
 *
 * Returns the edit distance, or -1 if a temporary allocation fails. The cost
 * model charges 1 for each insertion, deletion, and substitution. Equal
 * strings yield 0; a string and the empty string yield the longer length.
 * Complexity: O(n*m) time, O(min(n, m)) auxiliary space.
 */
int levenshtein_distance(const char *source, const char *target) {
    int n = (int)strlen(source);
    int m = (int)strlen(target);

    /* Trivial cases avoid allocation entirely. */
    if (n == 0) return m;
    if (m == 0) return n;

    /*
     * Make `target` the shorter axis so the rolling rows are as small as
     * possible. Swapping the operands does not change the distance because
     * Levenshtein distance is symmetric.
     */
    if (m > n) {
        const char *swap = source; source = target; target = swap;
        int t = n; n = m; m = t;
    }

    int *previous = malloc((size_t)(m + 1) * sizeof(int));
    int *current = malloc((size_t)(m + 1) * sizeof(int));
    if (!previous || !current) {
        free(previous);
        free(current);
        return -1;
    }

    /* Row 0: distance from the empty prefix of `source` to each prefix of
       `target` is just the number of insertions, i.e. the column index. */
    for (int j = 0; j <= m; j++) {
        previous[j] = j;
    }

    for (int i = 1; i <= n; i++) {
        current[0] = i;      /* Deleting i characters to reach empty target. */
        for (int j = 1; j <= m; j++) {
            /* No cost if the characters already match; otherwise pay 1. */
            int substitution_cost = (source[i - 1] == target[j - 1]) ? 0 : 1;
            current[j] = min3(
                previous[j] + 1,                 /* deletion from source */
                current[j - 1] + 1,              /* insertion into source */
                previous[j - 1] + substitution_cost /* match or substitute */
            );
        }
        /* Roll the rows: today's `current` becomes tomorrow's `previous`. */
        int *tmp = previous; previous = current; current = tmp;
    }

    int result = previous[m]; /* Final answer sits in the last rolled row. */
    free(previous);
    free(current);
    return result;
}

/*
 * Compute the Hamming distance between two equal-length strings.
 *
 * Returns the number of positions at which the characters differ, or -1 if
 * the strings have different lengths (Hamming distance is undefined then).
 * Complexity: O(n).
 */
int hamming_distance(const char *a, const char *b) {
    int la = (int)strlen(a);
    int lb = (int)strlen(b);
    if (la != lb) return -1; /* Hamming distance requires equal lengths. */

    int differences = 0;
    for (int i = 0; i < la; i++) {
        if (a[i] != b[i]) {
            differences++;
        }
    }
    return differences;
}

/*
 * Test whether `source` and `target` are within a given edit budget.
 *
 * Returns 1 if their Levenshtein distance is <= `max_edits`, else 0. A fast
 * length pre-check rejects pairs whose size gap alone exceeds the budget
 * before doing the full (more expensive) distance computation.
 */
int within_edit_budget(const char *source, const char *target, int max_edits) {
    int n = (int)strlen(source);
    int m = (int)strlen(target);
    int length_gap = n > m ? n - m : m - n;
    if (length_gap > max_edits) {
        return 0;            /* Impossible to bridge with so few edits. */
    }
    int distance = levenshtein_distance(source, target);
    if (distance < 0) return 0; /* Allocation failure treated as "no". */
    return distance <= max_edits;
}
