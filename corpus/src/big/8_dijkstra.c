/*
 * dijkstra.c
 *
 * Single-source shortest paths on a directed graph with non-negative integer
 * edge weights. This implementation uses a binary min-heap keyed on tentative
 * distance, giving O((V + E) log V) time.
 */

#include <stdio.h>
#include <stdlib.h>

/* A "very large" distance standing in for infinity (unreached vertices). */
#define DIST_INFINITY 0x3fffffff

/* Outgoing weighted edge in an adjacency list. */
typedef struct WEdge {
    int to;              /* destination vertex */
    int weight;          /* non-negative edge cost */
    struct WEdge *next;  /* next edge from the same source */
} WEdge;

/* Directed weighted graph as an array of adjacency lists. */
typedef struct WGraph {
    int n;          /* number of vertices */
    WEdge **adj;    /* adjacency-list heads */
} WGraph;

/* One heap entry: a vertex and the distance it was queued with. */
typedef struct HeapItem {
    int vertex;
    int dist;
} HeapItem;

/* A binary min-heap ordered by HeapItem.dist. */
typedef struct MinHeap {
    HeapItem *items;
    int size;       /* number of items currently stored */
    int capacity;   /* allocated slots */
} MinHeap;

/*
 * Create an empty weighted graph with n vertices, or NULL on failure.
 * Caller frees with wgraphDestroy.
 */
WGraph *wgraphCreate(int n) {
    WGraph *g = (WGraph *)malloc(sizeof(WGraph));
    if (g == NULL) {
        return NULL;
    }
    g->n = n;
    g->adj = (WEdge **)calloc((size_t)n, sizeof(WEdge *));
    if (g->adj == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

/*
 * Add a directed edge from -> to with the given non-negative weight.
 * Returns 0, or -1 on bad indices or allocation failure.
 */
int wgraphAddEdge(WGraph *g, int from, int to, int weight) {
    if (from < 0 || from >= g->n || to < 0 || to >= g->n || weight < 0) {
        return -1;
    }
    WEdge *e = (WEdge *)malloc(sizeof(WEdge));
    if (e == NULL) {
        return -1;
    }
    e->to = to;
    e->weight = weight;
    e->next = g->adj[from];
    g->adj[from] = e;
    return 0;
}

/* Restore the heap property upward from index i after an insertion. */
static void heapSiftUp(MinHeap *h, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->items[parent].dist <= h->items[i].dist) {
            break;  /* parent is already no larger; heap order holds */
        }
        HeapItem tmp = h->items[parent];
        h->items[parent] = h->items[i];
        h->items[i] = tmp;
        i = parent;
    }
}

/* Restore the heap property downward from index i after a removal. */
static void heapSiftDown(MinHeap *h, int i) {
    for (;;) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < h->size && h->items[left].dist < h->items[smallest].dist) {
            smallest = left;
        }
        if (right < h->size && h->items[right].dist < h->items[smallest].dist) {
            smallest = right;
        }
        if (smallest == i) {
            break;  /* both children are >= current; done */
        }
        HeapItem tmp = h->items[i];
        h->items[i] = h->items[smallest];
        h->items[smallest] = tmp;
        i = smallest;
    }
}

/* Push (vertex, dist) onto the heap. Assumes capacity is sufficient. */
static void heapPush(MinHeap *h, int vertex, int dist) {
    h->items[h->size].vertex = vertex;
    h->items[h->size].dist = dist;
    h->size++;
    heapSiftUp(h, h->size - 1);
}

/* Pop and return the minimum-distance item; caller checks h->size first. */
static HeapItem heapPop(MinHeap *h) {
    HeapItem top = h->items[0];
    h->size--;
    h->items[0] = h->items[h->size];  /* move last item to the root */
    heapSiftDown(h, 0);
    return top;
}

/*
 * Run Dijkstra from `source`, filling the caller-supplied `dist` array (length
 * g->n) with the shortest distance to each vertex; unreachable vertices get
 * DIST_INFINITY. Returns 0 on success or -1 on bad source / allocation failure.
 *
 * The heap may hold stale entries for a vertex; a popped entry whose recorded
 * distance exceeds the best known distance is simply skipped.
 */
int dijkstra(const WGraph *g, int source, int *dist) {
    if (source < 0 || source >= g->n) {
        return -1;
    }
    for (int v = 0; v < g->n; v++) {
        dist[v] = DIST_INFINITY;
    }

    MinHeap heap;
    /* Worst case one heap slot per edge relaxation, bounded loosely below. */
    heap.capacity = g->n * 4 + 16;
    heap.size = 0;
    heap.items = (HeapItem *)malloc((size_t)heap.capacity * sizeof(HeapItem));
    if (heap.items == NULL) {
        return -1;
    }

    dist[source] = 0;
    heapPush(&heap, source, 0);

    while (heap.size > 0) {
        HeapItem cur = heapPop(&heap);
        if (cur.dist > dist[cur.vertex]) {
            continue;  /* outdated entry superseded by a shorter path */
        }
        /* Relax every outgoing edge of the settled vertex. */
        for (WEdge *e = g->adj[cur.vertex]; e != NULL; e = e->next) {
            int candidate = cur.dist + e->weight;
            if (candidate < dist[e->to]) {
                dist[e->to] = candidate;
                /* Grow the heap array if a relaxation would overflow it. */
                if (heap.size >= heap.capacity) {
                    heap.capacity *= 2;
                    HeapItem *bigger = (HeapItem *)realloc(
                        heap.items, (size_t)heap.capacity * sizeof(HeapItem));
                    if (bigger == NULL) {
                        free(heap.items);
                        return -1;
                    }
                    heap.items = bigger;
                }
                heapPush(&heap, e->to, candidate);
            }
        }
    }

    free(heap.items);
    return 0;
}

/*
 * Free every edge list, the adjacency array, and the graph. NULL is ignored.
 */
void wgraphDestroy(WGraph *g) {
    if (g == NULL) {
        return;
    }
    for (int v = 0; v < g->n; v++) {
        WEdge *e = g->adj[v];
        while (e != NULL) {
            WEdge *doomed = e;
            e = e->next;
            free(doomed);
        }
    }
    free(g->adj);
    free(g);
}
