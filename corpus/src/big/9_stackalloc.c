/*
 * LIFO stack allocator with markers.
 *
 * A bump allocator that additionally supports freeing in last-in/first-out
 * order. Each allocation stores a small header recording the previous top, so
 * the most recent allocation can be popped, or the whole stack can be rewound
 * to a saved marker. This suits scoped scratch memory: take a marker on scope
 * entry, allocate freely, and rewind on exit.
 */

#include <stdint.h>
#include <stdlib.h>

/* Per-allocation header placed just before each payload. Records where the
 * allocation began so that a pop can restore the cursor exactly, even across
 * alignment padding. */
typedef struct AllocHeader {
    size_t prev_offset; /* cursor value before this allocation started */
} AllocHeader;

/* The stack itself: a buffer and a single growing/shrinking cursor. */
typedef struct StackAllocator {
    uint8_t *buffer;   /* owned backing store */
    size_t   capacity; /* total bytes */
    size_t   offset;   /* current top of stack */
} StackAllocator;

/* Opaque savepoint used to rewind the stack to an earlier state. */
typedef size_t StackMarker;

/*
 * Initialize the stack over a malloc'd buffer of `capacity` bytes. Returns 0
 * on success or -1 on allocation failure. Caller must call stack_destroy.
 */
int stack_init(StackAllocator *stack, size_t capacity) {
    stack->buffer = (uint8_t *)malloc(capacity);
    if (stack->buffer == NULL) {
        return -1;
    }
    stack->capacity = capacity;
    stack->offset = 0;
    return 0;
}

/* Round `value` up to a multiple of `align` (a power of two). */
static size_t round_up(size_t value, size_t align) {
    return (value + (align - 1)) & ~(align - 1);
}

/*
 * Push `size` aligned bytes onto the stack. Returns a pointer to the payload,
 * or NULL if the request does not fit. The allocation can later be removed by
 * stack_pop (LIFO) or by rewinding to a marker taken beforehand.
 */
void *stack_push(StackAllocator *stack, size_t size, size_t align) {
    /* Remember where we were so pop can restore it after any padding. */
    size_t prev = stack->offset;

    /* Place the header at an aligned address, then the payload after it. */
    size_t header_pos = round_up(stack->offset, sizeof(AllocHeader));
    size_t payload_pos = round_up(header_pos + sizeof(AllocHeader), align);

    if (payload_pos + size > stack->capacity) {
        return NULL; /* would overflow the buffer */
    }

    AllocHeader *header = (AllocHeader *)(stack->buffer + header_pos);
    header->prev_offset = prev; /* link back for the pop */

    stack->offset = payload_pos + size; /* commit the new top */
    return stack->buffer + payload_pos;
}

/*
 * Pop the most recent allocation, restoring the cursor to where it was before
 * that allocation. `ptr` must be the payload returned by the matching
 * stack_push and must be the current top. Returns 0 on success, -1 if `ptr`
 * is not the top of the stack.
 */
int stack_pop(StackAllocator *stack, void *ptr) {
    uint8_t *payload = (uint8_t *)ptr;

    /* The header lives immediately before the payload's aligned start. */
    AllocHeader *header = (AllocHeader *)payload - 1;

    /* Sanity check: the payload must lie within the live region. Reject pops
     * that are out of order, since only the top allocation can be removed. */
    if (payload < stack->buffer || payload >= stack->buffer + stack->offset) {
        return -1;
    }

    stack->offset = header->prev_offset; /* rewind to before this push */
    return 0;
}

/*
 * Capture the current top of the stack as a marker that can be passed to
 * stack_rewind later. O(1); markers are just the cursor value.
 */
StackMarker stack_mark(const StackAllocator *stack) {
    return stack->offset;
}

/*
 * Free everything allocated since `marker` was taken, in one step. Markers
 * must be rewound in reverse order of capture; rewinding to a marker beyond
 * the current top is ignored to avoid resurrecting freed memory.
 */
void stack_rewind(StackAllocator *stack, StackMarker marker) {
    if (marker <= stack->offset) {
        stack->offset = marker;
    }
}

/* Bytes currently available to push, before alignment overhead. */
size_t stack_remaining(const StackAllocator *stack) {
    return stack->capacity - stack->offset;
}

/*
 * Release the backing buffer and reset all fields. Safe to call on a
 * zero-initialized stack. Outstanding pointers become dangling.
 */
void stack_destroy(StackAllocator *stack) {
    free(stack->buffer);
    stack->buffer = NULL;
    stack->capacity = 0;
    stack->offset = 0;
}
