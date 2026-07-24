/*
 * bfs.c
 *
 * Breadth-first search over an undirected graph stored as adjacency lists.
 * BFS explores the graph in concentric layers from a start vertex, which makes
 * it ideal for shortest-path-by-edge-count and reachability queries.
 */

#include <stdio.h>
#include <stdlib.h>

/* Adjacency-list node naming a neighbouring vertex. */
typedef struct Neighbor {
    int id;                  /* the neighbouring vertex */
    struct Neighbor *next;   /* next neighbour, or NULL */
} Neighbor;

/* Undirected graph: each vertex keeps a list of its neighbours. */
typedef struct Graph {
    int n;             /* vertex count */
    Neighbor **lists;  /* per-vertex neighbour-list heads */
} Graph;

/*
 * Allocate an empty graph with n vertices. Returns NULL on failure.
 * Caller frees with graphFree.
 */
Graph *graphNew(int n) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->lists = (Neighbor **)calloc((size_t)n, sizeof(Neighbor *));
    if (g->lists == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

/* Prepend `b` to `a`'s neighbour list. Helper used by graphConnect. */
static int linkOneWay(Graph *g, int a, int b) {
    Neighbor *node = (Neighbor *)malloc(sizeof(Neighbor));
    if (node == NULL) {
        return -1;
    }
    node->id = b;
    node->next = g->lists[a];
    g->lists[a] = node;
    return 0;
}

/*
 * Add an undirected edge between a and b by linking each into the other's
 * neighbour list. Returns 0, or -1 on bad indices or allocation failure.
 */
int graphConnect(Graph *g, int a, int b) {
    if (a < 0 || a >= g->n || b < 0 || b >= g->n) {
        return -1;
    }
    if (linkOneWay(g, a, b) != 0) {
        return -1;
    }
    return linkOneWay(g, b, a);
}

/*
 * Breadth-first search from `start`, recording in `distance[v]` the minimum
 * number of edges from start to v, or -1 for vertices not reachable from start.
 * The `distance` array must hold g->n ints. Returns the number of vertices
 * visited, or -1 on bad start / allocation failure. Runs in O(V + E).
 */
int bfs(const Graph *g, int start, int *distance) {
    if (start < 0 || start >= g->n) {
        return -1;
    }
    int *queue = (int *)malloc((size_t)g->n * sizeof(int));
    if (queue == NULL) {
        return -1;
    }
    /* Mark all vertices unreached; -1 doubles as the "not visited" flag. */
    for (int v = 0; v < g->n; v++) {
        distance[v] = -1;
    }

    int head = 0, tail = 0;
    distance[start] = 0;
    queue[tail++] = start;   /* enqueue the source at distance 0 */

    int visited = 0;
    while (head < tail) {
        int v = queue[head++];   /* dequeue the next frontier vertex */
        visited++;
        for (Neighbor *nb = g->lists[v]; nb != NULL; nb = nb->next) {
            /* Discover each unvisited neighbour one layer deeper. */
            if (distance[nb->id] == -1) {
                distance[nb->id] = distance[v] + 1;
                queue[tail++] = nb->id;
            }
        }
    }

    free(queue);
    return visited;
}

/*
 * Return 1 if `target` is reachable from `start`, else 0 (or -1 on error).
 * Reuses bfs and then inspects the computed distance.
 */
int graphReachable(const Graph *g, int start, int target) {
    if (target < 0 || target >= g->n) {
        return -1;
    }
    int *distance = (int *)malloc((size_t)g->n * sizeof(int));
    if (distance == NULL) {
        return -1;
    }
    if (bfs(g, start, distance) < 0) {
        free(distance);
        return -1;
    }
    int reachable = distance[target] >= 0 ? 1 : 0;
    free(distance);
    return reachable;
}

/*
 * Free all neighbour nodes, the list array, and the graph. NULL is ignored.
 */
void graphFree(Graph *g) {
    if (g == NULL) {
        return;
    }
    for (int v = 0; v < g->n; v++) {
        Neighbor *nb = g->lists[v];
        while (nb != NULL) {
            Neighbor *doomed = nb;
            nb = nb->next;
            free(doomed);
        }
    }
    free(g->lists);
    free(g);
}
