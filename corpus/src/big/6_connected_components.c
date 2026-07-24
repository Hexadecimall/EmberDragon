/*
 * connected_components.c -- Label connected components of a binary grid.
 *
 * Scans a grid of foreground/background cells and assigns every foreground cell
 * a component id such that two cells share an id exactly when they are joined
 * through 4-connected foreground neighbours. Implemented as a single pass with a
 * union-find (disjoint-set) structure over a sliding two-row window, then a
 * relabel pass to produce dense, 1-based ids.
 */

#include <stdlib.h>

/*
 * DisjointSet is a classic union-find over [0, count) with path compression and
 * union by rank. Each element starts in its own singleton set.
 */
typedef struct {
    int *parent; /* parent[i] points toward the set representative */
    int *rank;   /* upper bound on tree height, for balanced unions  */
    int  count;
} DisjointSet;

/*
 * Allocate a disjoint set of `count` singletons. Returns NULL on failure; the
 * caller frees it with djs_destroy. O(count).
 */
static DisjointSet *djs_create(int count) {
    DisjointSet *s = (DisjointSet *)malloc(sizeof(DisjointSet));
    if (!s)
        return NULL;
    s->parent = (int *)malloc(count * sizeof(int));
    s->rank = (int *)malloc(count * sizeof(int));
    if (!s->parent || !s->rank) {
        free(s->parent);
        free(s->rank);
        free(s);
        return NULL;
    }
    s->count = count;
    for (int i = 0; i < count; i++) {
        s->parent[i] = i; /* each element is initially its own root */
        s->rank[i] = 0;
    }
    return s;
}

/*
 * Free a disjoint set. Safe with NULL. O(1).
 */
static void djs_destroy(DisjointSet *s) {
    if (!s)
        return;
    free(s->parent);
    free(s->rank);
    free(s);
}

/*
 * Find the representative of x's set, compressing the path so future lookups
 * are faster. Amortised near-constant time (inverse Ackermann).
 */
static int djs_find(DisjointSet *s, int x) {
    while (s->parent[x] != x) {
        s->parent[x] = s->parent[s->parent[x]]; /* halve the path on the way up */
        x = s->parent[x];
    }
    return x;
}

/*
 * Merge the sets containing a and b, attaching the shorter tree under the
 * taller one to keep depth low. No-op if they already share a root. O(alpha).
 */
static void djs_union(DisjointSet *s, int a, int b) {
    int ra = djs_find(s, a);
    int rb = djs_find(s, b);
    if (ra == rb)
        return;
    if (s->rank[ra] < s->rank[rb]) {
        s->parent[ra] = rb;
    } else if (s->rank[ra] > s->rank[rb]) {
        s->parent[rb] = ra;
    } else {
        s->parent[rb] = ra;
        s->rank[ra]++; /* equal ranks: pick a root and bump its height bound */
    }
}

/*
 * Label the connected components of a binary grid.
 *
 * `grid` holds width*height cells where non-zero is foreground. Writes a 1-based
 * component id into labels[] for each foreground cell (0 stays 0 for
 * background) and returns the number of distinct components. labels must have
 * room for width*height ints. Returns 0 on bad input or allocation failure.
 * O(w*h * alpha).
 */
int label_components(const unsigned char *grid, int width, int height,
                     int *labels) {
    if (!grid || !labels || width <= 0 || height <= 0)
        return 0;

    int n = width * height;
    DisjointSet *sets = djs_create(n);
    if (!sets)
        return 0;

    /* Union pass: each foreground cell is joined to its already-seen west and
     * north neighbours, so equivalent labels collapse into one set. */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int here = y * width + x;
            if (!grid[here])
                continue; /* background cells form no components */
            if (x > 0 && grid[here - 1])
                djs_union(sets, here, here - 1);       /* west neighbour */
            if (y > 0 && grid[here - width])
                djs_union(sets, here, here - width);   /* north neighbour */
        }
    }

    /* Relabel pass: map each component root to a small dense id the first time
     * we encounter it, so callers see ids 1, 2, 3, ... with no gaps. */
    int *root_to_id = (int *)malloc(n * sizeof(int));
    if (!root_to_id) {
        djs_destroy(sets);
        return 0;
    }
    for (int i = 0; i < n; i++)
        root_to_id[i] = 0; /* 0 means "no id assigned yet" */

    int next_id = 1;
    for (int i = 0; i < n; i++) {
        if (!grid[i]) {
            labels[i] = 0;
            continue;
        }
        int root = djs_find(sets, i);
        if (root_to_id[root] == 0)
            root_to_id[root] = next_id++; /* first cell of a new component */
        labels[i] = root_to_id[root];
    }

    free(root_to_id);
    djs_destroy(sets);
    return next_id - 1; /* number of components actually emitted */
}
