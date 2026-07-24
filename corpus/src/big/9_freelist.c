/*
 * Embedded free-list allocator over a fixed byte region.
 *
 * Manages a single contiguous heap as a singly linked list of free blocks.
 * Each block carries an inline header recording its size, so adjacent free
 * blocks can be coalesced on release. Uses a first-fit search with splitting,
 * which keeps the implementation small while avoiding most fragmentation.
 */

#include <stdint.h>
#include <stddef.h>

/* Header prepended to every block, free or allocated. `size` covers the
 * payload only (not the header). `next` links free blocks; it is unused while
 * a block is allocated. */
typedef struct BlockHeader {
    size_t size;
    struct BlockHeader *next;
} BlockHeader;

/* Manages one region of memory and the head of its free list. */
typedef struct FreeListAllocator {
    BlockHeader *free_head; /* lowest-address free block, or NULL when full */
    uint8_t     *region;    /* start of the managed bytes */
    size_t       total;     /* size of the managed region */
} FreeListAllocator;

/* Smallest payload a split remainder must have to be worth keeping as its own
 * free block; smaller leftovers are absorbed into the allocation. */
#define MIN_SPLIT_PAYLOAD 16

/*
 * Initialize the allocator to manage `total` bytes starting at `region`.
 * Lays down one big free block spanning the whole region. The region must be
 * at least sizeof(BlockHeader) bytes and remains owned by the caller.
 */
void freelist_init(FreeListAllocator *fl, void *region, size_t total) {
    BlockHeader *first = (BlockHeader *)region;
    first->size = total - sizeof(BlockHeader); /* reserve room for the header */
    first->next = NULL;

    fl->free_head = first;
    fl->region = (uint8_t *)region;
    fl->total = total;
}

/*
 * Allocate `size` payload bytes using first-fit. Returns a pointer to the
 * payload, or NULL if no free block is large enough. Splits the chosen block
 * when the leftover is large enough to be useful. O(n) in the free-list length.
 */
void *freelist_alloc(FreeListAllocator *fl, size_t size) {
    BlockHeader *prev = NULL;
    BlockHeader *curr = fl->free_head;

    /* Walk the free list looking for the first block that fits. */
    while (curr != NULL) {
        if (curr->size >= size) {
            /* Decide whether the remainder is big enough to split off. */
            size_t leftover = curr->size - size;
            if (leftover >= sizeof(BlockHeader) + MIN_SPLIT_PAYLOAD) {
                /* Carve a new free block out of the tail of `curr`. */
                uint8_t *raw = (uint8_t *)(curr + 1) + size;
                BlockHeader *split = (BlockHeader *)raw;
                split->size = leftover - sizeof(BlockHeader);
                split->next = curr->next;

                curr->size = size;        /* shrink the allocated block */
                curr->next = split;       /* link split in place of curr */
            }

            /* Unlink the chosen block from the free list. */
            if (prev == NULL) {
                fl->free_head = curr->next;
            } else {
                prev->next = curr->next;
            }

            return (void *)(curr + 1); /* payload sits right after the header */
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL; /* no block large enough */
}

/*
 * Return a block to the free list, keeping the list sorted by address so that
 * physically adjacent free blocks can be merged. `ptr` must be a pointer
 * previously returned by freelist_alloc, or NULL (which is a no-op).
 */
void freelist_free(FreeListAllocator *fl, void *ptr) {
    if (ptr == NULL) {
        return;
    }
    BlockHeader *block = (BlockHeader *)ptr - 1; /* recover the header */

    /* Find the insertion point that preserves ascending address order. */
    BlockHeader *prev = NULL;
    BlockHeader *curr = fl->free_head;
    while (curr != NULL && curr < block) {
        prev = curr;
        curr = curr->next;
    }

    /* Splice the freed block into the ordered list. */
    block->next = curr;
    if (prev == NULL) {
        fl->free_head = block;
    } else {
        prev->next = block;
    }

    /* Coalesce with the following block if they are physically contiguous. */
    if (curr != NULL &&
        (uint8_t *)(block + 1) + block->size == (uint8_t *)curr) {
        block->size += sizeof(BlockHeader) + curr->size;
        block->next = curr->next;
    }

    /* Coalesce with the preceding block under the same adjacency test. */
    if (prev != NULL &&
        (uint8_t *)(prev + 1) + prev->size == (uint8_t *)block) {
        prev->size += sizeof(BlockHeader) + block->size;
        prev->next = block->next;
    }
}

/*
 * Sum the payload bytes currently free. Walks the free list, so it is O(n) and
 * intended for diagnostics rather than hot-path use.
 */
size_t freelist_free_bytes(const FreeListAllocator *fl) {
    size_t sum = 0;
    for (const BlockHeader *b = fl->free_head; b != NULL; b = b->next) {
        sum += b->size;
    }
    return sum;
}
