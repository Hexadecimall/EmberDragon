/*
 * Arena (bump) allocator.
 *
 * A linear allocator that carves allocations out of a single contiguous
 * buffer by advancing a cursor. Individual objects cannot be freed; instead
 * the whole arena is reset or destroyed at once. This makes allocation a few
 * pointer arithmetic operations and is ideal for per-frame or per-request
 * scratch memory where lifetimes are bulk-managed.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Backing store plus the bookkeeping needed to hand out aligned slices. */
typedef struct Arena {
    uint8_t *base;     /* start of the owned memory region */
    size_t   capacity; /* total bytes in the region */
    size_t   offset;   /* bytes already handed out (the bump cursor) */
} Arena;

/*
 * Initialize an arena over a freshly malloc'd region of `capacity` bytes.
 * Returns 0 on success, or -1 if the backing allocation failed. The caller
 * must eventually call arena_destroy to release the region.
 */
int arena_init(Arena *arena, size_t capacity) {
    arena->base = (uint8_t *)malloc(capacity);
    if (arena->base == NULL) {
        return -1; /* propagate OOM rather than crash on first alloc */
    }
    arena->capacity = capacity;
    arena->offset = 0;
    return 0;
}

/*
 * Round `value` up to the next multiple of `align`, which must be a power of
 * two. Used to satisfy alignment requirements of the requested type.
 */
static size_t align_up(size_t value, size_t align) {
    /* (value + align - 1) & ~(align - 1) is the classic power-of-two round-up. */
    return (value + (align - 1)) & ~(align - 1);
}

/*
 * Allocate `size` bytes aligned to `align` from the arena. Returns a pointer
 * into the arena, or NULL if there is not enough room left. The returned
 * memory is uninitialized and is owned by the arena (do not free it).
 */
void *arena_alloc(Arena *arena, size_t size, size_t align) {
    /* Advance the cursor to a properly aligned address before measuring fit. */
    size_t aligned = align_up(arena->offset, align);

    /* Reject if the aligned start plus the request would overrun the buffer.
     * Checking against capacity here is the single bounds guard for the arena. */
    if (aligned + size > arena->capacity) {
        return NULL;
    }

    void *ptr = arena->base + aligned;
    arena->offset = aligned + size; /* commit the bump */
    return ptr;
}

/*
 * Allocate zero-initialized memory, mirroring calloc semantics. Returns NULL
 * if the arena cannot satisfy the request.
 */
void *arena_calloc(Arena *arena, size_t size, size_t align) {
    void *ptr = arena_alloc(arena, size, align);
    if (ptr != NULL) {
        memset(ptr, 0, size); /* arena_alloc leaves bytes undefined; clear them */
    }
    return ptr;
}

/*
 * Reset the arena to empty without freeing the backing store, so the region
 * can be reused for a new batch of allocations. Existing pointers into the
 * arena become dangling and must not be used afterward.
 */
void arena_reset(Arena *arena) {
    arena->offset = 0;
}

/*
 * Bytes still available for allocation, ignoring alignment padding. Useful for
 * deciding whether to grow or spill before attempting a large allocation.
 */
size_t arena_remaining(const Arena *arena) {
    return arena->capacity - arena->offset;
}

/*
 * Release the backing store and clear the arena fields so reuse-after-free is
 * easier to catch. Safe to call on a zero-initialized arena.
 */
void arena_destroy(Arena *arena) {
    free(arena->base);
    arena->base = NULL;
    arena->capacity = 0;
    arena->offset = 0;
}
