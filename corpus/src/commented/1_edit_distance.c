/*
 * edit_distance.c
 *
 * Levenshtein edit distance: the minimum number of single-character
 * insertions, deletions, or substitutions needed to turn one string into
 * another. Implemented with a space-optimized dynamic-programming pass that
 * keeps only two rows of the cost matrix.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Return the smaller of two integers. Trivial helper kept separate so the
 * three-way minimum in the DP step reads clearly.
 */
static int minInt(int a, int b) {
    return (a < b) ? a : b;
}

/*
 * Return the minimum of three integers. Used to pick the cheapest of the
 * insert / delete / substitute options at each matrix cell.
 */
static int minOfThree(int a, int b, int c) {
    return minInt(minInt(a, b), c);
}

/*
 * Compute the Levenshtein distance between `source` and `target`.
 * Returns the edit distance as a non-negative integer, or -1 if a needed
 * allocation fails. Runs in O(n*m) time but only O(min(n, m)) extra space
 * because just the previous and current matrix rows are retained.
 */
int editDistance(const char *source, const char *target) {
    int sourceLen = (int)strlen(source);
    int targetLen = (int)strlen(target);

    /* Distance to/from an empty string is the length of the other string. */
    if (sourceLen == 0) {
        return targetLen;
    }
    if (targetLen == 0) {
        return sourceLen;
    }

    /* Index the DP across `target`, so the rows are (targetLen + 1) wide. */
    int width = targetLen + 1;
    int *previousRow = malloc((size_t)width * sizeof(int));
    int *currentRow  = malloc((size_t)width * sizeof(int));
    if (previousRow == NULL || currentRow == NULL) {
        free(previousRow);
        free(currentRow);
        return -1;
    }

    /* Row 0: transforming an empty source prefix into target[0..j] costs j
     * insertions. */
    for (int j = 0; j <= targetLen; j++) {
        previousRow[j] = j;
    }

    for (int i = 1; i <= sourceLen; i++) {
        /* Column 0: deleting i characters to reach an empty target prefix. */
        currentRow[0] = i;

        for (int j = 1; j <= targetLen; j++) {
            /* No cost when the characters already agree; otherwise pay 1. */
            int substitutionCost = (source[i - 1] == target[j - 1]) ? 0 : 1;

            int deletion     = previousRow[j] + 1;                  /* drop source char */
            int insertion    = currentRow[j - 1] + 1;              /* add target char  */
            int substitution = previousRow[j - 1] + substitutionCost;

            currentRow[j] = minOfThree(deletion, insertion, substitution);
        }

        /* The current row becomes the previous row for the next iteration. */
        int *swap   = previousRow;
        previousRow = currentRow;
        currentRow  = swap;
    }

    /* After the final swap the answer sits in previousRow's last cell. */
    int distance = previousRow[targetLen];

    free(previousRow);
    free(currentRow);
    return distance;
}

/*
 * Return 1 if `source` can be turned into `target` with at most `maxEdits`
 * operations, 0 otherwise. A quick length-difference check rejects pairs that
 * are obviously too far apart before the full O(n*m) computation runs.
 */
int withinEditDistance(const char *source, const char *target, int maxEdits) {
    int lengthGap = (int)strlen(source) - (int)strlen(target);
    if (lengthGap < 0) {
        lengthGap = -lengthGap;  /* absolute difference in lengths */
    }

    /* Even an all-substitution path needs at least `lengthGap` edits, so if
     * that alone exceeds the budget we can answer without computing. */
    if (lengthGap > maxEdits) {
        return 0;
    }

    int distance = editDistance(source, target);
    if (distance < 0) {
        return 0;  /* treat allocation failure as "not within" */
    }
    return distance <= maxEdits;
}
