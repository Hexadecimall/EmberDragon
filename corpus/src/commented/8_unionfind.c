/*
 * Disjoint-set (union-find) data structure with path compression and
 * union by rank. It groups elements 0..n-1 into disjoint sets and answers
 * "are these two in the same set?" in near-constant amortized time.
 */

#include <stdio.h>
#include <stdlib.h>

/* Each element points at a parent; a root points at itself. `rank` is an
 * upper bound on the tree height used to keep merges shallow. `setCount`
 * tracks how many distinct sets currently exist. */
typedef struct DisjointSet {
    int *parent;    /* parent[i] is i's parent, or i itself if i is a root  */
    int *rank;      /* rank[i] is meaningful only when i is a root          */
    int size;       /* number of elements, i.e. valid ids are 0..size-1     */
    int setCount;   /* number of disjoint sets remaining after unions       */
} DisjointSet;

/*
 * Create a union-find over `size` singleton sets {0}, {1}, ..., {size-1}.
 * Returns a heap-allocated structure the caller frees with disjointSetFree,
 * or NULL if size is non-positive or any allocation fails.
 */
DisjointSet *disjointSetCreate(int size) {
    if (size <= 0)
        return NULL;
    DisjointSet *set = (DisjointSet *)malloc(sizeof(DisjointSet));
    if (set == NULL)
        return NULL;
    set->parent = (int *)malloc((size_t)size * sizeof(int));
    set->rank = (int *)malloc((size_t)size * sizeof(int));
    if (set->parent == NULL || set->rank == NULL) {
        free(set->parent);   /* free(NULL) is harmless if only one failed */
        free(set->rank);
        free(set);
        return NULL;
    }
    for (int i = 0; i < size; i++) {
        set->parent[i] = i;  /* every element starts as its own root */
        set->rank[i] = 0;
    }
    set->size = size;
    set->setCount = size;
    return set;
}

/*
 * Find the representative (root) of the set containing `x`, compressing the
 * path so future lookups are faster.
 * Returns the root id, or -1 if x is out of range.
 * Amortized near-O(1) with the inverse-Ackermann bound.
 */
int find(DisjointSet *set, int x) {
    if (set == NULL || x < 0 || x >= set->size)
        return -1;
    /* Walk up to the root. */
    int root = x;
    while (set->parent[root] != root)
        root = set->parent[root];
    /* Path compression: re-point every node on the path straight at root. */
    while (set->parent[x] != root) {
        int next = set->parent[x];
        set->parent[x] = root;
        x = next;
    }
    return root;
}

/*
 * Merge the sets containing `a` and `b` using union by rank.
 * Returns 1 if a merge happened, 0 if they were already together, and -1 if
 * either id is invalid. Decrements setCount on a real merge.
 */
int unite(DisjointSet *set, int a, int b) {
    int rootA = find(set, a);
    int rootB = find(set, b);
    if (rootA == -1 || rootB == -1)
        return -1;
    if (rootA == rootB)
        return 0;  /* already in the same set; nothing to do */

    /* Attach the shorter tree under the taller one to limit height. */
    if (set->rank[rootA] < set->rank[rootB]) {
        set->parent[rootA] = rootB;
    } else if (set->rank[rootA] > set->rank[rootB]) {
        set->parent[rootB] = rootA;
    } else {
        /* Equal ranks: pick one root and bump its rank by one. */
        set->parent[rootB] = rootA;
        set->rank[rootA]++;
    }
    set->setCount--;
    return 1;
}

/*
 * Report whether `a` and `b` belong to the same set.
 * Returns 1 if connected, 0 if not or if either id is invalid.
 */
int connected(DisjointSet *set, int a, int b) {
    int rootA = find(set, a);
    int rootB = find(set, b);
    if (rootA == -1 || rootB == -1)
        return 0;
    return rootA == rootB;
}

/*
 * Return the number of disjoint sets currently present, or 0 if set is NULL.
 */
int countSets(const DisjointSet *set) {
    if (set == NULL)
        return 0;
    return set->setCount;
}

/*
 * Release all memory owned by the structure. Safe to call on NULL.
 */
void disjointSetFree(DisjointSet *set) {
    if (set == NULL)
        return;
    free(set->parent);
    free(set->rank);
    free(set);
}
