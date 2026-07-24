/*
 * Bitmap block allocator.
 *
 * Manages a fixed array of equal-size blocks, tracking allocation state with a
 * bitmap (one bit per block: 1 = used, 0 = free). Supports allocating a single
 * block or a contiguous run of blocks, which lets it back simple DMA buffers or
 * page-style allocations where contiguity matters. Searches are linear over the
 * bitmap words.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Number of blocks tracked by one bitmap word. */
#define BITS_PER_WORD 32

/* Owns the block storage and the bitmap describing it. */
typedef struct BitmapAllocator {
    uint8_t  *blocks;     /* base of the block storage */
    uint32_t *bitmap;     /* one bit per block; ceil(count/32) words */
    size_t    block_size; /* bytes per block */
    size_t    block_count;/* total number of blocks */
    size_t    word_count; /* number of bitmap words */
} BitmapAllocator;

/* True if block `i` is currently allocated. */
static int bit_is_set(const BitmapAllocator *a, size_t i) {
    return (a->bitmap[i / BITS_PER_WORD] >> (i % BITS_PER_WORD)) & 1u;
}

/* Mark block `i` as allocated. */
static void bit_set(BitmapAllocator *a, size_t i) {
    a->bitmap[i / BITS_PER_WORD] |= (1u << (i % BITS_PER_WORD));
}

/* Mark block `i` as free. */
static void bit_clear(BitmapAllocator *a, size_t i) {
    a->bitmap[i / BITS_PER_WORD] &= ~(1u << (i % BITS_PER_WORD));
}

/*
 * Initialize an allocator over `block_count` blocks of `block_size` bytes.
 * Allocates both the block storage and the bitmap, marking all blocks free.
 * Returns 0 on success or -1 on allocation failure. Caller calls
 * bitmap_destroy to release both buffers.
 */
int bitmap_init(BitmapAllocator *a, size_t block_size, size_t block_count) {
    a->word_count = (block_count + BITS_PER_WORD - 1) / BITS_PER_WORD;

    a->blocks = (uint8_t *)malloc(block_size * block_count);
    if (a->blocks == NULL) {
        return -1;
    }
    a->bitmap = (uint32_t *)calloc(a->word_count, sizeof(uint32_t));
    if (a->bitmap == NULL) {
        free(a->blocks); /* undo the block allocation on partial failure */
        a->blocks = NULL;
        return -1;
    }
    a->block_size = block_size;
    a->block_count = block_count;
    return 0; /* calloc already zeroed the bitmap: all blocks free */
}

/*
 * Allocate a single free block. Returns a pointer to the block, or NULL if
 * none are free. Scans the bitmap linearly, so it is O(block_count) worst case.
 */
void *bitmap_alloc(BitmapAllocator *a) {
    for (size_t i = 0; i < a->block_count; i++) {
        if (!bit_is_set(a, i)) {
            bit_set(a, i);
            return a->blocks + i * a->block_size;
        }
    }
    return NULL; /* fully allocated */
}

/*
 * Allocate `count` physically contiguous free blocks and return a pointer to
 * the first. Returns NULL if no run of that length exists. Uses a sliding scan
 * that restarts past any used block, giving O(block_count) behavior.
 */
void *bitmap_alloc_run(BitmapAllocator *a, size_t count) {
    if (count == 0 || count > a->block_count) {
        return NULL;
    }
    size_t start = 0;
    while (start + count <= a->block_count) {
        /* Extend the run from `start` until a used block or the goal length. */
        size_t run = 0;
        while (run < count && !bit_is_set(a, start + run)) {
            run++;
        }
        if (run == count) {
            /* Found a long-enough gap: claim every block in it. */
            for (size_t k = 0; k < count; k++) {
                bit_set(a, start + k);
            }
            return a->blocks + start * a->block_size;
        }
        /* The block at start+run was used; the next candidate starts after it. */
        start += run + 1;
    }
    return NULL;
}

/*
 * Convert a block pointer back to its index, or return block_count if the
 * pointer is outside the storage or not on a block boundary.
 */
static size_t bitmap_index_of(const BitmapAllocator *a, const void *ptr) {
    const uint8_t *p = (const uint8_t *)ptr;
    if (p < a->blocks) {
        return a->block_count; /* sentinel for "invalid" */
    }
    size_t byte_off = (size_t)(p - a->blocks);
    if (byte_off % a->block_size != 0) {
        return a->block_count; /* not aligned to a block start */
    }
    size_t idx = byte_off / a->block_size;
    return idx < a->block_count ? idx : a->block_count;
}

/*
 * Free a run of `count` blocks beginning at `ptr`. `ptr` must come from a prior
 * bitmap_alloc/bitmap_alloc_run. Returns 0 on success, -1 if the pointer or
 * range is invalid. Passing NULL frees nothing and returns 0.
 */
int bitmap_free_run(BitmapAllocator *a, void *ptr, size_t count) {
    if (ptr == NULL) {
        return 0;
    }
    size_t idx = bitmap_index_of(a, ptr);
    if (idx == a->block_count || idx + count > a->block_count) {
        return -1; /* bad pointer or run extends past the storage */
    }
    for (size_t k = 0; k < count; k++) {
        bit_clear(a, idx + k);
    }
    return 0;
}

/*
 * Count the blocks currently free. Walks all blocks, so O(block_count); meant
 * for diagnostics and high-water-mark tracking.
 */
size_t bitmap_free_count(const BitmapAllocator *a) {
    size_t free = 0;
    for (size_t i = 0; i < a->block_count; i++) {
        if (!bit_is_set(a, i)) {
            free++;
        }
    }
    return free;
}

/*
 * Release both the block storage and the bitmap, clearing all fields. Safe on
 * a zero-initialized allocator.
 */
void bitmap_destroy(BitmapAllocator *a) {
    free(a->blocks);
    free(a->bitmap);
    a->blocks = NULL;
    a->bitmap = NULL;
    a->block_count = 0;
    a->word_count = 0;
}
