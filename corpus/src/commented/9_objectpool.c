/*
 * objectpool.c — A fixed-size object pool with an intrusive free list.
 *
 * The pool pre-carves a backing buffer into equally sized slots and threads
 * the free slots onto a singly linked list whose links are stored inside the
 * slots themselves (no per-slot overhead while in use). Acquire pops the list
 * head and release pushes back, making both operations O(1) and pointer-stable.
 */

#include <stdint.h>
#include <stddef.h>

/*
 * While a slot is free, its first bytes hold a FreeNode linking to the next
 * free slot. While in use, the same bytes hold the caller's object. This
 * "intrusive" trick is why the slot size must be at least sizeof(FreeNode).
 */
typedef struct FreeNode {
    struct FreeNode *next;
} FreeNode;

/* Bookkeeping for one pool: the buffer, the slot geometry, and the free head. */
typedef struct ObjectPool {
    uint8_t  *storage;    /* backing buffer holding all slots                 */
    size_t    slot_size;  /* bytes per slot (>= sizeof(FreeNode))             */
    size_t    slot_count; /* total number of slots                           */
    size_t    live_count; /* slots currently checked out                     */
    FreeNode *free_list;  /* head of the list of available slots             */
} ObjectPool;

/*
 * Initialize a pool that carves `buffer` into `slot_count` slots of
 * `slot_size` bytes each. `slot_size` is rounded up to hold a FreeNode if
 * needed. Threads every slot onto the free list in ascending address order.
 * The buffer must be at least slot_size * slot_count bytes.
 */
void pool_init(ObjectPool *pool, void *buffer, size_t slot_size,
               size_t slot_count) {
    /* A slot must be wide enough to store a free-list link when idle. */
    if (slot_size < sizeof(FreeNode)) {
        slot_size = sizeof(FreeNode);
    }

    pool->storage = (uint8_t *)buffer;
    pool->slot_size = slot_size;
    pool->slot_count = slot_count;
    pool->live_count = 0;
    pool->free_list = NULL;

    /* Build the free list from the last slot backward so the final list runs
     * forward through memory, which tends to be cache-friendlier on reuse. */
    for (size_t i = slot_count; i > 0; i--) {
        uint8_t *slot = pool->storage + (i - 1) * slot_size;
        FreeNode *node = (FreeNode *)slot;
        node->next = pool->free_list;
        pool->free_list = node;
    }
}

/*
 * Check out one slot from the pool.
 * Returns a pointer to slot_size bytes of uninitialized storage, or NULL if
 * the pool is exhausted. O(1). The pointer stays valid until released.
 */
void *pool_acquire(ObjectPool *pool) {
    FreeNode *node = pool->free_list;
    if (node == NULL) {
        return NULL; /* every slot is in use */
    }

    pool->free_list = node->next; /* pop the head */
    pool->live_count++;
    return node;
}

/*
 * Return a previously acquired slot to the pool.
 * Passing NULL is a no-op. O(1). The freed bytes are reused to store the
 * free-list link, so the caller must not touch the memory after releasing it.
 */
void pool_release(ObjectPool *pool, void *ptr) {
    if (ptr == NULL) {
        return;
    }

    FreeNode *node = (FreeNode *)ptr;
    node->next = pool->free_list; /* push onto the free list */
    pool->free_list = node;
    pool->live_count--;
}

/*
 * Report whether a pointer falls inside this pool's backing buffer and sits
 * on a slot boundary. Returns 1 if `ptr` is a valid slot address, else 0.
 * Useful for guarding against foreign or misaligned pointers before release.
 */
int pool_owns(const ObjectPool *pool, const void *ptr) {
    const uint8_t *p = (const uint8_t *)ptr;
    const uint8_t *start = pool->storage;
    const uint8_t *end = start + pool->slot_size * pool->slot_count;

    if (p < start || p >= end) {
        return 0; /* outside the buffer entirely */
    }
    /* Must land exactly on a slot start, not partway into one. */
    return ((size_t)(p - start) % pool->slot_size) == 0;
}

/*
 * Return the number of slots still available for acquisition.
 * Computed from the live count so it is O(1).
 */
size_t pool_available(const ObjectPool *pool) {
    return pool->slot_count - pool->live_count;
}
