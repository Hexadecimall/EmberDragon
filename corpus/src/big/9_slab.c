/*
 * Slab allocator for fixed-size objects.
 *
 * Allocates memory in large "slabs" (multi-object pages) and subdivides each
 * slab into equal object slots. Slabs are kept on a linked list and each slab
 * threads its free slots on an intrusive free list. New slabs are created on
 * demand and reclaimed when fully empty, giving cache-friendly batched
 * allocation for a single object size.
 */

#include <stdint.h>
#include <stdlib.h>

/* Intrusive link stored in a slot while it is free. */
typedef struct SlabFreeNode {
    struct SlabFreeNode *next;
} SlabFreeNode;

/* One slab: a block of `objects_per_slab` slots plus its own free list. */
typedef struct Slab {
    struct Slab  *next;       /* next slab in the cache's slab list */
    SlabFreeNode *free_list;  /* free slots within this slab */
    uint8_t      *memory;     /* base of this slab's slot block */
    size_t        used;       /* slots currently allocated from this slab */
} Slab;

/* A cache manages all slabs for one object size. */
typedef struct SlabCache {
    Slab  *slabs;             /* head of the slab list */
    size_t object_size;       /* padded slot size in bytes */
    size_t objects_per_slab;  /* slot count per slab */
} SlabCache;

/*
 * Initialize a cache that hands out objects of `object_size` bytes, grouping
 * `objects_per_slab` of them per slab. Sizes below a pointer are padded so
 * free slots can hold their list link. No memory is reserved until the first
 * allocation.
 */
void slab_cache_init(SlabCache *cache, size_t object_size,
                     size_t objects_per_slab) {
    if (object_size < sizeof(SlabFreeNode)) {
        object_size = sizeof(SlabFreeNode); /* must fit the free link */
    }
    cache->slabs = NULL;
    cache->object_size = object_size;
    cache->objects_per_slab = objects_per_slab;
}

/*
 * Allocate and fully populate a new slab, pushing it onto the cache's slab
 * list. Returns the new slab, or NULL on allocation failure. Internal helper
 * invoked when every existing slab is full.
 */
static Slab *slab_create(SlabCache *cache) {
    Slab *slab = (Slab *)malloc(sizeof(Slab));
    if (slab == NULL) {
        return NULL;
    }
    size_t block = cache->object_size * cache->objects_per_slab;
    slab->memory = (uint8_t *)malloc(block);
    if (slab->memory == NULL) {
        free(slab); /* roll back the partial slab on OOM */
        return NULL;
    }
    slab->used = 0;

    /* Thread all slots onto this slab's free list. */
    slab->free_list = NULL;
    for (size_t i = cache->objects_per_slab; i-- > 0;) {
        SlabFreeNode *node =
            (SlabFreeNode *)(slab->memory + i * cache->object_size);
        node->next = slab->free_list;
        slab->free_list = node;
    }

    slab->next = cache->slabs; /* link the slab into the cache */
    cache->slabs = slab;
    return slab;
}

/*
 * Allocate one object from the cache. Reuses a slab with free slots if one
 * exists, otherwise grows by one slab. Returns uninitialized object memory, or
 * NULL if a new slab was needed but could not be created. Amortized O(1).
 */
void *slab_alloc(SlabCache *cache) {
    /* Find the first slab with a free slot. */
    Slab *slab = cache->slabs;
    while (slab != NULL && slab->free_list == NULL) {
        slab = slab->next;
    }
    if (slab == NULL) {
        slab = slab_create(cache); /* all full (or none yet): grow */
        if (slab == NULL) {
            return NULL;
        }
    }

    SlabFreeNode *node = slab->free_list;
    slab->free_list = node->next; /* pop the free slot */
    slab->used++;
    return (void *)node;
}

/*
 * Return whether `ptr` falls inside `slab`'s slot block. Used to locate the
 * owning slab on free.
 */
static int slab_contains(const Slab *slab, const SlabCache *cache,
                         const void *ptr) {
    const uint8_t *p = (const uint8_t *)ptr;
    size_t block = cache->object_size * cache->objects_per_slab;
    return p >= slab->memory && p < slab->memory + block;
}

/*
 * Free an object back to the cache. Scans slabs to find the owner, returns the
 * slot to that slab's free list, and frees the slab entirely if it becomes
 * empty. Returns 0 on success or -1 if no slab owns `ptr`. NULL is a no-op.
 */
int slab_free(SlabCache *cache, void *ptr) {
    if (ptr == NULL) {
        return 0;
    }
    Slab *prev = NULL;
    Slab *slab = cache->slabs;
    while (slab != NULL) {
        if (slab_contains(slab, cache, ptr)) {
            SlabFreeNode *node = (SlabFreeNode *)ptr;
            node->next = slab->free_list; /* return the slot */
            slab->free_list = node;
            slab->used--;

            /* Reclaim wholly empty slabs to keep memory bounded. */
            if (slab->used == 0) {
                if (prev == NULL) {
                    cache->slabs = slab->next;
                } else {
                    prev->next = slab->next;
                }
                free(slab->memory);
                free(slab);
            }
            return 0;
        }
        prev = slab;
        slab = slab->next;
    }
    return -1; /* foreign pointer */
}

/*
 * Destroy the cache, freeing every slab and its backing memory. After this the
 * cache is empty and may be re-initialized. Any live objects become dangling.
 */
void slab_cache_destroy(SlabCache *cache) {
    Slab *slab = cache->slabs;
    while (slab != NULL) {
        Slab *next = slab->next; /* save before freeing the node */
        free(slab->memory);
        free(slab);
        slab = next;
    }
    cache->slabs = NULL;
}
