/*
 * mst_prim.c
 *
 * Prim's algorithm for the minimum spanning tree of a connected undirected
 * weighted graph given as a dense adjacency matrix. It grows the tree one
 * vertex at a time, always adding the cheapest edge that crosses the frontier.
 */

#include <stdio.h>
#include <stdlib.h>

/* Sentinel weight meaning "no edge" between two vertices. */
#define NO_EDGE 0

/* A large stand-in for infinity used for the cheapest-connecting-edge cost. */
#define COST_INFINITY 0x3fffffff

/* Undirected weighted graph stored as a flattened symmetric weight matrix. */
typedef struct DenseGraph {
    int n;        /* number of vertices */
    int *weight;  /* n*n weights, row-major; NO_EDGE where absent */
} DenseGraph;

/*
 * Create an n-vertex graph with no edges. Returns NULL on failure.
 * Caller frees with denseDestroy.
 */
DenseGraph *denseCreate(int n) {
    DenseGraph *g = (DenseGraph *)malloc(sizeof(DenseGraph));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->weight = (int *)calloc((size_t)n * (size_t)n, sizeof(int));
    if (g->weight == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

/*
 * Set the symmetric weight of the undirected edge (a,b).
 * Returns 0, or -1 on out-of-range indices.
 */
int denseSetEdge(DenseGraph *g, int a, int b, int w) {
    if (a < 0 || a >= g->n || b < 0 || b >= g->n) {
        return -1;
    }
    g->weight[a * g->n + b] = w;
    g->weight[b * g->n + a] = w;
    return 0;
}

/*
 * Build a minimum spanning tree rooted at vertex 0 using Prim's algorithm.
 *
 * For each non-root vertex v, parent[v] is set to the tree vertex it attaches
 * to (parent[0] is -1). The `parent` array must hold g->n ints.
 *
 * Returns the total MST weight on success, or -1 if the graph is disconnected
 * (some vertex can never be reached) or on allocation failure. Runs in O(n^2),
 * which suits the dense adjacency-matrix representation.
 */
long primMST(const DenseGraph *g, int *parent) {
    int n = g->n;
    int *key = (int *)malloc((size_t)n * sizeof(int));     /* cheapest edge to tree */
    int *inTree = (int *)malloc((size_t)n * sizeof(int));  /* membership flags */
    if (key == NULL || inTree == NULL) {
        free(key);
        free(inTree);
        return -1;
    }

    /* Every vertex starts infinitely far from the (empty) tree. */
    for (int v = 0; v < n; v++) {
        key[v] = COST_INFINITY;
        inTree[v] = 0;
        parent[v] = -1;
    }
    key[0] = 0;  /* seed the tree at vertex 0 with zero cost */

    long totalWeight = 0;
    for (int iter = 0; iter < n; iter++) {
        /* Pick the non-tree vertex with the smallest connecting-edge cost. */
        int best = -1;
        int bestKey = COST_INFINITY;
        for (int v = 0; v < n; v++) {
            if (!inTree[v] && key[v] < bestKey) {
                bestKey = key[v];
                best = v;
            }
        }
        if (best == -1) {
            /* No reachable vertex remains: the graph is disconnected. */
            free(key);
            free(inTree);
            return -1;
        }

        inTree[best] = 1;        /* commit `best` to the growing tree */
        totalWeight += key[best];

        /* Relax: cheaper edges from `best` may improve frontier vertices. */
        const int *row = &g->weight[best * n];
        for (int v = 0; v < n; v++) {
            int w = row[v];
            if (w != NO_EDGE && !inTree[v] && w < key[v]) {
                key[v] = w;
                parent[v] = best;  /* record how v would join the tree */
            }
        }
    }

    free(key);
    free(inTree);
    return totalWeight;
}

/*
 * Free the weight matrix and the graph struct. NULL is ignored.
 */
void denseDestroy(DenseGraph *g) {
    if (g == NULL) {
        return;
    }
    free(g->weight);
    free(g);
}
