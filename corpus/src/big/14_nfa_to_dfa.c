/*
 * nfa_to_dfa.c
 *
 * Subset construction: convert a nondeterministic finite automaton (NFA) into
 * an equivalent deterministic one (DFA). Each DFA state corresponds to a set
 * of NFA states reachable on the same input; the conversion materializes those
 * sets lazily, discovering new DFA states as transitions require them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_NFA_STATES 64    /* lets a state set fit in one uint64_t bitmask */
#define SYMBOLS         128  /* ASCII alphabet */

/* An NFA with epsilon-free transitions for clarity: trans[s][c] is a bitmask
 * of states reachable from s on symbol c. accept[s] flags accepting states. */
typedef struct NFA {
    uint64_t trans[MAX_NFA_STATES][SYMBOLS];
    uint64_t accept;     /* bitmask: bit s set if NFA state s accepts */
    int      numStates;
    int      start;      /* start state index */
} NFA;

/* The resulting DFA. Each DFA state stores the NFA-state set it represents,
 * indexed transitions, and whether it is accepting. */
typedef struct DFA {
    uint64_t stateSet[256]; /* the NFA-set defining each DFA state */
    int      trans[256][SYMBOLS];
    char     accept[256];
    int      numStates;
} DFA;

/*
 * Compute the union of NFA states reachable from any state in 'set' on symbol
 * 'c'. Returns the resulting bitmask (possibly 0 if 'c' is a dead transition).
 * Iterates only over the set bits, so cost is proportional to |set|.
 */
static uint64_t nfa_move(const NFA *nfa, uint64_t set, int c) {
    uint64_t result = 0;
    for (int s = 0; s < nfa->numStates; s++) {
        /* Skip states not in the current set without touching their tables. */
        if (set & ((uint64_t)1 << s))
            result |= nfa->trans[s][c];
    }
    return result;
}

/*
 * Look up the DFA-state index whose NFA-set equals 'set', registering it as a
 * new DFA state if it has not been seen before. Returns the index, or -1 if
 * the DFA-state table is full. Linear scan over existing states keeps the data
 * structure simple at the cost of O(numStates) per query.
 */
static int dfa_intern(DFA *dfa, uint64_t set) {
    for (int i = 0; i < dfa->numStates; i++)
        if (dfa->stateSet[i] == set)
            return i;                 /* already interned */
    if (dfa->numStates >= 256)
        return -1;                    /* out of DFA-state capacity */
    int idx = dfa->numStates++;
    dfa->stateSet[idx] = set;
    return idx;
}

/*
 * Run subset construction, filling 'dfa' from 'nfa'. Returns the number of DFA
 * states produced, or -1 if the DFA exceeds the 256-state cap. A worklist of
 * unprocessed DFA states drives the loop; each is expanded over every symbol.
 * Worst-case exponential in NFA size (inherent to the problem), but typically
 * far smaller for real automata.
 */
static int nfa_to_dfa(const NFA *nfa, DFA *dfa) {
    dfa->numStates = 0;

    /* The DFA start state is the singleton set holding the NFA start state. */
    uint64_t startSet = (uint64_t)1 << nfa->start;
    int start = dfa_intern(dfa, startSet);
    if (start < 0) return -1;

    /* Process DFA states in creation order; new states get appended past the
     * cursor, so a single forward index serves as the worklist. */
    for (int i = 0; i < dfa->numStates; i++) {
        uint64_t set = dfa->stateSet[i];

        /* A DFA state accepts iff its set contains any accepting NFA state. */
        dfa->accept[i] = (set & nfa->accept) ? 1 : 0;

        for (int c = 0; c < SYMBOLS; c++) {
            uint64_t target = nfa_move(nfa, set, c);
            if (target == 0) {
                dfa->trans[i][c] = -1;     /* no move: dead transition */
                continue;
            }
            int t = dfa_intern(dfa, target);
            if (t < 0) return -1;          /* ran out of room */
            dfa->trans[i][c] = t;
        }
    }
    return dfa->numStates;
}

/*
 * Execute the constructed DFA on 'input' and return 1 if it ends in an
 * accepting state, else 0. A -1 transition (dead state) rejects immediately.
 * Bytes >= SYMBOLS are outside the alphabet and also reject. O(length) time.
 */
static int dfa_accepts(const DFA *dfa, const char *input) {
    int state = 0;                 /* DFA start is always interned first */
    for (const unsigned char *p = (const unsigned char *)input; *p; p++) {
        if (*p >= SYMBOLS) return 0;          /* symbol outside alphabet */
        state = dfa->trans[state][*p];
        if (state < 0) return 0;              /* entered the dead state */
    }
    return dfa->accept[state];
}
