/*
 * union_find.c
 *
 * A disjoint-set (union-find) data structure with both path compression and
 * union by rank. These two optimisations together make a sequence of
 * operations run in near-constant amortised time (inverse Ackermann).
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * The forest of sets. Each element points at a parent; a root points at
 * itself. `rank` is an upper bound on a tree's height used to keep unions
 * shallow. `setCount` tracks how many distinct sets currently exist.
 */
typedef struct DisjointSet {
    int *parent;    /* parent[i] is i's parent, or i itself if i is a root */
    int *rank;      /* rank[i] is a height bound for the tree rooted at i */
    int count;      /* total number of elements */
    int setCount;   /* number of distinct sets remaining */
} DisjointSet;

/*
 * Create a structure of `count` elements, each in its own singleton set.
 * Returns NULL on allocation failure; caller frees with dsuDestroy.
 */
DisjointSet *dsuCreate(int count) {
    DisjointSet *d = (DisjointSet *)malloc(sizeof(DisjointSet));
    if (d == NULL) {
        return NULL;
    }
    d->parent = (int *)malloc((size_t)count * sizeof(int));
    d->rank = (int *)calloc((size_t)count, sizeof(int));  /* all ranks start 0 */
    if (d->parent == NULL || d->rank == NULL) {
        free(d->parent);
        free(d->rank);
        free(d);
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        d->parent[i] = i;   /* every element is initially its own root */
    }
    d->count = count;
    d->setCount = count;    /* count singletons == count sets */
    return d;
}

/*
 * Find the representative (root) of the set containing x, applying path
 * compression so future lookups are faster. Amortised near-O(1).
 * Returns -1 if x is out of range.
 */
int dsuFind(DisjointSet *d, int x) {
    if (x < 0 || x >= d->count) {
        return -1;
    }
    /* Walk to the root. */
    int root = x;
    while (d->parent[root] != root) {
        root = d->parent[root];
    }
    /* Second pass: point every node on the path directly at the root. */
    while (d->parent[x] != root) {
        int next = d->parent[x];
        d->parent[x] = root;
        x = next;
    }
    return root;
}

/*
 * Merge the sets containing a and b. Uses union by rank: the shorter tree is
 * hung under the taller one to limit growth. Returns 1 if a merge happened, 0
 * if a and b were already in the same set, or -1 on invalid indices.
 */
int dsuUnion(DisjointSet *d, int a, int b) {
    int ra = dsuFind(d, a);
    int rb = dsuFind(d, b);
    if (ra < 0 || rb < 0) {
        return -1;
    }
    if (ra == rb) {
        return 0;   /* already connected; nothing to do */
    }
    /* Attach the lower-rank root beneath the higher-rank root. */
    if (d->rank[ra] < d->rank[rb]) {
        d->parent[ra] = rb;
    } else if (d->rank[ra] > d->rank[rb]) {
        d->parent[rb] = ra;
    } else {
        /* Equal ranks: pick one root and bump its rank by one. */
        d->parent[rb] = ra;
        d->rank[ra]++;
    }
    d->setCount--;   /* two sets became one */
    return 1;
}

/*
 * Return 1 if a and b belong to the same set, else 0 (or -1 on bad indices).
 */
int dsuConnected(DisjointSet *d, int a, int b) {
    int ra = dsuFind(d, a);
    int rb = dsuFind(d, b);
    if (ra < 0 || rb < 0) {
        return -1;
    }
    return ra == rb ? 1 : 0;
}

/*
 * Free the internal arrays and the structure itself. NULL is ignored.
 */
void dsuDestroy(DisjointSet *d) {
    if (d == NULL) {
        return;
    }
    free(d->parent);
    free(d->rank);
    free(d);
}
