/*
 * levenshtein_automaton.c
 *
 * A bounded-edit-distance matcher built around the Levenshtein automaton idea,
 * implemented with the classic row-at-a-time dynamic-programming recurrence.
 * Given a fixed pattern and an edit budget k, it answers "is the Levenshtein
 * distance between pattern and a candidate word at most k?" without ever
 * filling the full DP matrix for words that can be rejected early.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* The matcher's configuration. The DP rows are sized to the pattern length,
 * which is the dimension that stays constant across many candidate words. */
typedef struct LevAutomaton {
    const char *pattern;   /* the word we are matching candidates against */
    int         plen;      /* cached strlen(pattern) */
    int         maxEdits;  /* the edit-distance budget k */
    int        *prev;      /* DP row for the previous input character */
    int        *cur;       /* DP row currently being computed */
} LevAutomaton;

/* Return the smaller of two ints. Used to combine the three edit operations. */
static int min2(int a, int b) { return a < b ? a : b; }

/* Return the smallest of three ints (insertion, deletion, substitution). */
static int min3(int a, int b, int c) { return min2(min2(a, b), c); }

/*
 * Initialize an automaton for 'pattern' with edit budget 'maxEdits'.
 * Allocates two DP rows of length plen+1. Returns 0 on success, -1 on
 * allocation failure. The caller must later call lev_free to release memory.
 */
static int lev_init(LevAutomaton *aut, const char *pattern, int maxEdits) {
    aut->pattern = pattern;
    aut->plen = (int)strlen(pattern);
    aut->maxEdits = maxEdits;
    aut->prev = (int *)malloc((size_t)(aut->plen + 1) * sizeof(int));
    aut->cur  = (int *)malloc((size_t)(aut->plen + 1) * sizeof(int));
    if (!aut->prev || !aut->cur) {
        free(aut->prev); free(aut->cur);
        aut->prev = aut->cur = NULL;
        return -1;
    }
    return 0;
}

/* Release the DP rows held by the automaton. Safe to call once after lev_init. */
static void lev_free(LevAutomaton *aut) {
    free(aut->prev);
    free(aut->cur);
    aut->prev = aut->cur = NULL;
}

/*
 * Test whether 'candidate' is within the automaton's edit budget of the
 * pattern. Returns 1 if the Levenshtein distance is <= maxEdits, else 0.
 *
 * Optimization: after each input character we track the minimum value in the
 * current DP row. Because each step changes any cell by at most 1, once the
 * row minimum exceeds maxEdits no later row can drop back to within budget,
 * so we bail out early. Worst case O(plen * len(candidate)).
 */
static int lev_accepts(LevAutomaton *aut, const char *candidate) {
    int plen = aut->plen;

    /* Row 0: distance from the empty input prefix to each pattern prefix is
     * just the number of deletions, i.e. the column index. */
    for (int j = 0; j <= plen; j++) aut->prev[j] = j;

    int row = 1;
    for (const char *c = candidate; *c; c++, row++) {
        aut->cur[0] = row;             /* deleting 'row' input chars to reach "" */
        int rowMin = aut->cur[0];

        for (int j = 1; j <= plen; j++) {
            /* Substitution cost is 0 when the characters already agree. */
            int subCost = (aut->pattern[j - 1] == *c) ? 0 : 1;
            int del = aut->prev[j] + 1;          /* drop an input character */
            int ins = aut->cur[j - 1] + 1;       /* insert a pattern character */
            int sub = aut->prev[j - 1] + subCost;/* match or substitute */
            aut->cur[j] = min3(del, ins, sub);
            if (aut->cur[j] < rowMin) rowMin = aut->cur[j];
        }

        /* Early exit: the whole row is already beyond budget, and rows can
         * only grow from here, so no acceptance is possible. */
        if (rowMin > aut->maxEdits) return 0;

        /* Reuse buffers by swapping rather than copying. */
        int *tmp = aut->prev; aut->prev = aut->cur; aut->cur = tmp;
    }

    /* The final cell holds the full edit distance between the two words. */
    return aut->prev[plen] <= aut->maxEdits;
}

/*
 * Filter a dictionary, returning how many words lie within the edit budget of
 * the pattern, and writing those words' indices into 'matches' (capacity
 * 'cap'). Indices beyond 'cap' are counted but not stored. Returns the total
 * count of matching words. O(sum of word lengths * plen).
 */
static int lev_search(LevAutomaton *aut, const char *const *words, int count,
                      int *matches, int cap) {
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (lev_accepts(aut, words[i])) {
            if (found < cap) matches[found] = i;
            found++;
        }
    }
    return found;
}
