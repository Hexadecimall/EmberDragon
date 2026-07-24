/*
 * heapsort.c
 *
 * In-place heapsort for arrays of unsigned 32-bit integers. The array is first
 * arranged into a binary max-heap, then the maximum is repeatedly swapped to
 * the end and the heap shrunk, producing an ascending order with no extra
 * memory and a guaranteed O(n log n) worst case.
 */

#include <stdint.h>

/*
 * Swap two unsigned integers in place.
 *
 * Parameters:
 *   a, b - non-NULL pointers to the values to exchange.
 *
 * Returns: nothing.
 */
static void swapU32(uint32_t *a, uint32_t *b) {
    uint32_t held = *a;
    *a = *b;
    *b = held;
}

/*
 * Restore the max-heap property for the subtree rooted at index `root`,
 * assuming both child subtrees are already valid heaps. The offending root
 * value is "sifted down" until it is no smaller than both of its children.
 *
 * Parameters:
 *   heap - array interpreted as a complete binary tree (index 0 is the root).
 *   size - number of elements currently part of the heap.
 *   root - index of the subtree root to repair.
 *
 * Returns: nothing. Runs in O(log size).
 */
static void siftDown(uint32_t *heap, int size, int root) {
    for (;;) {
        int largest = root;
        int left  = 2 * root + 1; /* left child index  */
        int right = 2 * root + 2; /* right child index */

        /* Find the largest of the root and its in-range children. */
        if (left < size && heap[left] > heap[largest]) {
            largest = left;
        }
        if (right < size && heap[right] > heap[largest]) {
            largest = right;
        }

        if (largest == root) {
            break; /* heap property holds; nothing left to do */
        }

        /* Move the larger child up and continue sifting from its old slot. */
        swapU32(&heap[root], &heap[largest]);
        root = largest;
    }
}

/*
 * Transform an arbitrary array into a max-heap bottom-up. Starting from the
 * last internal node and working toward the root lets each siftDown rely on
 * its children already being valid heaps.
 *
 * Parameters:
 *   heap - array to heapify in place.
 *   size - number of elements.
 *
 * Returns: nothing. Runs in O(size), tighter than n calls to siftDown.
 */
static void buildMaxHeap(uint32_t *heap, int size) {
    /* The last internal node is the parent of the final element. */
    for (int root = size / 2 - 1; root >= 0; root--) {
        siftDown(heap, size, root);
    }
}

/*
 * Sort an array of unsigned integers into ascending order in place.
 *
 * Parameters:
 *   data  - array to sort.
 *   count - number of elements; values < 2 are already sorted.
 *
 * Returns: nothing. Time O(n log n) worst case, O(1) extra space.
 */
void heapsort(uint32_t *data, int count) {
    if (count < 2) {
        return;
    }

    buildMaxHeap(data, count);

    /* Repeatedly extract the maximum (at index 0) to the end of the active
     * region, then restore the heap over the now-smaller prefix. */
    for (int end = count - 1; end > 0; end--) {
        swapU32(&data[0], &data[end]);
        siftDown(data, end, 0);
    }
}
