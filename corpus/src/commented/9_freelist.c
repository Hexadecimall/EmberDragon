/*
 * freelist.c — A first-fit free-list heap over a static byte pool.
 *
 * Memory is carved from a single static buffer organized as a singly linked
 * list of blocks. Allocation walks the list looking for the first free block
 * large enough to satisfy the request, splitting it when there is leftover
 * space. Freeing marks a block available and coalesces it with the following
 * block when they are adjacent and both free, limiting fragmentation.
 */

#include <stdint.h>
#include <stddef.h>

#define HEAP_CAPACITY 4096

/*
 * Each allocation is prefixed by a header describing the payload that follows.
 * Blocks are laid out back-to-back, so the next block always begins at
 * (this header + sizeof(BlockHeader) + size).
 */
typedef struct BlockHeader {
    size_t size;              /* payload size in bytes, excluding this header */
    int    free;              /* 1 if the block is available, 0 if in use     */
    struct BlockHeader *next; /* next block in address order, or NULL at end  */
} BlockHeader;

/* The whole heap lives in this static buffer; no OS allocation is involved. */
static uint8_t heap_storage[HEAP_CAPACITY];
static BlockHeader *heap_head = NULL;

/*
 * Prepare the heap as a single large free block spanning the whole buffer.
 * Must be called once before any allocation. Idempotent re-initialization
 * discards all prior allocations.
 */
void heap_init(void) {
    heap_head = (BlockHeader *)heap_storage;
    heap_head->size = HEAP_CAPACITY - sizeof(BlockHeader);
    heap_head->free = 1;
    heap_head->next = NULL;
}

/*
 * Split `block` so it holds exactly `size` payload bytes, carving the
 * remainder into a new trailing free block. Does nothing if the leftover
 * is too small to hold a header plus at least one usable byte.
 */
static void split_block(BlockHeader *block, size_t size) {
    size_t leftover = block->size - size;

    /* Only split when the tail can host a header and remain useful. */
    if (leftover > sizeof(BlockHeader)) {
        BlockHeader *tail =
            (BlockHeader *)((uint8_t *)block + sizeof(BlockHeader) + size);
        tail->size = leftover - sizeof(BlockHeader);
        tail->free = 1;
        tail->next = block->next;

        block->size = size;
        block->next = tail;
    }
}

/*
 * Allocate `size` payload bytes using first-fit search.
 * Returns a pointer to usable memory, or NULL if no block is large enough.
 * The search is O(n) in the number of blocks. The returned region is
 * uninitialized.
 */
void *heap_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    for (BlockHeader *cur = heap_head; cur != NULL; cur = cur->next) {
        /* Take the first free block that can hold the request. */
        if (cur->free && cur->size >= size) {
            split_block(cur, size);
            cur->free = 0;
            /* Payload begins immediately after the header. */
            return (uint8_t *)cur + sizeof(BlockHeader);
        }
    }
    return NULL; /* heap exhausted or too fragmented */
}

/*
 * Return a block to the free list, coalescing with the immediately following
 * block when both are free and physically adjacent. Passing NULL is a no-op.
 * Coalescing keeps the list from accumulating tiny unusable fragments.
 */
void heap_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    /* Recover the header that sits just before the payload. */
    BlockHeader *block = (BlockHeader *)((uint8_t *)ptr - sizeof(BlockHeader));
    block->free = 1;

    BlockHeader *next = block->next;
    if (next != NULL && next->free) {
        /* Verify the next header sits exactly where this block ends. */
        uint8_t *expected = (uint8_t *)block + sizeof(BlockHeader) + block->size;
        if ((uint8_t *)next == expected) {
            block->size += sizeof(BlockHeader) + next->size;
            block->next = next->next; /* absorb the neighbor */
        }
    }
}

/*
 * Count the bytes currently available across all free blocks.
 * Returns the summed payload capacity of free blocks; this may be larger
 * than the biggest single allocatable region due to fragmentation. O(n).
 */
size_t heap_free_bytes(void) {
    size_t total = 0;
    for (BlockHeader *cur = heap_head; cur != NULL; cur = cur->next) {
        if (cur->free) {
            total += cur->size;
        }
    }
    return total;
}
