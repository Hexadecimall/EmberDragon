/*
 * arena.c — A bump (arena) allocator over a caller-supplied buffer.
 *
 * An arena hands out memory by simply advancing a cursor through a fixed
 * backing buffer. Individual allocations are never freed; instead the whole
 * arena is reset or rewound at once, which makes allocation O(1) and frees
 * essentially free. This pattern suits per-frame or per-request scratch memory.
 */

#include <stdint.h>
#include <stddef.h>

/* An arena owns a contiguous byte buffer and a high-water cursor into it. */
typedef struct Arena {
    uint8_t *base;     /* first byte of the backing buffer                  */
    size_t   capacity; /* total number of bytes the buffer can hold         */
    size_t   offset;   /* bytes already handed out (the bump cursor)        */
} Arena;

/*
 * Initialize an arena to manage the given buffer.
 * `buffer` must remain valid for the arena's lifetime; the arena does not
 * take ownership and never frees it. Resets the cursor to the start.
 */
void arena_init(Arena *arena, void *buffer, size_t capacity) {
    arena->base = (uint8_t *)buffer;
    arena->capacity = capacity;
    arena->offset = 0;
}

/*
 * Round `n` up to the next multiple of `align`, which must be a power of two.
 * Returns the aligned value. Used to satisfy alignment requirements of the
 * type being allocated.
 */
static size_t align_up(size_t n, size_t align) {
    /* Adding (align-1) then masking off the low bits snaps up to a boundary. */
    return (n + (align - 1)) & ~(align - 1);
}

/*
 * Allocate `size` bytes aligned to `align` (a power of two) from the arena.
 * Returns a pointer into the backing buffer, or NULL if the request would
 * overflow the remaining capacity. The returned memory is uninitialized and
 * must not be freed individually. O(1).
 */
void *arena_alloc(Arena *arena, size_t size, size_t align) {
    /* Advance the cursor to a properly aligned start for this allocation. */
    size_t aligned = align_up(arena->offset, align);

    /* Reject the request if it would not fit; this also guards overflow
     * because `aligned` and `size` are bounded by capacity in practice. */
    if (aligned + size > arena->capacity) {
        return NULL;
    }

    void *result = arena->base + aligned;
    arena->offset = aligned + size; /* commit the bump */
    return result;
}

/*
 * Return the current cursor position as an opaque marker.
 * Pair with arena_rewind to free everything allocated after this point,
 * forming a simple nested (stack-like) lifetime.
 */
size_t arena_mark(const Arena *arena) {
    return arena->offset;
}

/*
 * Roll the cursor back to a previously obtained marker, logically freeing
 * all allocations made since the mark. The marker must not exceed the
 * current offset; out-of-range markers are clamped and ignored.
 */
void arena_rewind(Arena *arena, size_t marker) {
    if (marker <= arena->offset) {
        arena->offset = marker;
    }
}

/*
 * Free every allocation at once by resetting the cursor to the start.
 * The backing buffer is left intact and ready for reuse. O(1).
 */
void arena_reset(Arena *arena) {
    arena->offset = 0;
}

/*
 * Report how many bytes remain available before the arena is exhausted.
 * Returns the unused capacity in bytes.
 */
size_t arena_bytes_remaining(const Arena *arena) {
    return arena->capacity - arena->offset;
}
