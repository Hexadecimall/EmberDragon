/*
 * thompson_nfa.c
 *
 * A Thompson-construction NFA for a tiny regular-expression language.
 * The matcher simulates the NFA directly over an input string by tracking
 * the *set* of states reachable at each step (subset simulation), which
 * avoids backtracking and runs in O(states * input) time.
 *
 * Supported syntax: literal bytes, '.' (any byte), '*' (zero-or-more on the
 * previous atom), and concatenation. This is intentionally small so the
 * whole engine fits in one file and stays easy to reason about.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A single NFA state. Each state has up to two outgoing epsilon-or-symbol
 * edges. 'symbol' < 0 means an epsilon transition (no input consumed);
 * symbol == ANY matches any byte; otherwise it matches exactly that byte. */
#define EPSILON (-1)
#define ANY     (-2)
#define MATCH   (-3)   /* terminal accepting state: no outgoing edges */

typedef struct State {
    int symbol;          /* EPSILON, ANY, MATCH, or a byte value 0..255 */
    int out1;            /* index of first successor, or -1 */
    int out2;            /* index of second successor, or -1 */
} State;

/* The compiled program: a flat array of states plus the start index. */
typedef struct NFA {
    State *states;
    int    count;        /* number of states currently allocated */
    int    capacity;     /* allocated slots in 'states' */
    int    start;        /* index of the start state */
} NFA;

/*
 * Append a fresh state to the NFA, growing the backing array if needed.
 * Returns the index of the new state. The caller fills in its fields after.
 */
static int nfa_add_state(NFA *nfa, int symbol, int out1, int out2) {
    if (nfa->count == nfa->capacity) {
        int newcap = nfa->capacity ? nfa->capacity * 2 : 8;
        nfa->states = (State *)realloc(nfa->states, (size_t)newcap * sizeof(State));
        nfa->capacity = newcap;
    }
    int idx = nfa->count++;
    nfa->states[idx].symbol = symbol;
    nfa->states[idx].out1 = out1;
    nfa->states[idx].out2 = out2;
    return idx;
}

/*
 * Compile a pattern string into an NFA using a left-to-right scan.
 * Each atom becomes one state; a trailing '*' wraps the previous atom in a
 * split loop. Returns 0 on success and fills 'nfa'; the caller must free
 * nfa->states. The construction links states forward by index, so we patch
 * the previous atom's "out" pointer as we go.
 */
static int nfa_compile(NFA *nfa, const char *pattern) {
    memset(nfa, 0, sizeof(*nfa));
    int prev = -1;           /* index of the atom just emitted, for '*' */
    int prev_link = -1;      /* address (state index) whose out1 chains forward */

    for (const char *p = pattern; *p; p++) {
        if (*p == '*') {
            /* '*' must follow a real atom; ignore a leading/dangling star. */
            if (prev < 0) continue;
            /* Insert a split state in front of the previous atom: it can skip
             * the atom (zero matches) or enter it, and the atom loops back. */
            int split = nfa_add_state(nfa, EPSILON, prev, -1);
            nfa->states[prev].out1 = split;   /* atom loops back to the split */
            if (prev_link >= 0)
                nfa->states[prev_link].out1 = split;
            else
                nfa->start = split;
            prev_link = split;                /* future atoms chain off split.out2 */
            prev = -1;                          /* a star cannot follow a star */
            continue;
        }
        /* A normal atom: '.' becomes ANY, anything else a literal byte. */
        int sym = (*p == '.') ? ANY : (unsigned char)*p;
        int s = nfa_add_state(nfa, sym, -1, -1);
        if (prev_link >= 0)
            nfa->states[prev_link].out1 = s;
        else if (nfa->start == 0 && nfa->count == 1)
            nfa->start = s;                    /* very first atom is the start */
        else
            nfa->states[prev_link].out1 = s;
        /* For a split we chain successors via out2; otherwise via out1. */
        prev = s;
        prev_link = s;
    }

    /* Terminate the chain with a single MATCH state. */
    int m = nfa_add_state(nfa, MATCH, -1, -1);
    if (prev_link >= 0)
        nfa->states[prev_link].out1 = m;
    else
        nfa->start = m;                        /* empty pattern matches empty */
    return 0;
}

/*
 * Add a state index to the current frontier, following epsilon and split
 * edges so the set always holds only "real" (symbol-consuming or MATCH)
 * states. 'seen' marks visited states to prevent infinite epsilon loops.
 */
static void add_state(const NFA *nfa, int idx, int *list, int *n, char *seen) {
    if (idx < 0 || seen[idx]) return;
    seen[idx] = 1;
    const State *s = &nfa->states[idx];
    if (s->symbol == EPSILON) {
        /* Epsilon closure: expand both successors without consuming input. */
        add_state(nfa, s->out1, list, n, seen);
        add_state(nfa, s->out2, list, n, seen);
    } else {
        list[(*n)++] = idx;   /* a state that actually waits for a byte (or MATCH) */
    }
}

/*
 * Run the NFA against 'input' and return 1 if it matches the entire string,
 * 0 otherwise. Uses two frontiers (current/next) and the epsilon-closure
 * helper. Runs in O(count * length) time and O(count) extra space.
 */
static int nfa_match(const NFA *nfa, const char *input) {
    int *cur  = (int *)malloc((size_t)nfa->count * sizeof(int));
    int *next = (int *)malloc((size_t)nfa->count * sizeof(int));
    char *seen = (char *)malloc((size_t)nfa->count);
    int ncur = 0;

    memset(seen, 0, (size_t)nfa->count);
    add_state(nfa, nfa->start, cur, &ncur, seen);

    for (const char *p = input; *p; p++) {
        int nnext = 0;
        memset(seen, 0, (size_t)nfa->count);
        for (int i = 0; i < ncur; i++) {
            const State *s = &nfa->states[cur[i]];
            /* A state advances only if its symbol accepts this byte. */
            if (s->symbol == ANY || s->symbol == (unsigned char)*p)
                add_state(nfa, s->out1, next, &nnext, seen);
        }
        /* Swap frontiers: 'next' becomes the new current set. */
        int *tmp = cur; cur = next; next = tmp;
        ncur = nnext;
    }

    /* Accept iff a MATCH state survived in the final frontier. */
    int accepted = 0;
    for (int i = 0; i < ncur; i++)
        if (nfa->states[cur[i]].symbol == MATCH) { accepted = 1; break; }

    free(cur); free(next); free(seen);
    return accepted;
}
