/*
 * Adjacency-list graph representation for a directed graph.
 *
 * This module stores a graph as an array of singly linked edge lists, one
 * list per vertex. It supports creating the graph, adding directed edges,
 * computing out-degrees, testing edge existence, and freeing all memory.
 */

#include <stdio.h>
#include <stdlib.h>

/* A single outgoing edge: the destination vertex plus a link to the next
 * edge that leaves the same source vertex. */
typedef struct EdgeNode {
    int dest;               /* index of the vertex this edge points to      */
    struct EdgeNode *next;  /* next edge in this vertex's list, or NULL      */
} EdgeNode;

/* The graph: a fixed number of vertices, each owning a list of edges.
 * head[v] is the first edge leaving vertex v (NULL if v has no out-edges). */
typedef struct Graph {
    int numVertices;        /* total vertex count; valid ids are 0..n-1      */
    EdgeNode **head;        /* array of list heads, length numVertices       */
} Graph;

/*
 * Allocate a graph with `numVertices` vertices and no edges.
 * Returns a heap-allocated Graph the caller must free with destroyGraph,
 * or NULL if numVertices is non-positive or allocation fails.
 */
Graph *createGraph(int numVertices) {
    if (numVertices <= 0)
        return NULL;
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (graph == NULL)
        return NULL;
    graph->numVertices = numVertices;
    /* calloc zero-fills, so every list head starts out empty (NULL). */
    graph->head = (EdgeNode **)calloc((size_t)numVertices, sizeof(EdgeNode *));
    if (graph->head == NULL) {
        free(graph);
        return NULL;
    }
    return graph;
}

/*
 * Add a directed edge src -> dest by prepending a new node to src's list.
 * Prepending keeps insertion at O(1); list order is therefore reverse of
 * insertion order, which callers must not rely on.
 * Returns 1 on success, 0 if either endpoint is out of range or malloc fails.
 */
int addEdge(Graph *graph, int src, int dest) {
    if (graph == NULL)
        return 0;
    /* Reject endpoints outside the valid vertex range. */
    if (src < 0 || src >= graph->numVertices)
        return 0;
    if (dest < 0 || dest >= graph->numVertices)
        return 0;
    EdgeNode *node = (EdgeNode *)malloc(sizeof(EdgeNode));
    if (node == NULL)
        return 0;
    node->dest = dest;
    node->next = graph->head[src];  /* old head becomes second element */
    graph->head[src] = node;
    return 1;
}

/*
 * Count the outgoing edges of `vertex` by walking its list.
 * Runs in O(out-degree). Returns -1 if the vertex id is invalid.
 */
int outDegree(const Graph *graph, int vertex) {
    if (graph == NULL || vertex < 0 || vertex >= graph->numVertices)
        return -1;
    int count = 0;
    for (EdgeNode *cur = graph->head[vertex]; cur != NULL; cur = cur->next)
        count++;
    return count;
}

/*
 * Test whether the directed edge src -> dest exists.
 * Returns 1 if found, 0 if absent or if either endpoint is invalid.
 * Runs in O(out-degree of src).
 */
int hasEdge(const Graph *graph, int src, int dest) {
    if (graph == NULL || src < 0 || src >= graph->numVertices)
        return 0;
    for (EdgeNode *cur = graph->head[src]; cur != NULL; cur = cur->next) {
        if (cur->dest == dest)
            return 1;
    }
    return 0;
}

/*
 * Free every edge node, the head array, and the graph struct itself.
 * Safe to call on NULL. After this call the pointer must not be used.
 */
void destroyGraph(Graph *graph) {
    if (graph == NULL)
        return;
    for (int v = 0; v < graph->numVertices; v++) {
        EdgeNode *cur = graph->head[v];
        while (cur != NULL) {
            EdgeNode *doomed = cur;  /* save before advancing or we lose it */
            cur = cur->next;
            free(doomed);
        }
    }
    free(graph->head);
    free(graph);
}
