/*
 * bucketsort.c — Bucket sort for non-negative integers in a known range.
 *
 * Distributes values across a fixed number of equal-width buckets, sorts
 * each bucket independently with insertion sort, then concatenates the
 * buckets back into the input. Approaches O(n) when the keys are spread
 * roughly uniformly across the value range. Uses singly-linked nodes so a
 * bucket can hold any number of elements without per-bucket reallocation.
 */

#include <stdint.h>
#include <stdlib.h>

#define BUCKET_COUNT 16 /* Number of buckets the value range is split into. */

/*
 * BucketNode — One value living inside a bucket's linked list.
 */
typedef struct BucketNode {
    int32_t value;            /* The stored integer key. */
    struct BucketNode *next;  /* Next node in the same bucket, or NULL. */
} BucketNode;

/*
 * insertSorted — Insert a value into a bucket's ascending linked list.
 *
 * Performs an insertion-sort-style placement: the new node is linked in
 * before the first existing node whose value is larger. Keeping each
 * bucket sorted as we fill it means the final concatenation is trivial.
 *
 * @param head   Address of the bucket's head pointer (updated in place).
 * @param value  The value to insert.
 * @return       0 on success, -1 if a node could not be allocated.
 */
static int insertSorted(BucketNode **head, int32_t value) {
    BucketNode *node = (BucketNode *)malloc(sizeof(BucketNode));
    if (node == 0) {
        return -1; /* Allocation failure bubbles up to the caller. */
    }
    node->value = value;

    /* Walk to the insertion point, keeping a pointer to the link to rewrite. */
    BucketNode **link = head;
    while (*link != 0 && (*link)->value <= value) {
        link = &(*link)->next;
    }
    node->next = *link;
    *link = node;
    return 0;
}

/*
 * freeBucket — Release every node in a bucket's linked list.
 *
 * @param head  Head of the list (may be NULL).
 */
static void freeBucket(BucketNode *head) {
    while (head != 0) {
        BucketNode *next = head->next;
        free(head);
        head = next;
    }
}

/*
 * bucketsort — Sort an array of integers in [0, maxValue] ascending.
 *
 * Each value is mapped to a bucket by scaling it across BUCKET_COUNT,
 * inserted in sorted order, and finally the buckets are read back left
 * to right. Average O(n) for uniform input; worst case O(n^2) if every
 * value collides into one bucket.
 *
 * @param array     Pointer to the integer array. Values must be in
 *                  [0, maxValue]; out-of-range values are clamped.
 * @param count     Number of elements.
 * @param maxValue  The largest value any element may take (>= 0).
 * @return          0 on success, -1 on allocation failure (input may be
 *                  partially processed but all temporary memory is freed).
 */
int bucketsort(int32_t *array, int count, int32_t maxValue) {
    if (array == 0 || count < 2) {
        return 0; /* Nothing to sort. */
    }
    if (maxValue < 0) {
        maxValue = 0; /* Defensive: a negative range makes no sense. */
    }

    BucketNode *buckets[BUCKET_COUNT];
    for (int b = 0; b < BUCKET_COUNT; b++) {
        buckets[b] = 0; /* Start every bucket empty. */
    }

    /* Phase 1: scatter each value into its bucket, keeping buckets sorted. */
    for (int i = 0; i < count; i++) {
        int32_t v = array[i];
        if (v < 0) v = 0;
        if (v > maxValue) v = maxValue;

        /* Map v into [0, BUCKET_COUNT-1]. +1 avoids dividing by zero. */
        int index = (int)(((int64_t)v * BUCKET_COUNT) / (maxValue + 1));
        if (index >= BUCKET_COUNT) {
            index = BUCKET_COUNT - 1; /* Guard the top edge of the range. */
        }

        if (insertSorted(&buckets[index], v) != 0) {
            for (int b = 0; b < BUCKET_COUNT; b++) {
                freeBucket(buckets[b]); /* Clean up before reporting failure. */
            }
            return -1;
        }
    }

    /* Phase 2: gather buckets left to right back into the array. */
    int writePos = 0;
    for (int b = 0; b < BUCKET_COUNT; b++) {
        BucketNode *node = buckets[b];
        while (node != 0) {
            array[writePos++] = node->value;
            node = node->next;
        }
        freeBucket(buckets[b]);
    }
    return 0;
}
