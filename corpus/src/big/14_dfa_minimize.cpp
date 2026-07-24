/*
 * dfa_minimize.cpp
 *
 * A deterministic finite automaton over a small byte alphabet, plus
 * Hopcroft-style partition refinement to minimize it. Minimization merges
 * states that are behaviorally indistinguishable, producing the smallest DFA
 * that accepts exactly the same language.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* The DFA is dense: 'transitions[s][c]' gives the next state from state s on
 * symbol c, or -1 for a dead/missing transition. State 0 is the start. */
class DFA {
public:
    int numStates;
    int alphabetSize;
    int *transitions;    /* flat numStates * alphabetSize table, -1 = no edge */
    char *accepting;     /* 1 if the state is an accept state, else 0 */

    /*
     * Construct an all-dead DFA with the given dimensions. Every transition
     * starts as -1 and no state accepts; callers fill in the table afterward.
     */
    DFA(int states, int alphabet)
        : numStates(states), alphabetSize(alphabet) {
        transitions = new int[states * alphabet];
        accepting = new char[states];
        for (int i = 0; i < states * alphabet; i++) transitions[i] = -1;
        for (int i = 0; i < states; i++) accepting[i] = 0;
    }

    ~DFA() { delete[] transitions; delete[] accepting; }

    /* Read the target of (state, symbol); returns -1 when no edge exists. */
    int next(int state, int symbol) const {
        return transitions[state * alphabetSize + symbol];
    }

    /* Install an edge from 'state' on 'symbol' to 'target'. */
    void setEdge(int state, int symbol, int target) {
        transitions[state * alphabetSize + symbol] = target;
    }
};

/*
 * Run the DFA on a sequence of already-translated symbols (each in
 * [0, alphabetSize)). Returns 1 if it ends in an accepting state, else 0.
 * A move to a dead state (-1) rejects immediately. O(length) time.
 */
static int dfa_run(const DFA &dfa, const int *symbols, int length) {
    int state = 0;                 /* start state by convention */
    for (int i = 0; i < length; i++) {
        state = dfa.next(state, symbols[i]);
        if (state < 0) return 0;   /* fell into the dead state: reject */
    }
    return dfa.accepting[state];
}

/*
 * Minimize 'src' by partition refinement and return a freshly allocated
 * minimized DFA. The caller owns the result and must 'delete' it.
 *
 * Algorithm: start with two blocks (accepting / non-accepting), then keep
 * splitting any block whose members disagree on which block they reach for
 * some symbol, until no block changes. Each surviving block becomes one state.
 * Runs in O(numStates^2 * alphabetSize) with this simple fixpoint loop.
 */
static DFA *dfa_minimize(const DFA &src) {
    int n = src.numStates;
    int a = src.alphabetSize;

    /* block[s] = id of the partition block that currently contains state s. */
    int *block = new int[n];
    for (int s = 0; s < n; s++) block[s] = src.accepting[s] ? 1 : 0;
    int numBlocks = 2;

    /* Refine to a fixpoint. We compare each pair of states in the same block;
     * if any symbol sends them to different blocks, they must be separated. */
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int b = 0; b < numBlocks; b++) {
            /* Find the first state of block b to act as the block's exemplar. */
            int rep = -1;
            for (int s = 0; s < n; s++)
                if (block[s] == b) { rep = s; break; }
            if (rep < 0) continue;

            int newBlock = -1;     /* lazily allocated id for split-off states */
            for (int s = 0; s < n; s++) {
                if (block[s] != b || s == rep) continue;
                /* Does 's' behave like 'rep' on every symbol? */
                int distinguishable = 0;
                for (int c = 0; c < a; c++) {
                    int ts = src.next(s, c);
                    int tr = src.next(rep, c);
                    int bs = (ts < 0) ? -1 : block[ts];
                    int br = (tr < 0) ? -1 : block[tr];
                    if (bs != br) { distinguishable = 1; break; }
                }
                if (distinguishable) {
                    /* Move 's' into a new block split off from b. */
                    if (newBlock < 0) newBlock = numBlocks++;
                    block[s] = newBlock;
                    changed = 1;
                }
            }
        }
    }

    /* Build the quotient DFA: one state per block. Pick each block's exemplar
     * to read transitions and the accepting flag from. */
    DFA *out = new DFA(numBlocks, a);
    int *exemplar = new int[numBlocks];
    for (int b = 0; b < numBlocks; b++) exemplar[b] = -1;
    for (int s = 0; s < n; s++)
        if (exemplar[block[s]] < 0) exemplar[block[s]] = s;

    for (int b = 0; b < numBlocks; b++) {
        int s = exemplar[b];
        out->accepting[b] = src.accepting[s];
        for (int c = 0; c < a; c++) {
            int t = src.next(s, c);
            out->setEdge(b, c, t < 0 ? -1 : block[t]);
        }
    }

    /* Ensure the start block (the one holding old state 0) is block 0 by
     * convention; if it is not, swap so dfa_run still starts correctly. */
    if (block[0] != 0) {
        int sb = block[0];
        for (int c = 0; c < a; c++) {
            int tmp = out->transitions[0 * a + c];
            out->transitions[0 * a + c] = out->transitions[sb * a + c];
            out->transitions[sb * a + c] = tmp;
        }
        char ta = out->accepting[0];
        out->accepting[0] = out->accepting[sb];
        out->accepting[sb] = ta;
        /* Repoint any edge that referenced the swapped blocks. */
        for (int i = 0; i < numBlocks * a; i++) {
            if (out->transitions[i] == 0) out->transitions[i] = sb;
            else if (out->transitions[i] == sb) out->transitions[i] = 0;
        }
    }

    delete[] block;
    delete[] exemplar;
    return out;
}
