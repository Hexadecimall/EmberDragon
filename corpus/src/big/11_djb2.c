/*
 * djb2.c — Daniel J. Bernstein's classic string hashes (djb2 and sdbm).
 *
 * djb2 is one of the oldest and simplest string-hashing functions still in wide
 * use: it starts from the magic seed 5381 and folds each byte in via
 * hash * 33 + byte. This module provides both the additive (djb2) and the
 * xor-mixed (djb2a) variants plus the related sdbm hash, and shows how to use
 * them to build a tiny separate-chaining symbol set.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* The famous djb2 seed. Bernstein never fully explained why 5381 works well;
 * empirically it disperses ASCII text better than a zero start. */
#define DJB2_SEED 5381u

/*
 * Classic additive djb2 hash of a NUL-terminated string.
 *
 * The recurrence hash = hash * 33 + c is computed as (hash << 5) + hash + c,
 * which is how Bernstein originally wrote it to avoid a multiply.
 *
 * str: NUL-terminated input. Returns the 32-bit hash. O(strlen(str)).
 */
uint32_t djb2(const char *str) {
    uint32_t hash = DJB2_SEED;
    int c;
    while ((c = (uint8_t)*str++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c; /* hash * 33 + c */
    }
    return hash;
}

/*
 * The xor variant (often called djb2a) of the same hash.
 *
 * Folding the byte in with xor instead of addition tends to give slightly
 * better avalanche on highly structured keys. Same parameters and complexity
 * as djb2.
 */
uint32_t djb2a(const char *str) {
    uint32_t hash = DJB2_SEED;
    int c;
    while ((c = (uint8_t)*str++)) {
        hash = (((hash << 5) + hash) ^ (uint32_t)c); /* (hash * 33) xor c */
    }
    return hash;
}

/*
 * sdbm hash — a sibling of djb2 from the sdbm database library.
 *
 * The recurrence hash = hash * 65599 + c, expanded into shifts, produces a
 * good distribution and is the default hash in several historical key-value
 * stores. O(strlen(str)), no allocation.
 */
uint32_t sdbm(const char *str) {
    uint32_t hash = 0;
    int c;
    while ((c = (uint8_t)*str++)) {
        hash = (uint32_t)c + (hash << 6) + (hash << 16) - hash; /* c + hash*65599 */
    }
    return hash;
}

/* A node in a separate-chaining set: just the owned key string and a next
 * pointer. We store only membership, not an associated value. */
typedef struct StringNode {
    char *key;
    struct StringNode *next;
} StringNode;

/* A fixed-bucket hash set keyed by string, resolving collisions by chaining. */
typedef struct StringSet {
    StringNode **buckets;
    size_t bucket_count;
} StringSet;

/*
 * Allocate an empty StringSet with the given number of buckets.
 *
 * bucket_count: number of chains; should be > 0 and ideally prime-ish.
 * Returns a heap-allocated set whose buckets are all NULL, or NULL on OOM.
 * Caller must release it with string_set_free.
 */
StringSet *string_set_create(size_t bucket_count) {
    StringSet *set = (StringSet *)malloc(sizeof(StringSet));
    if (!set) return NULL;
    set->buckets = (StringNode **)calloc(bucket_count, sizeof(StringNode *));
    if (!set->buckets) {        /* roll back the partial allocation */
        free(set);
        return NULL;
    }
    set->bucket_count = bucket_count;
    return set;
}

/*
 * Insert a key into the set if it is not already present.
 *
 * set: target set. key: NUL-terminated string, copied internally so the caller
 * keeps ownership of its own buffer.
 * Returns 1 if a new key was added, 0 if it was already present, -1 on OOM.
 * Average O(1) plus the cost of hashing the key.
 */
int string_set_add(StringSet *set, const char *key) {
    uint32_t idx = djb2(key) % (uint32_t)set->bucket_count;
    for (StringNode *n = set->buckets[idx]; n; n = n->next) {
        if (strcmp(n->key, key) == 0) return 0; /* duplicate; nothing to do */
    }
    StringNode *node = (StringNode *)malloc(sizeof(StringNode));
    if (!node) return -1;
    node->key = strdup(key);                    /* own a private copy */
    if (!node->key) { free(node); return -1; }
    node->next = set->buckets[idx];             /* prepend onto the chain */
    set->buckets[idx] = node;
    return 1;
}

/*
 * Test whether a key is a member of the set.
 *
 * Returns 1 if present, 0 otherwise. Walks one bucket chain, so average O(1).
 */
int string_set_contains(const StringSet *set, const char *key) {
    uint32_t idx = djb2(key) % (uint32_t)set->bucket_count;
    for (StringNode *n = set->buckets[idx]; n; n = n->next) {
        if (strcmp(n->key, key) == 0) return 1;
    }
    return 0;
}

/*
 * Free a StringSet and every key it owns.
 *
 * Safe to call with NULL. After return the pointer is dangling and must not
 * be reused.
 */
void string_set_free(StringSet *set) {
    if (!set) return;
    for (size_t i = 0; i < set->bucket_count; i++) {
        StringNode *n = set->buckets[i];
        while (n) {
            StringNode *next = n->next; /* save before we free the node */
            free(n->key);
            free(n);
            n = next;
        }
    }
    free(set->buckets);
    free(set);
}
