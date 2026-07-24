#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Separate-chaining hash map from string keys to integer counts. */
#define BUCKET_COUNT 16

struct Entry {
    char key[32];
    int value;
    struct Entry *next;
};

struct HashMap {
    struct Entry *buckets[BUCKET_COUNT];
    int size;
};

void map_init(struct HashMap *map) {
    int index;
    for (index = 0; index < BUCKET_COUNT; index = index + 1) {
        map->buckets[index] = NULL;
    }
    map->size = 0;
}

uint32_t hash_key(const char *key) {
    uint32_t hash = 2166136261u;
    int index = 0;
    while (key[index] != '\0') {
        hash = hash ^ (uint32_t)key[index];
        hash = hash * 16777619u;
        index = index + 1;
    }
    return hash;
}

int bucket_index(const char *key) {
    return (int)(hash_key(key) % BUCKET_COUNT);
}

struct Entry *map_find(struct HashMap *map, const char *key) {
    int slot = bucket_index(key);
    struct Entry *cursor = map->buckets[slot];
    while (cursor != NULL) {
        if (strcmp(cursor->key, key) == 0) {
            return cursor;
        }
        cursor = cursor->next;
    }
    return NULL;
}

void map_put(struct HashMap *map, const char *key, int value) {
    struct Entry *existing = map_find(map, key);
    if (existing != NULL) {
        existing->value = value;
        return;
    }
    int slot = bucket_index(key);
    struct Entry *entry = (struct Entry *)malloc(sizeof(struct Entry));
    strncpy(entry->key, key, 31);
    entry->key[31] = '\0';
    entry->value = value;
    entry->next = map->buckets[slot];
    map->buckets[slot] = entry;
    map->size = map->size + 1;
}

int map_get(struct HashMap *map, const char *key, int fallback) {
    struct Entry *entry = map_find(map, key);
    if (entry == NULL) {
        return fallback;
    }
    return entry->value;
}

void map_increment(struct HashMap *map, const char *key) {
    struct Entry *entry = map_find(map, key);
    if (entry == NULL) {
        map_put(map, key, 1);
    } else {
        entry->value = entry->value + 1;
    }
}

int map_remove(struct HashMap *map, const char *key) {
    int slot = bucket_index(key);
    struct Entry *prev = NULL;
    struct Entry *cursor = map->buckets[slot];
    while (cursor != NULL) {
        if (strcmp(cursor->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[slot] = cursor->next;
            } else {
                prev->next = cursor->next;
            }
            free(cursor);
            map->size = map->size - 1;
            return 1;
        }
        prev = cursor;
        cursor = cursor->next;
    }
    return 0;
}

int longest_chain(struct HashMap *map) {
    int best = 0;
    int slot;
    for (slot = 0; slot < BUCKET_COUNT; slot = slot + 1) {
        int length = 0;
        struct Entry *cursor = map->buckets[slot];
        while (cursor != NULL) {
            length = length + 1;
            cursor = cursor->next;
        }
        if (length > best) {
            best = length;
        }
    }
    return best;
}
