/*
 * string_intern.c — A string interning pool backed by a hash table.
 *
 * Interning stores exactly one canonical copy of each distinct string and
 * hands out a stable identifier for it. Callers can then compare strings by
 * integer id instead of byte-by-byte, and repeated strings share storage.
 * Uses open-addressing with linear probing and grows when the table fills.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * One slot in the interning table. `text` is an owned, heap-allocated copy of
 * the canonical string; a NULL `text` marks an empty slot. `hash` caches the
 * string's hash so resizing and probing avoid recomputation.
 */
typedef struct InternEntry {
    char *text;
    uint32_t hash;
} InternEntry;

/*
 * The interning pool. `entries` is the open-addressing table of `capacity`
 * slots, `count` is the number of live strings, and `ids` maps a dense
 * integer id back to its slot index so callers get small, stable handles.
 */
typedef struct InternPool {
    InternEntry *entries;
    int *ids;
    int capacity;
    int count;
} InternPool;

/*
 * Compute the 32-bit FNV-1a hash of a NUL-terminated string.
 *
 * Returns the hash. FNV-1a is chosen for being tiny, fast, and well-spread
 * for short keys, with no external dependencies. Complexity: O(length).
 */
static uint32_t fnv1a_hash(const char *text) {
    uint32_t hash = 2166136261u;     /* FNV offset basis. */
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        hash ^= *p;
        hash *= 16777619u;           /* FNV prime; relies on uint32 wraparound. */
    }
    return hash;
}

/*
 * Create an interning pool with the given initial slot capacity.
 *
 * Returns a heap-allocated pool the caller frees with intern_pool_free, or
 * NULL on allocation failure. `initial_capacity` is rounded up to at least 8
 * to keep the load factor sane from the start.
 */
InternPool *intern_pool_create(int initial_capacity) {
    if (initial_capacity < 8) initial_capacity = 8;
    InternPool *pool = malloc(sizeof(InternPool));
    if (!pool) return NULL;

    pool->entries = calloc((size_t)initial_capacity, sizeof(InternEntry));
    pool->ids = malloc((size_t)initial_capacity * sizeof(int));
    if (!pool->entries || !pool->ids) {
        free(pool->entries);
        free(pool->ids);
        free(pool);
        return NULL;
    }
    pool->capacity = initial_capacity;
    pool->count = 0;
    return pool;
}

/*
 * Insert an owned string copy into a table without growing it.
 *
 * Places `(text, hash)` at the first empty slot found by linear probing.
 * Assumes a free slot exists (the caller guarantees headroom). Returns the
 * slot index used. Helper for both interning and resizing.
 */
static int table_place(InternEntry *entries, int capacity, char *text,
                       uint32_t hash) {
    int index = (int)(hash % (uint32_t)capacity);
    /* Linear probing: step forward until an empty slot turns up. */
    while (entries[index].text != NULL) {
        index = (index + 1) % capacity;
    }
    entries[index].text = text;
    entries[index].hash = hash;
    return index;
}

/*
 * Double the pool's capacity and rehash every live string into the new table.
 *
 * Returns 1 on success, 0 on allocation failure (in which case the pool is
 * left unchanged and usable). Called automatically when the load factor would
 * exceed roughly 70%. Complexity: O(capacity).
 */
static int intern_pool_grow(InternPool *pool) {
    int new_capacity = pool->capacity * 2;
    InternEntry *new_entries = calloc((size_t)new_capacity, sizeof(InternEntry));
    int *new_ids = malloc((size_t)new_capacity * sizeof(int));
    if (!new_entries || !new_ids) {
        free(new_entries);
        free(new_ids);
        return 0;
    }

    /* Rehash by id so the id->slot mapping stays consistent after the move. */
    for (int id = 0; id < pool->count; id++) {
        int old_slot = pool->ids[id];
        char *text = pool->entries[old_slot].text;
        uint32_t hash = pool->entries[old_slot].hash;
        int new_slot = table_place(new_entries, new_capacity, text, hash);
        new_ids[id] = new_slot;
    }

    free(pool->entries);
    free(pool->ids);
    pool->entries = new_entries;
    pool->ids = new_ids;
    pool->capacity = new_capacity;
    return 1;
}

/*
 * Intern `text`, returning a stable integer id for its canonical copy.
 *
 * If `text` was interned before, returns the existing id without copying.
 * Otherwise stores a fresh copy and returns a new id. Returns -1 on
 * allocation failure. Ids are dense (0,1,2,...) and never change for the life
 * of the pool. Complexity: amortized O(length).
 */
int intern_string(InternPool *pool, const char *text) {
    /* Grow before we get too full to keep probe chains short. */
    if (pool->count * 10 >= pool->capacity * 7) {
        if (!intern_pool_grow(pool)) return -1;
    }

    uint32_t hash = fnv1a_hash(text);
    int index = (int)(hash % (uint32_t)pool->capacity);

    /* Probe for either an existing match or an empty landing slot. */
    while (pool->entries[index].text != NULL) {
        if (pool->entries[index].hash == hash &&
            strcmp(pool->entries[index].text, text) == 0) {
            /* Already interned: find and return its existing id. */
            for (int id = 0; id < pool->count; id++) {
                if (pool->ids[id] == index) {
                    return id;
                }
            }
        }
        index = (index + 1) % pool->capacity;
    }

    /* Not present: duplicate the string so the pool owns its own copy. */
    char *copy = malloc(strlen(text) + 1);
    if (!copy) return -1;
    strcpy(copy, text);

    pool->entries[index].text = copy;
    pool->entries[index].hash = hash;
    int new_id = pool->count;
    pool->ids[new_id] = index;
    pool->count++;
    return new_id;
}

/*
 * Look up the canonical string for an interned id.
 *
 * Returns a pointer to the pool-owned string, or NULL if `id` is out of range.
 * The caller must not free or modify the returned pointer. Complexity: O(1).
 */
const char *intern_lookup(const InternPool *pool, int id) {
    if (id < 0 || id >= pool->count) return NULL;
    return pool->entries[pool->ids[id]].text;
}

/*
 * Free a pool and every string it owns.
 *
 * Safe to call with NULL. Invalidates all ids and string pointers previously
 * handed out. Complexity: O(capacity).
 */
void intern_pool_free(InternPool *pool) {
    if (!pool) return;
    for (int i = 0; i < pool->capacity; i++) {
        free(pool->entries[i].text); /* free(NULL) is a no-op for empty slots. */
    }
    free(pool->entries);
    free(pool->ids);
    free(pool);
}
