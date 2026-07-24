/*
 * Topological sort of a directed acyclic graph using Kahn's algorithm.
 *
 * Given a graph held as a fixed-size adjacency matrix, this module produces
 * an ordering of vertices such that every edge u -> v places u before v. If
 * the graph contains a cycle, no such ordering exists and the sort reports
 * failure.
 */

#include <stdio.h>
#include <string.h>

#define MAX_VERTICES 64

/* A directed graph stored as a dense adjacency matrix. adj[u][v] is 1 when
 * the edge u -> v is present. `count` is the number of vertices in use. */
typedef struct DirectedGraph {
    int count;                              /* active vertices, 0..count-1   */
    int adj[MAX_VERTICES][MAX_VERTICES];    /* 1 = edge present, 0 = absent  */
} DirectedGraph;

/*
 * Initialize a graph to `count` vertices and zero edges.
 * Clamps count into the range [0, MAX_VERTICES]. No return value.
 */
void initGraph(DirectedGraph *graph, int count) {
    if (count < 0)
        count = 0;
    if (count > MAX_VERTICES)
        count = MAX_VERTICES;
    graph->count = count;
    /* memset is correct here because the "no edge" value is all-zero bytes. */
    memset(graph->adj, 0, sizeof(graph->adj));
}

/*
 * Record a directed edge from `from` to `to`.
 * Ignores out-of-range endpoints. Idempotent: adding an existing edge is a
 * no-op. No return value.
 */
void addDirectedEdge(DirectedGraph *graph, int from, int to) {
    if (from < 0 || from >= graph->count)
        return;
    if (to < 0 || to >= graph->count)
        return;
    graph->adj[from][to] = 1;
}

/*
 * Compute the in-degree (number of incoming edges) of every vertex.
 * Writes one entry per vertex into `indegree`, which must hold at least
 * graph->count ints. Runs in O(V^2) over the matrix.
 */
static void computeIndegrees(const DirectedGraph *graph, int *indegree) {
    for (int v = 0; v < graph->count; v++)
        indegree[v] = 0;
    /* For each edge u -> v, the destination v gains one incoming edge. */
    for (int u = 0; u < graph->count; u++) {
        for (int v = 0; v < graph->count; v++) {
            if (graph->adj[u][v])
                indegree[v]++;
        }
    }
}

/*
 * Produce a topological ordering of `graph` into `order`.
 *
 * `order` must have room for graph->count ints; on success it receives the
 * vertices in a valid topological sequence. Uses Kahn's algorithm: repeatedly
 * emit a vertex whose in-degree has dropped to zero and decrement its
 * successors. Runs in O(V^2).
 *
 * Returns the number of vertices ordered. That equals graph->count on
 * success; a smaller value means a cycle was found and `order` holds only the
 * acyclic prefix that could be emitted.
 */
int topologicalSort(const DirectedGraph *graph, int *order) {
    int indegree[MAX_VERTICES];
    int removed[MAX_VERTICES];  /* removed[v] = 1 once v has been emitted */
    computeIndegrees(graph, indegree);
    for (int v = 0; v < graph->count; v++)
        removed[v] = 0;

    int produced = 0;
    /* Each pass emits at least one vertex unless none has in-degree 0, which
     * signals a cycle. At most `count` passes are ever needed. */
    for (int pass = 0; pass < graph->count; pass++) {
        int picked = -1;
        for (int v = 0; v < graph->count; v++) {
            if (!removed[v] && indegree[v] == 0) {
                picked = v;
                break;
            }
        }
        if (picked == -1)
            break;  /* every remaining vertex sits on a cycle */

        order[produced++] = picked;
        removed[picked] = 1;
        /* Removing `picked` deletes its out-edges, lowering successors. */
        for (int v = 0; v < graph->count; v++) {
            if (graph->adj[picked][v] && !removed[v])
                indegree[v]--;
        }
    }
    return produced;
}

/*
 * Report whether the graph is acyclic.
 * Returns 1 if a full topological order exists (no cycle), 0 otherwise.
 * Internally runs a sort and checks that all vertices were emitted.
 */
int isAcyclic(const DirectedGraph *graph) {
    int order[MAX_VERTICES];
    return topologicalSort(graph, order) == graph->count;
}
