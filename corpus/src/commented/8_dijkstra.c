/*
 * Dijkstra's single-source shortest-path algorithm on a small graph with
 * non-negative integer edge weights, stored as a dense adjacency matrix.
 *
 * Distances are plain ints; an absent edge or unreachable vertex is marked
 * with a sentinel "infinity". This implementation uses a linear scan to pick
 * the next closest vertex, which is ideal for the small graphs it targets.
 */

#include <stdio.h>

#define MAX_NODES 32
#define INF 1000000000   /* stand-in for infinity; safe from int overflow */

/* Weighted directed graph. weight[u][v] holds the cost of edge u -> v, or
 * INF when no such edge exists. `nodeCount` is the number of vertices. */
typedef struct WeightedGraph {
    int nodeCount;                          /* active vertices, 0..count-1   */
    int weight[MAX_NODES][MAX_NODES];       /* edge cost or INF if no edge   */
} WeightedGraph;

/*
 * Initialize a graph with `nodeCount` vertices and no edges.
 * Clamps nodeCount into [0, MAX_NODES]. Every off-diagonal weight starts at
 * INF; the diagonal is 0 since a vertex reaches itself at zero cost.
 */
void graphInit(WeightedGraph *graph, int nodeCount) {
    if (nodeCount < 0)
        nodeCount = 0;
    if (nodeCount > MAX_NODES)
        nodeCount = MAX_NODES;
    graph->nodeCount = nodeCount;
    for (int u = 0; u < nodeCount; u++) {
        for (int v = 0; v < nodeCount; v++)
            graph->weight[u][v] = (u == v) ? 0 : INF;
    }
}

/*
 * Add or update a directed edge from -> to with the given non-negative cost.
 * Ignores invalid endpoints or a negative cost (Dijkstra requires weights
 * >= 0). A later call to the same endpoints overwrites the earlier weight.
 */
void setEdge(WeightedGraph *graph, int from, int to, int cost) {
    if (from < 0 || from >= graph->nodeCount)
        return;
    if (to < 0 || to >= graph->nodeCount)
        return;
    if (cost < 0)
        return;  /* negative weights would break Dijkstra's correctness */
    graph->weight[from][to] = cost;
}

/*
 * Select the unvisited vertex with the smallest tentative distance.
 * Scans all vertices in O(V). Returns its index, or -1 when every remaining
 * vertex is unreachable (distance INF) or already visited.
 */
static int pickClosest(const WeightedGraph *graph, const int *dist,
                       const int *visited) {
    int best = -1;
    int bestDist = INF;
    for (int v = 0; v < graph->nodeCount; v++) {
        if (!visited[v] && dist[v] < bestDist) {
            bestDist = dist[v];
            best = v;
        }
    }
    return best;
}

/*
 * Compute shortest-path distances from `source` to every vertex.
 *
 * Fills `dist` (length graph->nodeCount) with the minimum cost from source to
 * each vertex, or INF where no path exists. Runs in O(V^2), which beats a
 * heap for the small graphs this module is built for.
 *
 * Returns 1 on success, or 0 if source is out of range.
 */
int dijkstra(const WeightedGraph *graph, int source, int *dist) {
    if (source < 0 || source >= graph->nodeCount)
        return 0;

    int visited[MAX_NODES];
    for (int v = 0; v < graph->nodeCount; v++) {
        dist[v] = INF;       /* nothing reached yet */
        visited[v] = 0;
    }
    dist[source] = 0;        /* the source is zero cost from itself */

    /* Each iteration finalizes one vertex, so V iterations settle them all. */
    for (int iter = 0; iter < graph->nodeCount; iter++) {
        int u = pickClosest(graph, dist, visited);
        if (u == -1)
            break;           /* the rest of the graph is unreachable */
        visited[u] = 1;      /* dist[u] is now final */

        /* Relax every edge leaving u: a path through u may be shorter. */
        for (int v = 0; v < graph->nodeCount; v++) {
            if (graph->weight[u][v] == INF || visited[v])
                continue;
            int candidate = dist[u] + graph->weight[u][v];
            if (candidate < dist[v])
                dist[v] = candidate;
        }
    }
    return 1;
}

/*
 * Return the shortest-path cost from `source` to `target`.
 * Returns -1 if either endpoint is invalid or no path exists; otherwise the
 * non-negative path cost. Runs one full Dijkstra pass, O(V^2).
 */
int shortestPath(const WeightedGraph *graph, int source, int target) {
    if (target < 0 || target >= graph->nodeCount)
        return -1;
    int dist[MAX_NODES];
    if (!dijkstra(graph, source, dist))
        return -1;
    if (dist[target] == INF)
        return -1;  /* target is not reachable from source */
    return dist[target];
}
