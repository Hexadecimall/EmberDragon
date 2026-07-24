/*
 * quicksort.c — In-place quicksort over a 32-bit integer array.
 *
 * Implements the classic divide-and-conquer quicksort using Lomuto
 * partitioning with a median-of-three pivot to avoid the O(n^2) worst
 * case on already-sorted or reverse-sorted input. Sorting is performed
 * in place with no auxiliary array.
 */

#include <stdint.h>

/*
 * swapElements — Exchange the values stored at two array slots.
 *
 * @param array  Pointer to the integer array.
 * @param i      Index of the first element.
 * @param j      Index of the second element.
 *
 * A no-op when i == j. Runs in O(1).
 */
static void swapElements(int32_t *array, int i, int j) {
    if (i == j) {
        return; /* Avoid a pointless self-assignment. */
    }
    int32_t temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

/*
 * medianOfThree — Choose a robust pivot index for the range [low, high].
 *
 * Inspects the first, middle, and last elements and orders them so that
 * the median of the three ends up at the middle index, which is then
 * returned as the pivot. This guards against the degenerate inputs that
 * push naive quicksort toward quadratic time.
 *
 * @param array  Pointer to the integer array.
 * @param low    Inclusive lower bound of the range.
 * @param high   Inclusive upper bound of the range.
 * @return       Index of the chosen pivot element.
 */
static int medianOfThree(int32_t *array, int low, int high) {
    int mid = low + (high - low) / 2; /* Overflow-safe midpoint. */

    /* Sort the trio (array[low], array[mid], array[high]) ascending. */
    if (array[mid] < array[low]) {
        swapElements(array, low, mid);
    }
    if (array[high] < array[low]) {
        swapElements(array, low, high);
    }
    if (array[high] < array[mid]) {
        swapElements(array, mid, high);
    }
    return mid; /* The median now sits in the middle slot. */
}

/*
 * partitionRange — Lomuto partition around a median-of-three pivot.
 *
 * Moves all elements less than the pivot to its left and all greater
 * elements to its right, then returns the pivot's final resting index.
 *
 * @param array  Pointer to the integer array.
 * @param low    Inclusive lower bound of the range.
 * @param high   Inclusive upper bound of the range.
 * @return       Final index of the pivot after partitioning.
 */
static int partitionRange(int32_t *array, int low, int high) {
    int pivotIndex = medianOfThree(array, low, high);

    /* Park the pivot at the end so the scan below has a clean range. */
    swapElements(array, pivotIndex, high);
    int32_t pivotValue = array[high];

    int boundary = low; /* Invariant: array[low..boundary) < pivotValue. */
    for (int scan = low; scan < high; scan++) {
        if (array[scan] < pivotValue) {
            swapElements(array, boundary, scan);
            boundary++;
        }
    }

    /* Drop the pivot into the gap between the two partitions. */
    swapElements(array, boundary, high);
    return boundary;
}

/*
 * quicksortRange — Recursively sort the inclusive range [low, high].
 *
 * Average complexity O(n log n); worst case O(n^2) is made unlikely by
 * the median-of-three pivot. Recursion depth stays manageable because
 * we always recurse, but real callers should consider tail-call style.
 *
 * @param array  Pointer to the integer array.
 * @param low    Inclusive lower bound of the range.
 * @param high   Inclusive upper bound of the range.
 */
void quicksortRange(int32_t *array, int low, int high) {
    if (low >= high) {
        return; /* Zero- or one-element ranges are already sorted. */
    }
    int split = partitionRange(array, low, high);
    quicksortRange(array, low, split - 1);
    quicksortRange(array, split + 1, high);
}

/*
 * quicksort — Convenience entry point sorting an entire array ascending.
 *
 * @param array  Pointer to the integer array (may be NULL if count == 0).
 * @param count  Number of elements in the array.
 */
void quicksort(int32_t *array, int count) {
    if (array == 0 || count < 2) {
        return; /* Nothing to do for empty or single-element arrays. */
    }
    quicksortRange(array, 0, count - 1);
}
