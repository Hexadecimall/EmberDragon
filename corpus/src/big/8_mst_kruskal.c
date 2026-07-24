/*
 * mst_kruskal.c
 *
 * Kruskal's algorithm for the minimum spanning tree (MST) of a connected
 * undirected weighted graph. Edges are sorted by weight and added greedily,
 * skipping any edge that would form a cycle, detected via union-find.
 */

#include <stdio.h>
#include <stdlib.h>

/* An undirected weighted edge between endpoints u and v. */
typedef struct Edge {
    int u;
    int v;
    int weight;
} Edge;

/* Disjoint-set forest used purely for cycle detection while building the MST. */
typedef struct Forest {
    int *parent;   /* parent[i] is i's parent, or i if i is a root */
    int *rank;     /* tree-height bound for union by rank */
} Forest;

/* Initialise a forest of n singletons. Returns 0 or -1 on allocation failure. */
static int forestInit(Forest *f, int n) {
    f->parent = (int *)malloc((size_t)n * sizeof(int));
    f->rank = (int *)calloc((size_t)n, sizeof(int));
    if (f->parent == NULL || f->rank == NULL) {
        free(f->parent);
        free(f->rank);
        return -1;
    }
    for (int i = 0; i < n; i++) {
        f->parent[i] = i;
    }
    return 0;
}

/* Find x's set representative with path compression. */
static int forestFind(Forest *f, int x) {
    int root = x;
    while (f->parent[root] != root) {
        root = f->parent[root];
    }
    /* Compress the path so subsequent finds are O(1) amortised. */
    while (f->parent[x] != root) {
        int next = f->parent[x];
        f->parent[x] = root;
        x = next;
    }
    return root;
}

/*
 * Union the sets of a and b by rank. Returns 1 if they were merged, or 0 if
 * they already shared a set (which means adding this edge would make a cycle).
 */
static int forestUnion(Forest *f, int a, int b) {
    int ra = forestFind(f, a);
    int rb = forestFind(f, b);
    if (ra == rb) {
        return 0;
    }
    if (f->rank[ra] < f->rank[rb]) {
        f->parent[ra] = rb;
    } else if (f->rank[ra] > f->rank[rb]) {
        f->parent[rb] = ra;
    } else {
        f->parent[rb] = ra;
        f->rank[ra]++;
    }
    return 1;
}

/* Release a forest's arrays. */
static void forestFree(Forest *f) {
    free(f->parent);
    free(f->rank);
}

/* Comparison callback ordering edges by ascending weight for qsort. */
static int compareEdges(const void *a, const void *b) {
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    /* Return the weight difference; safe because weights fit comfortably in int. */
    return ea->weight - eb->weight;
}

/*
 * Compute the minimum spanning tree of a graph with `vertexCount` vertices and
 * the `edgeCount` edges in `edges`. Selected MST edges are written, in the
 * order chosen, into `mstOut` (which must hold at least vertexCount-1 edges).
 *
 * Returns the total weight of the MST. On success the number of selected edges
 * is vertexCount-1; if the graph is disconnected fewer edges are written and
 * the returned weight covers a minimum spanning forest instead. Returns -1 on
 * allocation failure. The input `edges` array is reordered (sorted) in place.
 * Runs in O(E log E).
 */
long kruskalMST(int vertexCount, Edge *edges, int edgeCount, Edge *mstOut) {
    Forest forest;
    if (forestInit(&forest, vertexCount) != 0) {
        return -1;
    }

    /* Greedy choice requires examining edges from cheapest to most expensive. */
    qsort(edges, (size_t)edgeCount, sizeof(Edge), compareEdges);

    long totalWeight = 0;
    int selected = 0;
    for (int i = 0; i < edgeCount; i++) {
        /* Take an edge only if its endpoints are in different components. */
        if (forestUnion(&forest, edges[i].u, edges[i].v)) {
            mstOut[selected++] = edges[i];
            totalWeight += edges[i].weight;
            /* A spanning tree needs exactly vertexCount-1 edges; stop early. */
            if (selected == vertexCount - 1) {
                break;
            }
        }
    }

    forestFree(&forest);
    return totalWeight;
}
