/*
 * adjacency_matrix.c
 *
 * An undirected weighted graph stored as a dense adjacency matrix. The matrix
 * is a single contiguous block of integers indexed as matrix[row*n + col],
 * which gives O(1) edge lookups at the cost of O(n^2) memory.
 */

#include <stdio.h>
#include <stdlib.h>

/* Sentinel stored in a cell when no edge connects two vertices. */
#define NO_EDGE 0

/* A graph backed by an n-by-n weight matrix flattened into one array. */
typedef struct MatrixGraph {
    int order;     /* number of vertices (the "order" of the graph) */
    int *cells;    /* order*order weights, row-major */
} MatrixGraph;

/*
 * Allocate an undirected graph with `order` vertices and no edges.
 * All cells start at NO_EDGE. Returns NULL on allocation failure; the
 * caller frees the result with matrixDestroy.
 */
MatrixGraph *matrixCreate(int order) {
    MatrixGraph *g = (MatrixGraph *)malloc(sizeof(MatrixGraph));
    if (g == NULL) {
        return NULL;
    }
    g->order = order;
    /* calloc gives every cell the value 0 (== NO_EDGE) for free. */
    g->cells = (int *)calloc((size_t)order * (size_t)order, sizeof(int));
    if (g->cells == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

/*
 * Set the weight of the undirected edge between a and b. Because the graph is
 * undirected, both symmetric cells (a,b) and (b,a) are written.
 * Returns 0 on success or -1 if either index is out of range.
 */
int matrixSetEdge(MatrixGraph *g, int a, int b, int weight) {
    if (a < 0 || a >= g->order || b < 0 || b >= g->order) {
        return -1;
    }
    g->cells[a * g->order + b] = weight;
    g->cells[b * g->order + a] = weight;  /* keep the matrix symmetric */
    return 0;
}

/*
 * Return the weight stored for edge (a,b), or NO_EDGE when no edge exists.
 * Also returns NO_EDGE for invalid indices so callers can treat "absent" and
 * "out of range" uniformly. Runs in O(1).
 */
int matrixGetWeight(const MatrixGraph *g, int a, int b) {
    if (a < 0 || a >= g->order || b < 0 || b >= g->order) {
        return NO_EDGE;
    }
    return g->cells[a * g->order + b];
}

/*
 * Count the neighbours of a vertex by scanning its row for non-zero cells.
 * Runs in O(order). Returns -1 if the vertex index is invalid.
 */
int matrixDegree(const MatrixGraph *g, int vertex) {
    if (vertex < 0 || vertex >= g->order) {
        return -1;
    }
    int degree = 0;
    const int *row = &g->cells[vertex * g->order];
    for (int col = 0; col < g->order; col++) {
        if (row[col] != NO_EDGE) {
            degree++;
        }
    }
    return degree;
}

/*
 * Sum the weights of every edge in the graph. Each undirected edge appears in
 * two symmetric cells, so the raw cell sum is halved to count it once.
 * Runs in O(order^2).
 */
long matrixTotalWeight(const MatrixGraph *g) {
    long sum = 0;
    int n = g->order;
    for (int i = 0; i < n * n; i++) {
        sum += g->cells[i];
    }
    return sum / 2;  /* undo the double counting from symmetry */
}

/*
 * Release the cell array and the graph struct. A NULL argument is ignored.
 */
void matrixDestroy(MatrixGraph *g) {
    if (g == NULL) {
        return;
    }
    free(g->cells);
    free(g);
}
