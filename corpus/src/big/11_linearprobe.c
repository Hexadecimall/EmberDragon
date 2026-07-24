/*
 * linearprobe.c — an open-addressing hash map with linear probing.
 *
 * Rather than chaining colliding keys in linked lists, this map stores all
 * entries inline in a single array and resolves collisions by scanning forward
 * to the next empty slot. Linear probing is extremely cache-friendly, and with
 * tombstone-based deletion plus automatic resizing it keeps lookups near O(1).
 * Keys are 64-bit integers and values are 64-bit integers.
 */

#include <stdint.h>
#include <stdlib.h>

/* Per-slot occupancy state. EMPTY slots terminate a probe scan; DELETED slots
 * (tombstones) do not, so a probe must keep going past them. */
typedef enum SlotState {
    SLOT_EMPTY = 0,
    SLOT_OCCUPIED = 1,
    SLOT_DELETED = 2
} SlotState;

/* One bucket: a key/value pair plus its occupancy state. */
typedef struct Slot {
    uint64_t  key;
    uint64_t  value;
    SlotState state;
} Slot;

/* The map: a power-of-two array of slots with bookkeeping counts. */
typedef struct HashMap {
    Slot    *slots;
    size_t   capacity; /* always a power of two so we can mask instead of mod */
    size_t   size;     /* number of OCCUPIED slots */
    size_t   tombstones; /* number of DELETED slots, counted toward load */
} HashMap;

/*
 * Mix a 64-bit key into a well-dispersed hash (Fibonacci/Murmur-style finalizer).
 *
 * Integer keys are often sequential or aligned, which clusters badly under
 * linear probing; this avalanche step scrambles them first. Returns the mixed
 * hash. O(1).
 */
static uint64_t hashmap_mix(uint64_t key) {
    key ^= key >> 33;
    key *= 0xFF51AFD7ED558CCDULL;
    key ^= key >> 33;
    key *= 0xC4CEB9FE1A85EC53ULL;
    key ^= key >> 33;
    return key;
}

/* Forward declaration: insertion is used by the resize routine. */
static int hashmap_put_internal(HashMap *map, uint64_t key, uint64_t value);

/*
 * Create a hash map with the given initial capacity (rounded to a power of two).
 *
 * initial_capacity: desired minimum number of slots; values below 8 are bumped
 * to 8 to avoid pathological early resizing.
 * Returns an empty map, or NULL on OOM. Caller frees it with hashmap_free.
 */
HashMap *hashmap_create(size_t initial_capacity) {
    size_t cap = 8;
    while (cap < initial_capacity) cap <<= 1; /* next power of two >= request */

    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    if (!map) return NULL;
    map->slots = (Slot *)calloc(cap, sizeof(Slot)); /* calloc => all SLOT_EMPTY */
    if (!map->slots) {
        free(map);
        return NULL;
    }
    map->capacity = cap;
    map->size = 0;
    map->tombstones = 0;
    return map;
}

/*
 * Grow the table to new_capacity and re-insert every live entry.
 *
 * Resizing also discards all tombstones, since probe chains are rebuilt from
 * scratch. Returns 0 on success or -1 on OOM (the old table is left intact).
 * O(new_capacity).
 */
static int hashmap_resize(HashMap *map, size_t new_capacity) {
    Slot *old_slots = map->slots;
    size_t old_cap = map->capacity;

    Slot *fresh = (Slot *)calloc(new_capacity, sizeof(Slot));
    if (!fresh) return -1;

    map->slots = fresh;
    map->capacity = new_capacity;
    map->size = 0;
    map->tombstones = 0;

    /* Re-hash only the occupied slots into the larger table. */
    for (size_t i = 0; i < old_cap; i++) {
        if (old_slots[i].state == SLOT_OCCUPIED) {
            hashmap_put_internal(map, old_slots[i].key, old_slots[i].value);
        }
    }
    free(old_slots);
    return 0;
}

/*
 * Insert or overwrite a key/value pair, assuming there is room.
 *
 * This is the core probe loop shared by the public put and by resize. It does
 * not itself trigger resizing. Returns 1 if a new key was added, 0 if an
 * existing key's value was updated. Average O(1).
 */
static int hashmap_put_internal(HashMap *map, uint64_t key, uint64_t value) {
    size_t mask = map->capacity - 1;
    size_t idx = (size_t)(hashmap_mix(key) & mask);
    long first_tombstone = -1; /* remember the earliest reusable tombstone */

    for (;;) {
        Slot *s = &map->slots[idx];
        if (s->state == SLOT_EMPTY) {
            /* End of the probe chain: the key is absent. Prefer filling a
             * tombstone we passed so the table stays compact. */
            if (first_tombstone >= 0) {
                Slot *t = &map->slots[(size_t)first_tombstone];
                t->key = key; t->value = value; t->state = SLOT_OCCUPIED;
                map->tombstones--;
            } else {
                s->key = key; s->value = value; s->state = SLOT_OCCUPIED;
            }
            map->size++;
            return 1;
        } else if (s->state == SLOT_DELETED) {
            if (first_tombstone < 0) first_tombstone = (long)idx;
        } else if (s->key == key) {
            s->value = value; /* same key: just overwrite the value */
            return 0;
        }
        idx = (idx + 1) & mask; /* linear step, wrapping around the array */
    }
}

/*
 * Insert or update a key, growing the table first if it is getting full.
 *
 * The load factor counts both live entries and tombstones; once they exceed
 * 70% of capacity we double the table to keep probe chains short.
 *
 * Returns 1 if the key was newly added, 0 if updated, -1 on OOM during resize.
 */
int hashmap_put(HashMap *map, uint64_t key, uint64_t value) {
    if ((map->size + map->tombstones + 1) * 10 >= map->capacity * 7) {
        if (hashmap_resize(map, map->capacity << 1) != 0) return -1;
    }
    return hashmap_put_internal(map, key, value);
}

/*
 * Look up a key.
 *
 * map/key: the table and the key to find. out_value: if non-NULL and the key
 * exists, receives the stored value.
 * Returns 1 if found, 0 if absent. The scan stops at the first EMPTY slot;
 * tombstones are skipped so they cannot hide a later match. Average O(1).
 */
int hashmap_get(const HashMap *map, uint64_t key, uint64_t *out_value) {
    size_t mask = map->capacity - 1;
    size_t idx = (size_t)(hashmap_mix(key) & mask);

    for (;;) {
        const Slot *s = &map->slots[idx];
        if (s->state == SLOT_EMPTY) return 0; /* probe chain ended: not found */
        if (s->state == SLOT_OCCUPIED && s->key == key) {
            if (out_value) *out_value = s->value;
            return 1;
        }
        idx = (idx + 1) & mask;
    }
}

/*
 * Remove a key by converting its slot into a tombstone.
 *
 * We cannot simply mark it EMPTY, because that would truncate the probe chain
 * and make later keys unreachable; a tombstone preserves the chain while
 * freeing the logical entry.
 *
 * Returns 1 if a key was removed, 0 if it was not present. Average O(1).
 */
int hashmap_remove(HashMap *map, uint64_t key) {
    size_t mask = map->capacity - 1;
    size_t idx = (size_t)(hashmap_mix(key) & mask);

    for (;;) {
        Slot *s = &map->slots[idx];
        if (s->state == SLOT_EMPTY) return 0;
        if (s->state == SLOT_OCCUPIED && s->key == key) {
            s->state = SLOT_DELETED;
            map->size--;
            map->tombstones++;
            return 1;
        }
        idx = (idx + 1) & mask;
    }
}

/*
 * Return the number of live key/value pairs in the map. O(1).
 */
size_t hashmap_size(const HashMap *map) {
    return map->size;
}

/*
 * Free the map and its slot array. Safe to call with NULL.
 */
void hashmap_free(HashMap *map) {
    if (!map) return;
    free(map->slots);
    free(map);
}
