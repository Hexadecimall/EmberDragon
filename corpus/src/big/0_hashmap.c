/*
 * Separate-chaining hash map from NUL-terminated string keys to integers.
 *
 * Keys are hashed with the FNV-1a algorithm and collisions are resolved by
 * chaining entries in per-bucket singly linked lists. The table grows and
 * rehashes when the load factor passes 0.75, keeping expected lookup,
 * insert, and delete at O(1). Keys are copied on insert, so callers retain
 * ownership of the strings they pass in.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* One key/value pair living in a bucket chain. */
typedef struct HashEntry {
    char *key;               /* owned copy of the caller's key string */
    int32_t value;
    struct HashEntry *next;  /* next entry hashing to the same bucket */
} HashEntry;

/* The table: an array of bucket-chain heads plus bookkeeping counters. */
typedef struct {
    HashEntry **buckets;  /* array of `bucket_count` chain heads */
    uint32_t bucket_count;
    uint32_t size;        /* number of stored key/value pairs */
} HashMap;

/*
 * FNV-1a hash over a NUL-terminated string.
 * @return a 32-bit hash; the caller reduces it modulo the bucket count.
 */
static uint32_t fnv1a(const char *key) {
    uint32_t hash = 2166136261u;  /* FNV offset basis */
    while (*key != '\0') {
        hash ^= (uint8_t)*key;    /* mix in the next byte */
        hash *= 16777619u;        /* FNV prime */
        key++;
    }
    return hash;
}

/*
 * Initialize an empty map with `initial_buckets` (or 16 if 0 is passed).
 * @return 0 on success, -1 on allocation failure.
 */
int hashmap_init(HashMap *map, uint32_t initial_buckets) {
    if (initial_buckets == 0) {
        initial_buckets = 16;
    }
    /* calloc zeroes the array so every chain head starts as NULL. */
    map->buckets = (HashEntry **)calloc(initial_buckets, sizeof(HashEntry *));
    if (map->buckets == NULL) {
        return -1;
    }
    map->bucket_count = initial_buckets;
    map->size = 0;
    return 0;
}

/*
 * Grow the table to `new_count` buckets and re-link every entry.
 * Entries are moved (not reallocated), so their key copies are preserved.
 * @return 0 on success, -1 if the new bucket array could not be allocated.
 */
static int hashmap_rehash(HashMap *map, uint32_t new_count) {
    HashEntry **fresh = (HashEntry **)calloc(new_count, sizeof(HashEntry *));
    if (fresh == NULL) {
        return -1;
    }
    /* Walk every old chain and re-home each entry in the new array. */
    for (uint32_t i = 0; i < map->bucket_count; i++) {
        HashEntry *entry = map->buckets[i];
        while (entry != NULL) {
            HashEntry *next = entry->next;          /* save before relinking */
            uint32_t idx = fnv1a(entry->key) % new_count;
            entry->next = fresh[idx];               /* prepend into new bucket */
            fresh[idx] = entry;
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = fresh;
    map->bucket_count = new_count;
    return 0;
}

/*
 * Insert or update the mapping `key -> value`.
 * If the key already exists its value is overwritten in place; otherwise a
 * new entry with a copied key is prepended to its bucket. Triggers a 2x
 * rehash once the load factor exceeds 0.75.
 * @return 0 on success, -1 on allocation failure.
 */
int hashmap_put(HashMap *map, const char *key, int32_t value) {
    uint32_t idx = fnv1a(key) % map->bucket_count;
    for (HashEntry *e = map->buckets[idx]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;  /* update existing mapping */
            return 0;
        }
    }
    /* Not found: build a new entry. */
    HashEntry *entry = (HashEntry *)malloc(sizeof(HashEntry));
    if (entry == NULL) {
        return -1;
    }
    size_t len = strlen(key) + 1;
    entry->key = (char *)malloc(len);
    if (entry->key == NULL) {
        free(entry);
        return -1;
    }
    memcpy(entry->key, key, len);
    entry->value = value;
    entry->next = map->buckets[idx];  /* prepend to the chain */
    map->buckets[idx] = entry;
    map->size++;

    /* Grow when load factor count/buckets exceeds 3/4 (compared via *4 to
     * avoid any non-integer arithmetic). */
    if (map->size * 4 > map->bucket_count * 3) {
        hashmap_rehash(map, map->bucket_count * 2);  /* best-effort growth */
    }
    return 0;
}

/*
 * Look up the value stored under `key`.
 * @param out_value  receives the value if the key is present.
 * @return 1 if found (and *out_value set), 0 if the key is absent.
 */
int hashmap_get(const HashMap *map, const char *key, int32_t *out_value) {
    uint32_t idx = fnv1a(key) % map->bucket_count;
    for (HashEntry *e = map->buckets[idx]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            *out_value = e->value;
            return 1;
        }
    }
    return 0;
}

/*
 * Remove the mapping for `key` if present.
 * @return 1 if an entry was removed, 0 if the key was not found.
 */
int hashmap_remove(HashMap *map, const char *key) {
    uint32_t idx = fnv1a(key) % map->bucket_count;
    HashEntry *prev = NULL;
    HashEntry *cur = map->buckets[idx];
    while (cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            /* Unlink: patch either the bucket head or the predecessor. */
            if (prev == NULL) {
                map->buckets[idx] = cur->next;
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

/* Free every entry, every key copy, and the bucket array itself. */
void hashmap_destroy(HashMap *map) {
    for (uint32_t i = 0; i < map->bucket_count; i++) {
        HashEntry *entry = map->buckets[i];
        while (entry != NULL) {
            HashEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = NULL;
    map->bucket_count = 0;
    map->size = 0;
}
