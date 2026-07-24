/*
 * stackalloc.c — A LIFO stack allocator with header-tracked deallocation.
 *
 * This allocator hands out memory by growing a top-of-stack pointer, but
 * unlike a plain bump arena it records the previous top in a small header in
 * front of each allocation. That lets the most recent allocation be popped
 * individually, so memory must be freed in strict reverse (last-in-first-out)
 * order. Markers allow rolling back many allocations at once.
 */

#include <stdint.h>
#include <stddef.h>

/*
 * Header stored immediately before every allocation's payload. It remembers
 * where the stack top was before this allocation, so freeing can restore it.
 */
typedef struct AllocHeader {
    size_t prev_top; /* stack offset that was current before this allocation */
} AllocHeader;

/* The stack allocator: a buffer and a single growing top offset. */
typedef struct StackAllocator {
    uint8_t *base;     /* start of the backing buffer        */
    size_t   capacity; /* size of the buffer in bytes        */
    size_t   top;      /* offset of the next free byte       */
} StackAllocator;

/*
 * Initialize the allocator over `buffer` of `capacity` bytes.
 * The buffer is borrowed, not owned, and the stack starts empty.
 */
void stack_init(StackAllocator *sa, void *buffer, size_t capacity) {
    sa->base = (uint8_t *)buffer;
    sa->capacity = capacity;
    sa->top = 0;
}

/*
 * Round `n` up to a multiple of `align` (a power of two).
 * Returns the aligned offset; helper for honoring object alignment.
 */
static size_t round_up(size_t n, size_t align) {
    return (n + (align - 1)) & ~(align - 1);
}

/*
 * Push an allocation of `size` bytes aligned to `align` onto the stack.
 * Returns a pointer to the payload, or NULL if it would overflow the buffer.
 * A hidden header precedes the payload to enable LIFO freeing. O(1).
 */
void *stack_push(StackAllocator *sa, size_t size, size_t align) {
    /* Reserve space for the header first, then align the payload start. */
    size_t header_at = sa->top;
    size_t payload_at = round_up(header_at + sizeof(AllocHeader), align);

    if (payload_at + size > sa->capacity) {
        return NULL; /* not enough room */
    }

    /* The header lives just before the (aligned) payload, not necessarily
     * right at header_at, so place it where the payload can find it. */
    AllocHeader *hdr = (AllocHeader *)(sa->base + payload_at - sizeof(AllocHeader));
    hdr->prev_top = sa->top; /* remember where to rewind on free */

    sa->top = payload_at + size; /* advance the stack top */
    return sa->base + payload_at;
}

/*
 * Pop the most recent allocation, restoring the stack top recorded in its
 * header. Passing NULL is a no-op. Behavior is undefined if `ptr` is not the
 * single most recently pushed allocation, since the stack discipline relies
 * on strict LIFO order. O(1).
 */
void stack_pop(StackAllocator *sa, void *ptr) {
    if (ptr == NULL) {
        return;
    }

    /* The header sits immediately before the payload pointer. */
    AllocHeader *hdr = (AllocHeader *)((uint8_t *)ptr - sizeof(AllocHeader));
    sa->top = hdr->prev_top; /* unwind to before this allocation */
}

/*
 * Capture the current stack top as a marker for bulk rollback.
 * Returns an opaque offset to pass later to stack_free_to.
 */
size_t stack_get_marker(const StackAllocator *sa) {
    return sa->top;
}

/*
 * Free every allocation made after `marker` by resetting the top to it.
 * The marker must come from stack_get_marker and not exceed the current top;
 * out-of-range markers are ignored to avoid corrupting the stack. O(1).
 */
void stack_free_to(StackAllocator *sa, size_t marker) {
    if (marker <= sa->top) {
        sa->top = marker;
    }
}

/*
 * Report the number of bytes currently in use on the stack.
 * Returns the top offset, which equals header plus payload bytes consumed.
 */
size_t stack_bytes_used(const StackAllocator *sa) {
    return sa->top;
}
