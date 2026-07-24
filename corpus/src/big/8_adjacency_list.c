/*
 * adjacency_list.c
 *
 * A directed graph represented as an adjacency list: each vertex owns a
 * singly linked list of outgoing edges. This layout is memory-efficient for
 * sparse graphs and gives O(degree) iteration over a vertex's neighbours.
 */

#include <stdio.h>
#include <stdlib.h>

/* One outgoing edge: the destination vertex plus a link to the next edge. */
typedef struct Edge {
    int dest;            /* index of the vertex this edge points to */
    int weight;          /* edge cost; 1 for an unweighted graph */
    struct Edge *next;   /* next edge in this vertex's list, or NULL */
} Edge;

/* A graph holding a fixed number of vertices, each with an edge list head. */
typedef struct Graph {
    int vertexCount;     /* total number of vertices, indexed 0..vertexCount-1 */
    Edge **heads;        /* array of list heads, one per vertex */
} Graph;

/*
 * Allocate a graph with the given number of vertices and no edges.
 * Returns a heap-allocated Graph the caller must free with graphDestroy,
 * or NULL if allocation fails.
 */
Graph *graphCreate(int vertexCount) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (g == NULL) {
        return NULL;
    }
    g->vertexCount = vertexCount;
    /* calloc zero-initialises every head pointer to NULL (empty lists). */
    g->heads = (Edge **)calloc((size_t)vertexCount, sizeof(Edge *));
    if (g->heads == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

/*
 * Insert a directed edge src -> dest with the given weight.
 * The new edge is prepended to src's list, so insertion is O(1).
 * Returns 0 on success, or -1 on bad indices or allocation failure.
 */
int graphAddEdge(Graph *g, int src, int dest, int weight) {
    if (src < 0 || src >= g->vertexCount ||
        dest < 0 || dest >= g->vertexCount) {
        return -1;  /* reject out-of-range endpoints */
    }
    Edge *edge = (Edge *)malloc(sizeof(Edge));
    if (edge == NULL) {
        return -1;
    }
    edge->dest = dest;
    edge->weight = weight;
    /* Prepend: the old head becomes our successor. */
    edge->next = g->heads[src];
    g->heads[src] = edge;
    return 0;
}

/*
 * Return 1 if a directed edge src -> dest exists, else 0.
 * Runs in O(degree(src)) by scanning src's edge list.
 */
int graphHasEdge(const Graph *g, int src, int dest) {
    if (src < 0 || src >= g->vertexCount) {
        return 0;
    }
    for (Edge *e = g->heads[src]; e != NULL; e = e->next) {
        if (e->dest == dest) {
            return 1;
        }
    }
    return 0;
}

/*
 * Compute the out-degree (number of outgoing edges) of a vertex.
 * Returns -1 if the vertex index is invalid.
 */
int graphOutDegree(const Graph *g, int vertex) {
    if (vertex < 0 || vertex >= g->vertexCount) {
        return -1;
    }
    int degree = 0;
    for (Edge *e = g->heads[vertex]; e != NULL; e = e->next) {
        degree++;
    }
    return degree;
}

/*
 * Free every edge of every vertex, then the head array and the graph itself.
 * Safe to call on a NULL pointer, which is treated as a no-op.
 */
void graphDestroy(Graph *g) {
    if (g == NULL) {
        return;
    }
    for (int v = 0; v < g->vertexCount; v++) {
        Edge *e = g->heads[v];
        while (e != NULL) {
            Edge *doomed = e;   /* hold the current node before advancing */
            e = e->next;
            free(doomed);
        }
    }
    free(g->heads);
    free(g);
}
