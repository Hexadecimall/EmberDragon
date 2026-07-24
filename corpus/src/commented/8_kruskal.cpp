/*
 * Kruskal's minimum-spanning-tree algorithm for a small undirected weighted
 * graph. Edges are sorted by ascending weight and added greedily, skipping
 * any edge whose endpoints already share a tree. Cycle detection is handled
 * by an embedded union-find with path compression and union by rank.
 */

#include <cstdlib>

const int kMaxVertices = 64;
const int kMaxEdges = 256;

// One undirected weighted edge connecting two vertices.
struct Edge {
    int from;     // one endpoint vertex id
    int to;       // the other endpoint vertex id
    int weight;   // edge cost; may be any integer
};

// A weighted graph: a vertex count plus a flat array of edges.
struct EdgeListGraph {
    int vertexCount;          // active vertices, ids 0..vertexCount-1
    int edgeCount;            // number of edges currently stored
    Edge edges[kMaxEdges];    // the edge set, unordered until sorted
};

// Disjoint-set forest used to detect cycles while building the tree.
struct UnionFind {
    int parent[kMaxVertices]; // parent[i] == i marks a root
    int rank[kMaxVertices];   // height bound, valid only at roots
};

/*
 * Initialize a graph with `vertexCount` vertices and no edges.
 * Clamps the count into [0, kMaxVertices]. Returns nothing.
 */
void graphInit(EdgeListGraph *graph, int vertexCount) {
    if (vertexCount < 0)
        vertexCount = 0;
    if (vertexCount > kMaxVertices)
        vertexCount = kMaxVertices;
    graph->vertexCount = vertexCount;
    graph->edgeCount = 0;
}

/*
 * Append an undirected edge {from, to} with the given weight.
 * Ignores invalid endpoints and silently drops the edge if the table is full.
 * Returns true if the edge was stored, false otherwise.
 */
bool addWeightedEdge(EdgeListGraph *graph, int from, int to, int weight) {
    if (from < 0 || from >= graph->vertexCount)
        return false;
    if (to < 0 || to >= graph->vertexCount)
        return false;
    if (graph->edgeCount >= kMaxEdges)
        return false;  // no room left in the fixed edge array
    Edge &e = graph->edges[graph->edgeCount++];
    e.from = from;
    e.to = to;
    e.weight = weight;
    return true;
}

/*
 * Reset a union-find so each of `count` vertices is its own singleton set.
 */
static void unionFindInit(UnionFind *uf, int count) {
    for (int i = 0; i < count; i++) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }
}

/*
 * Find the root of `x`'s set, compressing the path on the way back.
 * Returns the representative vertex id. Amortized near-constant time.
 */
static int unionFindFind(UnionFind *uf, int x) {
    int root = x;
    while (uf->parent[root] != root)
        root = uf->parent[root];
    // Point every node on the path directly at the root.
    while (uf->parent[x] != root) {
        int next = uf->parent[x];
        uf->parent[x] = root;
        x = next;
    }
    return root;
}

/*
 * Merge the sets of `a` and `b` using union by rank.
 * Returns true if they were separate (a real merge happened), false if they
 * were already in the same set.
 */
static bool unionFindUnite(UnionFind *uf, int a, int b) {
    int ra = unionFindFind(uf, a);
    int rb = unionFindFind(uf, b);
    if (ra == rb)
        return false;  // adding this edge would create a cycle
    if (uf->rank[ra] < uf->rank[rb]) {
        uf->parent[ra] = rb;
    } else if (uf->rank[ra] > uf->rank[rb]) {
        uf->parent[rb] = ra;
    } else {
        uf->parent[rb] = ra;
        uf->rank[ra]++;
    }
    return true;
}

/*
 * Order edges by ascending weight with an in-place insertion sort.
 * Stable and simple; O(E^2) worst case, which is fine for the small edge
 * counts this module handles. Sorts `count` entries of `edges` in place.
 */
static void sortEdgesByWeight(Edge *edges, int count) {
    for (int i = 1; i < count; i++) {
        Edge key = edges[i];
        int j = i - 1;
        // Shift larger-weight edges one slot right to open a hole for key.
        while (j >= 0 && edges[j].weight > key.weight) {
            edges[j + 1] = edges[j];
            j--;
        }
        edges[j + 1] = key;
    }
}

/*
 * Build a minimum spanning tree (or forest) of `graph` with Kruskal's
 * algorithm.
 *
 * Chosen tree edges are written to `mstOut`, which must hold at least
 * vertexCount-1 edges. The total weight of the selected edges is stored
 * through `totalWeight` when that pointer is non-null.
 *
 * Returns the number of edges placed in the tree. For a connected graph this
 * is vertexCount-1; a smaller value means the graph is disconnected and the
 * result is a spanning forest. Dominated by the O(E^2) sort.
 */
int kruskalMST(EdgeListGraph *graph, Edge *mstOut, int *totalWeight) {
    sortEdgesByWeight(graph->edges, graph->edgeCount);

    UnionFind uf;
    unionFindInit(&uf, graph->vertexCount);

    int chosen = 0;
    int weightSum = 0;
    for (int i = 0; i < graph->edgeCount; i++) {
        const Edge &e = graph->edges[i];
        // Take the edge only if it joins two so-far-separate components.
        if (unionFindUnite(&uf, e.from, e.to)) {
            mstOut[chosen++] = e;
            weightSum += e.weight;
            // A spanning tree needs exactly vertexCount-1 edges; stop early.
            if (chosen == graph->vertexCount - 1)
                break;
        }
    }

    if (totalWeight != nullptr)
        *totalWeight = weightSum;
    return chosen;
}
