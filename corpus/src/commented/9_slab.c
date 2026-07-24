/*
 * slab.c — A slab allocator that grows by attaching fixed-size slabs.
 *
 * Each slab is a contiguous run of equally sized objects backed by a bitmap
 * marking which slots are occupied. The allocator chains slabs together and,
 * on allocation, scans existing slabs for a free slot before reporting that a
 * fresh slab is needed. This amortizes bookkeeping and keeps objects of one
 * size class densely packed.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define SLOTS_PER_SLAB 64 /* one 64-bit word's worth of bitmap, for simplicity */

/*
 * One slab: a buffer of SLOTS_PER_SLAB objects plus an occupancy bitmap where
 * bit i set means slot i is in use. Slabs are linked into a list.
 */
typedef struct Slab {
    uint8_t     *objects;  /* base of the object array (object_size * SLOTS)  */
    uint64_t     bitmap;   /* occupancy bits; bit i == 1 means slot i is used */
    struct Slab *next;     /* next slab in the cache's chain, or NULL         */
} Slab;

/* A cache manages all slabs for a single object size class. */
typedef struct SlabCache {
    size_t object_size; /* size in bytes of each object in this cache        */
    Slab  *slabs;       /* head of the slab chain                            */
} SlabCache;

/*
 * Initialize a cache for objects of `object_size` bytes.
 * Starts with no slabs; the first allocation reports that one is needed.
 */
void slab_cache_init(SlabCache *cache, size_t object_size) {
    cache->object_size = object_size;
    cache->slabs = NULL;
}

/*
 * Attach a pre-allocated slab to the cache. `slab` holds the metadata and
 * `object_storage` is a buffer of at least object_size * SLOTS_PER_SLAB bytes.
 * The new slab becomes the chain head with all slots free.
 */
void slab_cache_add_slab(SlabCache *cache, Slab *slab, void *object_storage) {
    slab->objects = (uint8_t *)object_storage;
    slab->bitmap = 0; /* all slots start free */
    slab->next = cache->slabs;
    cache->slabs = slab;
}

/*
 * Find the index of the lowest clear bit in `bitmap`.
 * Returns 0..63 for the first free slot, or -1 if every slot is occupied.
 * Implemented with a linear bit scan, O(SLOTS_PER_SLAB) in the worst case.
 */
static int first_free_slot(uint64_t bitmap) {
    for (int i = 0; i < SLOTS_PER_SLAB; i++) {
        if ((bitmap & ((uint64_t)1 << i)) == 0) {
            return i; /* this slot is free */
        }
    }
    return -1; /* slab is full */
}

/*
 * Allocate one object from the cache.
 * Scans existing slabs for a free slot and returns its address, marking the
 * slot occupied. Returns NULL when every attached slab is full, signaling the
 * caller to add another slab and retry. The returned memory is uninitialized.
 */
void *slab_alloc(SlabCache *cache) {
    for (Slab *slab = cache->slabs; slab != NULL; slab = slab->next) {
        int slot = first_free_slot(slab->bitmap);
        if (slot >= 0) {
            slab->bitmap |= ((uint64_t)1 << slot); /* claim the slot */
            return slab->objects + (size_t)slot * cache->object_size;
        }
    }
    return NULL; /* all slabs full; caller should grow the cache */
}

/*
 * Return an object to its slab.
 * Locates the owning slab by address range, clears the corresponding bitmap
 * bit, and returns 1 on success. Returns 0 if `ptr` belongs to no slab in
 * this cache (e.g. a foreign or already-freed pointer). O(number of slabs).
 */
int slab_free(SlabCache *cache, void *ptr) {
    uint8_t *p = (uint8_t *)ptr;

    for (Slab *slab = cache->slabs; slab != NULL; slab = slab->next) {
        uint8_t *start = slab->objects;
        uint8_t *end = start + (size_t)SLOTS_PER_SLAB * cache->object_size;

        if (p >= start && p < end) {
            /* Recover the slot index from the offset within the slab. */
            size_t slot = (size_t)(p - start) / cache->object_size;
            slab->bitmap &= ~((uint64_t)1 << slot); /* release the slot */
            return 1;
        }
    }
    return 0; /* pointer not owned by this cache */
}

/*
 * Count how many objects are currently allocated across the whole cache.
 * Sums the set bits of every slab's bitmap. Returns the live object count.
 */
size_t slab_live_count(const SlabCache *cache) {
    size_t total = 0;
    for (const Slab *slab = cache->slabs; slab != NULL; slab = slab->next) {
        uint64_t bits = slab->bitmap;
        /* Brian Kernighan's bit-count: clear the lowest set bit each pass. */
        while (bits) {
            bits &= bits - 1;
            total++;
        }
    }
    return total;
}
