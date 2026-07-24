/*
 * Generational handle pool.
 *
 * A fixed-capacity store that hands out small integer "handles" instead of raw
 * pointers. Each slot carries a generation counter that is bumped on free, so a
 * stale handle to a recycled slot can be detected and rejected. This gives
 * safe, dangling-free references for game entities, GPU resources, or any
 * long-lived object table.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* A 32-bit handle packs a slot index in the low 16 bits and a generation in
 * the high 16 bits. Index 0 / generation 0 is reserved as the null handle. */
typedef uint32_t Handle;

#define HANDLE_NULL        ((Handle)0)
#define HANDLE_INDEX_BITS  16
#define HANDLE_INDEX_MASK  ((1u << HANDLE_INDEX_BITS) - 1u)

/* The user payload stored in each slot. Kept simple and trivially copyable. */
typedef struct Entity {
    int32_t x;
    int32_t y;
    int32_t health;
} Entity;

/* One table slot: live payload plus the metadata to validate handles. */
typedef struct Slot {
    Entity   entity;
    uint16_t generation; /* incremented each time the slot is freed */
    uint8_t  active;     /* 1 while the slot holds a live entity */
} Slot;

/* The pool: a flat array of slots plus a free-index stack. */
typedef struct HandlePool {
    Slot     *slots;       /* capacity slots */
    uint16_t *free_stack;  /* indices of free slots */
    size_t    capacity;    /* number of slots */
    size_t    free_top;    /* number of entries on the free stack */
} HandlePool;

/* Build a handle from a slot index and generation. */
static Handle make_handle(uint16_t index, uint16_t generation) {
    /* +1 on the index keeps slot 0 usable while reserving 0 as the null value. */
    return ((Handle)generation << HANDLE_INDEX_BITS) | (Handle)(index + 1);
}

/* Extract the slot index from a non-null handle. */
static uint16_t handle_index(Handle h) {
    return (uint16_t)((h & HANDLE_INDEX_MASK) - 1);
}

/* Extract the generation from a handle. */
static uint16_t handle_generation(Handle h) {
    return (uint16_t)(h >> HANDLE_INDEX_BITS);
}

/*
 * Initialize a pool holding up to `capacity` entities. Allocates the slot array
 * and free stack, pushing every index as free. Returns 0 on success or -1 on
 * allocation failure. Capacity must not exceed the index field's range.
 */
int handlepool_init(HandlePool *pool, size_t capacity) {
    if (capacity == 0 || capacity > HANDLE_INDEX_MASK) {
        return -1; /* would overflow the packed index */
    }
    pool->slots = (Slot *)calloc(capacity, sizeof(Slot));
    if (pool->slots == NULL) {
        return -1;
    }
    pool->free_stack = (uint16_t *)malloc(capacity * sizeof(uint16_t));
    if (pool->free_stack == NULL) {
        free(pool->slots);
        pool->slots = NULL;
        return -1;
    }
    pool->capacity = capacity;

    /* Seed the free stack with all indices, highest on top so the first
     * allocation hands out index 0. */
    for (size_t i = 0; i < capacity; i++) {
        pool->free_stack[i] = (uint16_t)(capacity - 1 - i);
    }
    pool->free_top = capacity;
    return 0;
}

/*
 * Allocate a slot and copy `value` into it, returning a fresh handle. Returns
 * HANDLE_NULL if the pool is full. O(1): pops a free index off the stack.
 */
Handle handlepool_create(HandlePool *pool, Entity value) {
    if (pool->free_top == 0) {
        return HANDLE_NULL; /* no free slots */
    }
    uint16_t index = pool->free_stack[--pool->free_top];
    Slot *slot = &pool->slots[index];
    slot->entity = value;
    slot->active = 1;
    return make_handle(index, slot->generation);
}

/*
 * Resolve a handle to the live Entity it refers to, or NULL if the handle is
 * null, out of range, or stale (its generation no longer matches the slot).
 * O(1); does not transfer ownership.
 */
Entity *handlepool_get(HandlePool *pool, Handle h) {
    if (h == HANDLE_NULL) {
        return NULL;
    }
    uint16_t index = handle_index(h);
    if (index >= pool->capacity) {
        return NULL; /* index outside the table */
    }
    Slot *slot = &pool->slots[index];

    /* The handle is valid only if the slot is live and the generations agree;
     * a mismatch means this handle outlived the object it once named. */
    if (!slot->active || slot->generation != handle_generation(h)) {
        return NULL;
    }
    return &slot->entity;
}

/*
 * Destroy the entity referenced by `h`, freeing its slot for reuse and bumping
 * the slot's generation so existing copies of the handle become stale. Returns
 * 0 on success or -1 if the handle was already invalid. O(1).
 */
int handlepool_destroy(HandlePool *pool, Handle h) {
    Entity *e = handlepool_get(pool, h); /* reuse validation logic */
    if (e == NULL) {
        return -1;
    }
    uint16_t index = handle_index(h);
    Slot *slot = &pool->slots[index];

    slot->active = 0;
    slot->generation++; /* invalidate every handle that named this slot */
    pool->free_stack[pool->free_top++] = index; /* return the slot */
    return 0;
}

/* Number of live entities currently stored. O(1). */
size_t handlepool_count(const HandlePool *pool) {
    return pool->capacity - pool->free_top;
}

/*
 * Release the slot array and free stack and clear all fields. Safe on a
 * zero-initialized pool. Every outstanding handle becomes invalid.
 */
void handlepool_destroy_all(HandlePool *pool) {
    free(pool->slots);
    free(pool->free_stack);
    pool->slots = NULL;
    pool->free_stack = NULL;
    pool->capacity = 0;
    pool->free_top = 0;
}
