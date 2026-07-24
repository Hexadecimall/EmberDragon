/*
 * Fixed-size object pool.
 *
 * Pre-allocates a contiguous array of equally sized slots and tracks which are
 * free using an intrusive singly linked list threaded through the free slots
 * themselves. Acquire and release are O(1) with no per-object heap traffic,
 * which makes this ideal for high-churn objects of one type (particles,
 * network packets, tree nodes).
 */

#include <stdint.h>
#include <stdlib.h>

/* When a slot is free, its first bytes hold a FreeNode linking it to the next
 * free slot. When in use, the same bytes hold caller data. */
typedef struct FreeNode {
    struct FreeNode *next;
} FreeNode;

/* Owns the slab of slots and the head of the free list. */
typedef struct ObjectPool {
    uint8_t  *slots;      /* base of the slab: slot_count * slot_size bytes */
    FreeNode *free_head;  /* next slot to hand out, or NULL when exhausted */
    size_t    slot_size;  /* bytes per slot (>= sizeof(FreeNode)) */
    size_t    slot_count; /* number of slots in the slab */
    size_t    in_use;     /* slots currently acquired, for bookkeeping */
} ObjectPool;

/*
 * Initialize a pool of `slot_count` objects, each at least `object_size` bytes.
 * Slots are padded up to hold a FreeNode so free slots can be threaded.
 * Returns 0 on success, -1 on allocation failure. Caller must call
 * pool_destroy to release the slab.
 */
int pool_init(ObjectPool *pool, size_t object_size, size_t slot_count) {
    /* A slot must be wide enough to store the free-list link while idle. */
    size_t slot_size = object_size < sizeof(FreeNode)
                           ? sizeof(FreeNode)
                           : object_size;

    pool->slots = (uint8_t *)malloc(slot_size * slot_count);
    if (pool->slots == NULL) {
        return -1;
    }
    pool->slot_size = slot_size;
    pool->slot_count = slot_count;
    pool->in_use = 0;

    /* Thread every slot onto the free list in order. Walking backward would
     * also work; forward keeps the initial hand-out order ascending. */
    pool->free_head = NULL;
    for (size_t i = slot_count; i-- > 0;) {
        FreeNode *node = (FreeNode *)(pool->slots + i * slot_size);
        node->next = pool->free_head;
        pool->free_head = node;
    }
    return 0;
}

/*
 * Acquire one slot from the pool. Returns a pointer to uninitialized slot
 * memory, or NULL if the pool is exhausted. O(1): pops the free-list head.
 */
void *pool_acquire(ObjectPool *pool) {
    FreeNode *node = pool->free_head;
    if (node == NULL) {
        return NULL; /* every slot is in use */
    }
    pool->free_head = node->next; /* unlink the head */
    pool->in_use++;
    return (void *)node;
}

/*
 * Return whether `ptr` points at a valid, properly aligned slot within this
 * pool's slab. Used to guard release against foreign or misaligned pointers.
 */
static int pool_owns(const ObjectPool *pool, const void *ptr) {
    const uint8_t *p = (const uint8_t *)ptr;
    if (p < pool->slots) {
        return 0; /* before the slab */
    }
    size_t span = pool->slot_size * pool->slot_count;
    if (p >= pool->slots + span) {
        return 0; /* past the slab */
    }
    /* Reject addresses that do not land exactly on a slot boundary. */
    return ((size_t)(p - pool->slots) % pool->slot_size) == 0;
}

/*
 * Release a slot previously returned by pool_acquire back to the pool. Ignores
 * NULL and pointers the pool does not own, returning -1 in the latter case and
 * 0 on a successful release. O(1): pushes onto the free-list head.
 */
int pool_release(ObjectPool *pool, void *ptr) {
    if (ptr == NULL) {
        return 0;
    }
    if (!pool_owns(pool, ptr)) {
        return -1; /* not one of our slots; refuse to corrupt the list */
    }
    FreeNode *node = (FreeNode *)ptr;
    node->next = pool->free_head; /* push onto the free list */
    pool->free_head = node;
    pool->in_use--;
    return 0;
}

/*
 * Number of slots still available to acquire without exhausting the pool.
 * Derived from the fixed total minus the in-use count, so it is O(1).
 */
size_t pool_available(const ObjectPool *pool) {
    return pool->slot_count - pool->in_use;
}

/*
 * Release the slab and clear the pool fields. Safe on a zero-initialized pool.
 * Any outstanding slot pointers become dangling after this call.
 */
void pool_destroy(ObjectPool *pool) {
    free(pool->slots);
    pool->slots = NULL;
    pool->free_head = NULL;
    pool->slot_count = 0;
    pool->in_use = 0;
}
