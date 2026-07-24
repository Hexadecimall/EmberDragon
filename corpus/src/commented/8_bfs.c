/*
 * Breadth-first search over an adjacency-list graph.
 *
 * The graph is undirected: each call to connectVertices records the edge in
 * both directions. BFS explores level by level from a source vertex using a
 * ring-buffer queue, recording each reachable vertex's hop distance and the
 * predecessor that discovered it so shortest unweighted paths can be rebuilt.
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_NODES 128

/* A neighbour entry in a vertex's adjacency list. */
typedef struct Neighbor {
    int vertex;              /* the adjacent vertex id                  */
    struct Neighbor *next;   /* next neighbour of the same owner, or NULL */
} Neighbor;

/* Undirected graph as an array of adjacency lists. */
typedef struct AdjGraph {
    int vertexCount;                 /* active vertices, ids 0..count-1   */
    Neighbor *lists[MAX_NODES];      /* lists[v] heads v's neighbour list */
} AdjGraph;

/*
 * Initialize a graph to `vertexCount` vertices and no edges.
 * Clamps the count into [0, MAX_NODES]. Sets every adjacency list empty.
 */
void graphReset(AdjGraph *graph, int vertexCount) {
    if (vertexCount < 0)
        vertexCount = 0;
    if (vertexCount > MAX_NODES)
        vertexCount = MAX_NODES;
    graph->vertexCount = vertexCount;
    for (int v = 0; v < vertexCount; v++)
        graph->lists[v] = NULL;
}

/*
 * Prepend `to` to `from`'s adjacency list.
 * Internal helper used by connectVertices. Returns 1 on success, 0 if malloc
 * fails. Does no range checking; the caller validates endpoints first.
 */
static int linkNeighbor(AdjGraph *graph, int from, int to) {
    Neighbor *node = (Neighbor *)malloc(sizeof(Neighbor));
    if (node == NULL)
        return 0;
    node->vertex = to;
    node->next = graph->lists[from];
    graph->lists[from] = node;
    return 1;
}

/*
 * Connect two vertices with an undirected edge by linking each into the
 * other's list. Returns 1 on success, 0 if endpoints are invalid or an
 * allocation fails. Self-loops (a == b) are rejected.
 */
int connectVertices(AdjGraph *graph, int a, int b) {
    if (a < 0 || a >= graph->vertexCount)
        return 0;
    if (b < 0 || b >= graph->vertexCount)
        return 0;
    if (a == b)
        return 0;  /* a self-loop adds nothing useful for BFS */
    if (!linkNeighbor(graph, a, b))
        return 0;
    if (!linkNeighbor(graph, b, a))
        return 0;
    return 1;
}

/*
 * Run breadth-first search from `source`.
 *
 * Fills `dist` with each vertex's hop count from source (-1 if unreachable)
 * and `parent` with the vertex that first discovered it (-1 for the source
 * and for unreachable vertices). Both arrays must hold vertexCount ints.
 * Runs in O(V + E).
 *
 * Returns the number of vertices reached (including the source), or -1 if
 * source is out of range.
 */
int breadthFirstSearch(const AdjGraph *graph, int source,
                       int *dist, int *parent) {
    if (source < 0 || source >= graph->vertexCount)
        return -1;

    /* A simple ring buffer; capacity equals the vertex count because each
     * vertex is enqueued at most once. */
    int queue[MAX_NODES];
    int head = 0, tail = 0;

    for (int v = 0; v < graph->vertexCount; v++) {
        dist[v] = -1;       /* -1 marks "not yet visited" */
        parent[v] = -1;
    }

    dist[source] = 0;
    queue[tail++] = source;
    int reached = 1;

    while (head != tail) {
        int current = queue[head++];
        /* Visit each neighbour exactly once: the dist == -1 test guards it. */
        for (Neighbor *n = graph->lists[current]; n != NULL; n = n->next) {
            if (dist[n->vertex] == -1) {
                dist[n->vertex] = dist[current] + 1;
                parent[n->vertex] = current;
                queue[tail++] = n->vertex;
                reached++;
            }
        }
    }
    return reached;
}

/*
 * Free every neighbour node and empty all adjacency lists.
 * The graph struct itself is caller-owned and is not freed. Safe to call on
 * NULL.
 */
void graphFreeEdges(AdjGraph *graph) {
    if (graph == NULL)
        return;
    for (int v = 0; v < graph->vertexCount; v++) {
        Neighbor *cur = graph->lists[v];
        while (cur != NULL) {
            Neighbor *doomed = cur;
            cur = cur->next;
            free(doomed);
        }
        graph->lists[v] = NULL;
    }
}
