/*
 * topological_sort.c
 *
 * Kahn's algorithm for topologically ordering the vertices of a directed
 * acyclic graph (DAG). Repeatedly emits a vertex with no remaining incoming
 * edges; if any vertex is left over, the graph contained a cycle.
 */

#include <stdio.h>
#include <stdlib.h>

/* Adjacency-list edge node pointing at a successor vertex. */
typedef struct AdjNode {
    int target;            /* vertex this edge leads to */
    struct AdjNode *next;  /* next successor in the list, or NULL */
} AdjNode;

/* A directed graph: per-vertex successor lists indexed by vertex id. */
typedef struct DAG {
    int size;          /* number of vertices, ids 0..size-1 */
    AdjNode **adj;     /* array of successor-list heads */
} DAG;

/*
 * Create an empty directed graph with `size` vertices.
 * Returns NULL on allocation failure. Caller frees with dagDestroy.
 */
DAG *dagCreate(int size) {
    DAG *g = (DAG *)malloc(sizeof(DAG));
    if (g == NULL) {
        return NULL;
    }
    g->size = size;
    g->adj = (AdjNode **)calloc((size_t)size, sizeof(AdjNode *));
    if (g->adj == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

/*
 * Add a directed dependency edge from -> to (meaning `from` precedes `to`).
 * Prepends to the successor list in O(1). Returns 0, or -1 on bad indices or
 * allocation failure.
 */
int dagAddEdge(DAG *g, int from, int to) {
    if (from < 0 || from >= g->size || to < 0 || to >= g->size) {
        return -1;
    }
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
    if (node == NULL) {
        return -1;
    }
    node->target = to;
    node->next = g->adj[from];
    g->adj[from] = node;
    return 0;
}

/*
 * Produce a topological ordering of the graph into the caller-supplied
 * `order` array (which must have room for g->size ints).
 *
 * Returns the number of vertices placed. If that equals g->size the ordering
 * is complete; a smaller count means the graph has a cycle and the remaining
 * vertices could not be ordered. Runs in O(V + E).
 */
int dagTopoSort(const DAG *g, int *order) {
    int n = g->size;
    int *indegree = (int *)calloc((size_t)n, sizeof(int));
    int *queue = (int *)malloc((size_t)n * sizeof(int));
    if (indegree == NULL || queue == NULL) {
        free(indegree);
        free(queue);
        return -1;  /* signal allocation failure distinctly from a cycle */
    }

    /* First pass: tally how many incoming edges each vertex has. */
    for (int v = 0; v < n; v++) {
        for (AdjNode *e = g->adj[v]; e != NULL; e = e->next) {
            indegree[e->target]++;
        }
    }

    /* Seed the queue with every source (indegree 0). */
    int head = 0, tail = 0;
    for (int v = 0; v < n; v++) {
        if (indegree[v] == 0) {
            queue[tail++] = v;
        }
    }

    int placed = 0;
    while (head < tail) {
        int v = queue[head++];   /* dequeue a vertex with no pending deps */
        order[placed++] = v;
        /* Removing v lowers each successor's indegree; newly freed ones queue. */
        for (AdjNode *e = g->adj[v]; e != NULL; e = e->next) {
            if (--indegree[e->target] == 0) {
                queue[tail++] = e->target;
            }
        }
    }

    free(indegree);
    free(queue);
    return placed;  /* < n iff a cycle blocked some vertices */
}

/*
 * Free all edge nodes, the adjacency array, and the graph. NULL is ignored.
 */
void dagDestroy(DAG *g) {
    if (g == NULL) {
        return;
    }
    for (int v = 0; v < g->size; v++) {
        AdjNode *e = g->adj[v];
        while (e != NULL) {
            AdjNode *doomed = e;
            e = e->next;
            free(doomed);
        }
    }
    free(g->adj);
    free(g);
}
