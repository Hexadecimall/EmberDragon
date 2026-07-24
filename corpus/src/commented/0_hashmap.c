/*
 * String-to-integer hash map using separate chaining.
 *
 * Collisions are resolved with per-bucket singly linked lists. Keys are
 * copied on insert and owned by the map. The bucket count is fixed at
 * construction; load is not rebalanced, so choose a capacity suited to
 * the expected key count.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A single key/value pair living in a bucket's collision chain. */
typedef struct Entry {
    char *key;
    int value;
    struct Entry *next;
} Entry;

/* The map: an array of `bucketCount` chain heads. */
typedef struct {
    Entry **buckets;
    int bucketCount;
    int size; /* Total number of stored pairs across all buckets. */
} HashMap;

/*
 * Compute the FNV-1a hash of a NUL-terminated string.
 * Returns a 32-bit hash; the caller reduces it modulo the bucket count.
 */
static uint32_t hashKey(const char *key) {
    uint32_t hash = 2166136261u; /* FNV offset basis. */
    while (*key) {
        hash ^= (uint32_t)(unsigned char)(*key);
        hash *= 16777619u; /* FNV prime. */
        key++;
    }
    return hash;
}

/*
 * Create an empty map with `bucketCount` buckets (forced to at least 1).
 * Returns a heap-allocated map the caller owns, or NULL on failure.
 */
HashMap *mapCreate(int bucketCount) {
    if (bucketCount < 1) {
        bucketCount = 1;
    }
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (map == NULL) {
        return NULL;
    }
    /* calloc zeroes the array so every chain starts empty (NULL). */
    map->buckets = (Entry **)calloc((size_t)bucketCount, sizeof(Entry *));
    if (map->buckets == NULL) {
        free(map);
        return NULL;
    }
    map->bucketCount = bucketCount;
    map->size = 0;
    return map;
}

/*
 * Insert or update `key` -> `value`. The key string is duplicated, so
 * the caller keeps ownership of its own buffer. O(1) average.
 * Returns 1 on success, 0 on allocation failure.
 */
int mapPut(HashMap *map, const char *key, int value) {
    uint32_t index = hashKey(key) % (uint32_t)map->bucketCount;
    /* Walk the chain looking for an existing key to overwrite. */
    for (Entry *e = map->buckets[index]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return 1;
        }
    }
    /* Not present: prepend a fresh entry to the bucket's chain. */
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    if (entry == NULL) {
        return 0;
    }
    entry->key = strdup(key);
    if (entry->key == NULL) {
        free(entry);
        return 0;
    }
    entry->value = value;
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;
    return 1;
}

/*
 * Look up `key`, writing its value to `*out` if found. O(1) average.
 * Returns 1 if the key exists, 0 otherwise (and `*out` is untouched).
 */
int mapGet(const HashMap *map, const char *key, int *out) {
    uint32_t index = hashKey(key) % (uint32_t)map->bucketCount;
    for (Entry *e = map->buckets[index]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            *out = e->value;
            return 1;
        }
    }
    return 0;
}

/*
 * Remove `key` and free its entry. O(1) average.
 * Returns 1 if a pair was removed, 0 if the key was absent.
 */
int mapRemove(HashMap *map, const char *key) {
    uint32_t index = hashKey(key) % (uint32_t)map->bucketCount;
    Entry *prev = NULL;
    Entry *cur = map->buckets[index];
    while (cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[index] = cur->next; /* Unlink the chain head. */
            } else {
                prev->next = cur->next;
            }
            free(cur->key);
            free(cur);
            map->size--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

/*
 * Free every entry, every duplicated key, the bucket array, and the map.
 * Passing NULL is a no-op.
 */
void mapDestroy(HashMap *map) {
    if (map == NULL) {
        return;
    }
    for (int i = 0; i < map->bucketCount; i++) {
        Entry *cur = map->buckets[i];
        while (cur != NULL) {
            Entry *next = cur->next;
            free(cur->key);
            free(cur);
            cur = next;
        }
    }
    free(map->buckets);
    free(map);
}
