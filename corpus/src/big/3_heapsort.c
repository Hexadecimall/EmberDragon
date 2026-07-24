/*
 * heapsort.c — In-place heapsort using a binary max-heap.
 *
 * Builds a max-heap over the array, then repeatedly swaps the root
 * (the current maximum) to the end and shrinks the heap. Sorting is
 * in place, requires no extra memory, and runs in guaranteed
 * O(n log n) time regardless of input distribution.
 */

#include <stdint.h>

/*
 * swapInts — Swap the contents of two integer cells.
 *
 * @param a  Pointer to the first integer.
 * @param b  Pointer to the second integer.
 */
static void swapInts(int32_t *a, int32_t *b) {
    int32_t temp = *a;
    *a = *b;
    *b = temp;
}

/*
 * siftDown — Restore the max-heap property at a single node.
 *
 * Assumes the subtrees rooted at the children of `root` are already
 * valid max-heaps. The value at `root` is pushed down until both of
 * its children are no larger than it. Uses 0-based indexing where the
 * children of node i are 2i+1 and 2i+2.
 *
 * @param heap      Pointer to the array backing the heap.
 * @param root      Index whose value may violate the heap property.
 * @param heapSize  Number of elements currently part of the heap.
 *
 * Runs in O(log n) where n is heapSize.
 */
static void siftDown(int32_t *heap, int root, int heapSize) {
    while (1) {
        int largest = root;
        int leftChild = 2 * root + 1;
        int rightChild = 2 * root + 2;

        /* Pick the larger of root and its (existing) children. */
        if (leftChild < heapSize && heap[leftChild] > heap[largest]) {
            largest = leftChild;
        }
        if (rightChild < heapSize && heap[rightChild] > heap[largest]) {
            largest = rightChild;
        }

        if (largest == root) {
            break; /* Heap property already holds here; stop sinking. */
        }
        swapInts(&heap[root], &heap[largest]);
        root = largest; /* Continue sifting into the affected subtree. */
    }
}

/*
 * buildMaxHeap — Arrange an arbitrary array into a valid max-heap.
 *
 * Sifts down every internal node from the last parent up to the root.
 * This bottom-up construction is O(n), faster than n successive inserts.
 *
 * @param heap   Pointer to the array.
 * @param count  Number of elements.
 */
static void buildMaxHeap(int32_t *heap, int count) {
    /* The last internal (non-leaf) node is at index count/2 - 1. */
    for (int node = count / 2 - 1; node >= 0; node--) {
        siftDown(heap, node, count);
    }
}

/*
 * heapsort — Sort an array ascending in place using heapsort.
 *
 * Phase 1 builds a max-heap. Phase 2 repeatedly moves the maximum to
 * the end of the unsorted region and re-heapifies the remainder.
 * Overall complexity O(n log n) time, O(1) extra space.
 *
 * @param array  Pointer to the integer array (may be NULL if count == 0).
 * @param count  Number of elements.
 */
void heapsort(int32_t *array, int count) {
    if (array == 0 || count < 2) {
        return; /* Nothing to sort. */
    }

    buildMaxHeap(array, count);

    /* Invariant: array[end+1..count-1] holds the largest, sorted values. */
    for (int end = count - 1; end > 0; end--) {
        /* The root is the current max; move it to its final slot. */
        swapInts(&array[0], &array[end]);
        /* Restore the heap over the shrunken, still-unsorted prefix. */
        siftDown(array, 0, end);
    }
}
