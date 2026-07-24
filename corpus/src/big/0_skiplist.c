/*
 * Probabilistic skip list implementing an ordered integer set.
 *
 * A skip list layers several forward-linked "express lanes" over a sorted
 * base chain. Higher levels skip over more nodes, giving expected O(log n)
 * search, insert, and delete without the rotation machinery of a balanced
 * tree. Node tower heights are chosen by a coin-flip process, here driven by
 * a small self-contained xorshift PRNG so the module needs no <math.h> or
 * external randomness.
 */

#include <stdlib.h>
#include <stdint.h>

#define SKIPLIST_MAX_LEVEL 16  /* tower height cap; supports ~2^16 elements */

/*
 * A skip-list node. `forward` is a flexible-length array of `level+1`
 * forward pointers, one per lane this node participates in. The shared head
 * node uses the same layout with a sentinel key.
 */
typedef struct SkipNode {
    int32_t key;
    int32_t level;             /* highest lane index this node reaches */
    struct SkipNode **forward; /* forward[i] = next node on lane i */
} SkipNode;

/* List handle: a head sentinel, the current top occupied level, and an
 * xorshift32 state used to randomize tower heights. */
typedef struct {
    SkipNode *head;     /* sentinel before all keys; spans all levels */
    int32_t   level;    /* index of the highest lane in use, 0-based */
    uint32_t  rng_state;/* xorshift32 PRNG state, must be non-zero */
} SkipList;

/* Allocate a node with `level + 1` forward slots, all initialized to NULL. */
static SkipNode *skip_make_node(int32_t key, int32_t level) {
    SkipNode *node = (SkipNode *)malloc(sizeof(SkipNode));
    if (node == NULL) {
        return NULL;
    }
    node->forward = (SkipNode **)calloc((size_t)level + 1, sizeof(SkipNode *));
    if (node->forward == NULL) {
        free(node);
        return NULL;
    }
    node->key = key;
    node->level = level;
    return node;
}

/*
 * Initialize an empty skip list.
 * @param seed  PRNG seed; coerced to 1 if zero so xorshift never sticks at 0.
 * @return 0 on success, -1 on allocation failure.
 */
int skip_init(SkipList *list, uint32_t seed) {
    /* The head sentinel spans every possible level so searches can start at
     * the top lane regardless of how tall the list later grows. */
    list->head = skip_make_node(0, SKIPLIST_MAX_LEVEL - 1);
    if (list->head == NULL) {
        return -1;
    }
    list->level = 0;
    list->rng_state = (seed == 0) ? 1u : seed;
    return 0;
}

/* Advance the xorshift32 generator and return the next pseudo-random word. */
static uint32_t skip_rand(SkipList *list) {
    uint32_t x = list->rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    list->rng_state = x;
    return x;
}

/*
 * Draw a random tower height in [0, SKIPLIST_MAX_LEVEL-1].
 * Each additional level is granted with probability 1/2 (we promote while a
 * random bit is set), giving the geometric distribution skip lists rely on.
 */
static int32_t skip_random_level(SkipList *list) {
    int32_t level = 0;
    /* Inspect successive low bits of the PRNG output; stop at the first 0. */
    while ((skip_rand(list) & 1u) && level < SKIPLIST_MAX_LEVEL - 1) {
        level++;
    }
    return level;
}

/*
 * Test membership of `key`.
 * @return 1 if present, 0 otherwise. Expected O(log n).
 */
int skip_contains(const SkipList *list, int32_t key) {
    const SkipNode *cur = list->head;
    /* Descend from the top lane, moving right while the next key is smaller. */
    for (int32_t i = list->level; i >= 0; i--) {
        while (cur->forward[i] != NULL && cur->forward[i]->key < key) {
            cur = cur->forward[i];
        }
    }
    /* We are now just left of where key would be on the base lane. */
    cur = cur->forward[0];
    return (cur != NULL && cur->key == key) ? 1 : 0;
}

/*
 * Insert `key` into the set.
 * @return 0 on success (or if the key already exists), -1 on OOM.
 * Expected O(log n). Duplicate keys are ignored to preserve set semantics.
 */
int skip_insert(SkipList *list, int32_t key) {
    SkipNode *update[SKIPLIST_MAX_LEVEL];  /* per-lane predecessor of key */
    SkipNode *cur = list->head;

    /* Record, for each level, the last node we pass before dropping down. */
    for (int32_t i = list->level; i >= 0; i--) {
        while (cur->forward[i] != NULL && cur->forward[i]->key < key) {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }

    /* If the base-lane successor already holds key, there is nothing to do. */
    cur = cur->forward[0];
    if (cur != NULL && cur->key == key) {
        return 0;
    }

    int32_t new_level = skip_random_level(list);
    if (new_level > list->level) {
        /* The new tower is taller than anything so far: the head sentinel is
         * the predecessor on every freshly activated lane. */
        for (int32_t i = list->level + 1; i <= new_level; i++) {
            update[i] = list->head;
        }
        list->level = new_level;
    }

    SkipNode *node = skip_make_node(key, new_level);
    if (node == NULL) {
        return -1;
    }
    /* Splice the new node into each lane it participates in. */
    for (int32_t i = 0; i <= new_level; i++) {
        node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = node;
    }
    return 0;
}

/*
 * Remove `key` if present.
 * @return 1 if a node was removed, 0 if the key was absent.
 */
int skip_remove(SkipList *list, int32_t key) {
    SkipNode *update[SKIPLIST_MAX_LEVEL];
    SkipNode *cur = list->head;

    for (int32_t i = list->level; i >= 0; i--) {
        while (cur->forward[i] != NULL && cur->forward[i]->key < key) {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }

    cur = cur->forward[0];
    if (cur == NULL || cur->key != key) {
        return 0;  /* not in the list */
    }

    /* Unlink the node from every lane it appears on. */
    for (int32_t i = 0; i <= list->level; i++) {
        if (update[i]->forward[i] != cur) {
            break;  /* higher lanes no longer reference this node */
        }
        update[i]->forward[i] = cur->forward[i];
    }
    free(cur->forward);
    free(cur);

    /* Drop now-empty top lanes so future searches don't scan dead levels. */
    while (list->level > 0 && list->head->forward[list->level] == NULL) {
        list->level--;
    }
    return 1;
}

/* Free every node (including the head sentinel) and reset the handle. */
void skip_destroy(SkipList *list) {
    SkipNode *cur = list->head;
    while (cur != NULL) {
        SkipNode *next = cur->forward[0];  /* base lane threads all nodes */
        free(cur->forward);
        free(cur);
        cur = next;
    }
    list->head = NULL;
    list->level = 0;
}
