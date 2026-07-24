/*
 * Buddy-system allocator.
 *
 * Splits a power-of-two memory region into blocks whose sizes are also powers
 * of two. Each size class keeps a free list; allocating a small block splits a
 * larger one in half repeatedly, and freeing merges a block with its "buddy"
 * (the other half it was split from) whenever the buddy is also free. This
 * bounds external fragmentation while keeping allocation and free at
 * O(log size).
 */

#include <stdint.h>
#include <stdlib.h>

/* Maximum number of size classes (orders) the allocator supports. Order k
 * corresponds to blocks of size (1 << k). */
#define MAX_ORDERS 32

/* Free-list link stored inside an idle block. */
typedef struct BuddyFreeNode {
    struct BuddyFreeNode *next;
} BuddyFreeNode;

/* A buddy allocator over one region of 2^max_order bytes. */
typedef struct BuddyAllocator {
    uint8_t       *base;                 /* start of the managed region */
    BuddyFreeNode *free_lists[MAX_ORDERS]; /* per-order free lists */
    unsigned       min_order;            /* smallest block = 1 << min_order */
    unsigned       max_order;            /* whole region = 1 << max_order */
} BuddyAllocator;

/* Smallest order whose block size is >= `size`, clamped to min_order. */
static unsigned order_for_size(const BuddyAllocator *b, size_t size) {
    unsigned order = b->min_order;
    while ((((size_t)1) << order) < size) {
        order++;
    }
    return order;
}

/* Push a block onto the free list for `order`. */
static void push_free(BuddyAllocator *b, unsigned order, void *block) {
    BuddyFreeNode *node = (BuddyFreeNode *)block;
    node->next = b->free_lists[order];
    b->free_lists[order] = node;
}

/* Pop a block from the free list for `order`, or NULL if it is empty. */
static void *pop_free(BuddyAllocator *b, unsigned order) {
    BuddyFreeNode *node = b->free_lists[order];
    if (node != NULL) {
        b->free_lists[order] = node->next;
    }
    return node;
}

/* Remove a specific block from `order`'s free list. Returns 1 if it was found
 * and unlinked, 0 if it was not present (i.e. the buddy is in use). */
static int remove_free(BuddyAllocator *b, unsigned order, void *block) {
    BuddyFreeNode **link = &b->free_lists[order];
    while (*link != NULL) {
        if (*link == (BuddyFreeNode *)block) {
            *link = (*link)->next; /* unlink */
            return 1;
        }
        link = &(*link)->next;
    }
    return 0;
}

/*
 * Initialize the allocator over a region of (1 << max_order) bytes, with the
 * smallest allocatable block being (1 << min_order) bytes. Returns 0 on
 * success, or -1 on bad orders or allocation failure. Caller calls
 * buddy_destroy to release the region.
 */
int buddy_init(BuddyAllocator *b, unsigned min_order, unsigned max_order) {
    if (max_order >= MAX_ORDERS || min_order > max_order) {
        return -1; /* orders out of range */
    }
    b->base = (uint8_t *)malloc(((size_t)1) << max_order);
    if (b->base == NULL) {
        return -1;
    }
    for (unsigned i = 0; i < MAX_ORDERS; i++) {
        b->free_lists[i] = NULL;
    }
    b->min_order = min_order;
    b->max_order = max_order;

    /* Start with one free block covering the entire region. */
    push_free(b, max_order, b->base);
    return 0;
}

/*
 * Allocate at least `size` bytes, rounded up to a power-of-two block. Returns a
 * pointer to the block, or NULL if no large-enough free block can be formed.
 * Splits larger blocks as needed; O(max_order - min_order).
 */
void *buddy_alloc(BuddyAllocator *b, size_t size) {
    if (size == 0) {
        return NULL;
    }
    unsigned need = order_for_size(b, size);
    if (need > b->max_order) {
        return NULL; /* larger than the whole region */
    }

    /* Find the smallest available order >= need. */
    unsigned order = need;
    while (order <= b->max_order && b->free_lists[order] == NULL) {
        order++;
    }
    if (order > b->max_order) {
        return NULL; /* nothing free big enough */
    }

    void *block = pop_free(b, order);

    /* Split down to the requested order, returning each unused upper half to
     * its free list. The block stays at `block`; the buddy is the upper half. */
    while (order > need) {
        order--;
        uint8_t *buddy = (uint8_t *)block + (((size_t)1) << order);
        push_free(b, order, buddy);
    }
    return block;
}

/*
 * Free a block previously returned by buddy_alloc. `size` must be the same
 * size requested for that allocation so the order can be recomputed. Merges
 * with free buddies up the tree. NULL is a no-op. O(max_order - min_order).
 */
void buddy_free(BuddyAllocator *b, void *ptr, size_t size) {
    if (ptr == NULL) {
        return;
    }
    unsigned order = order_for_size(b, size);

    /* Repeatedly try to coalesce with the buddy at the current order. */
    while (order < b->max_order) {
        size_t block_size = ((size_t)1) << order;
        /* The buddy address is found by flipping the order-th bit of the
         * block's offset from base, the defining trick of the buddy system. */
        size_t offset = (size_t)((uint8_t *)ptr - b->base);
        size_t buddy_offset = offset ^ block_size;
        void *buddy = b->base + buddy_offset;

        if (!remove_free(b, order, buddy)) {
            break; /* buddy not free: stop merging */
        }
        /* Merged block starts at the lower of the two addresses. */
        if (buddy < ptr) {
            ptr = buddy;
        }
        order++;
    }
    push_free(b, order, ptr); /* insert the (possibly merged) block */
}

/*
 * Release the managed region and clear the allocator. Safe on a
 * zero-initialized allocator; outstanding pointers become dangling.
 */
void buddy_destroy(BuddyAllocator *b) {
    free(b->base);
    b->base = NULL;
    for (unsigned i = 0; i < MAX_ORDERS; i++) {
        b->free_lists[i] = NULL;
    }
}
