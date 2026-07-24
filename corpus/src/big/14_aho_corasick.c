/*
 * aho_corasick.c
 *
 * Aho-Corasick multi-pattern string matching. The structure is a trie of all
 * keywords augmented with "failure" links, turning it into a finite automaton
 * that scans the text once and reports every keyword occurrence, regardless of
 * how many keywords overlap. Total search time is O(text length + matches).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ALPHABET 256   /* one slot per possible byte value */

/* A single automaton node. 'children' indexes other nodes by byte (-1 means no
 * edge). 'fail' is the failure link followed on a mismatch. 'output' is the id
 * of a keyword that ends here, or -1; 'outLink' chains to shorter keywords that
 * are suffixes of this one so all matches at a position are reported. */
typedef struct ACNode {
    int children[ALPHABET];
    int fail;
    int output;     /* keyword id ending at this node, or -1 */
    int outLink;    /* next node in the dictionary-suffix chain, or -1 */
} ACNode;

/* The whole machine: a growable pool of nodes. Node 0 is the root. */
typedef struct ACMachine {
    ACNode *nodes;
    int     count;
    int     capacity;
} ACMachine;

/*
 * Allocate and zero-initialize a new node, returning its index.
 * Grows the pool geometrically. All child edges start as -1 (absent), and the
 * node has no output and no failure target yet.
 */
static int ac_new_node(ACMachine *m) {
    if (m->count == m->capacity) {
        int newcap = m->capacity ? m->capacity * 2 : 16;
        m->nodes = (ACNode *)realloc(m->nodes, (size_t)newcap * sizeof(ACNode));
        m->capacity = newcap;
    }
    int idx = m->count++;
    ACNode *n = &m->nodes[idx];
    for (int i = 0; i < ALPHABET; i++) n->children[i] = -1;
    n->fail = 0;
    n->output = -1;
    n->outLink = -1;
    return idx;
}

/*
 * Initialize an empty machine with just the root node.
 * Returns 0 on success. The caller must free m->nodes when done.
 */
static int ac_init(ACMachine *m) {
    m->nodes = NULL;
    m->count = 0;
    m->capacity = 0;
    ac_new_node(m);   /* root is node 0 */
    return 0;
}

/*
 * Insert 'keyword' (length 'len') into the trie and tag its final node with
 * 'id'. Walks from the root, creating child nodes for bytes not yet present.
 * Must be called before ac_build_failures. O(len) time.
 */
static void ac_add_keyword(ACMachine *m, const unsigned char *keyword, int len, int id) {
    int node = 0;
    for (int i = 0; i < len; i++) {
        int c = keyword[i];
        if (m->nodes[node].children[c] < 0)
            m->nodes[node].children[c] = ac_new_node(m);
        node = m->nodes[node].children[c];
    }
    m->nodes[node].output = id;   /* this node accepts keyword 'id' */
}

/*
 * Compute failure links and output chains via a breadth-first sweep over the
 * trie. After this call the trie is a complete automaton ready for searching.
 * Uses an explicit array as a FIFO queue. O(total keyword length * alphabet).
 */
static void ac_build_failures(ACMachine *m) {
    int *queue = (int *)malloc((size_t)m->count * sizeof(int));
    int head = 0, tail = 0;

    /* Depth-1 nodes fail straight back to the root; seed the BFS with them. */
    for (int c = 0; c < ALPHABET; c++) {
        int child = m->nodes[0].children[c];
        if (child >= 0) {
            m->nodes[child].fail = 0;
            queue[tail++] = child;
        } else {
            /* Make the root self-looping so a miss at depth 0 stays at root. */
            m->nodes[0].children[c] = 0;
        }
    }

    while (head < tail) {
        int u = queue[head++];
        for (int c = 0; c < ALPHABET; c++) {
            int v = m->nodes[u].children[c];
            if (v < 0) {
                /* Missing edge: redirect it through the failure link so the
                 * automaton can transition on every byte in O(1). */
                m->nodes[u].children[c] = m->nodes[m->nodes[u].fail].children[c];
                continue;
            }
            /* The failure link of v is found by following u's failure link and
             * taking the same byte; the root's edges are now all defined. */
            int f = m->nodes[u].fail;
            m->nodes[v].fail = m->nodes[f].children[c];
            /* Link v's output chain to the nearest ancestor-suffix that is
             * itself a keyword, so all matches ending here are enumerable. */
            int vf = m->nodes[v].fail;
            m->nodes[v].outLink = (m->nodes[vf].output >= 0) ? vf : m->nodes[vf].outLink;
            queue[tail++] = v;
        }
    }
    free(queue);
}

/*
 * Scan 'text' (length 'len') and invoke 'report(id, endPos)' for every keyword
 * occurrence, where endPos is the index one past the last matched byte.
 * Returns the total number of reported matches. Single pass: O(len + matches).
 */
static int ac_search(const ACMachine *m, const unsigned char *text, int len,
                     void (*report)(int id, int endPos)) {
    int node = 0;
    int total = 0;
    for (int i = 0; i < len; i++) {
        /* Every edge is now defined, so a single table lookup advances us. */
        node = m->nodes[node].children[text[i]];

        /* Emit the keyword ending exactly here, then walk the output chain to
         * emit every shorter keyword that is a suffix of the current match. */
        for (int out = node; out > 0; out = m->nodes[out].outLink) {
            if (m->nodes[out].output >= 0) {
                if (report) report(m->nodes[out].output, i + 1);
                total++;
            }
            if (m->nodes[out].outLink == 0) break;
        }
    }
    return total;
}
