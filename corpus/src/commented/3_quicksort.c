/*
 * quicksort.c
 *
 * An in-place quicksort over arrays of 32-bit signed integers. Partitioning
 * uses the Lomuto scheme with a median-of-three pivot to reduce the chance of
 * worst-case behavior on already-sorted or reverse-sorted input.
 */

#include <stdint.h>

/*
 * Exchange the values stored at two integer locations.
 *
 * Parameters:
 *   a, b - non-NULL pointers to the elements to swap.
 *
 * Returns: nothing. Mutates the two pointed-to values in place.
 */
static void swapInts(int32_t *a, int32_t *b) {
    int32_t held = *a;
    *a = *b;
    *b = held;
}

/*
 * Choose a pivot using the median of the first, middle, and last elements of
 * the range, then move that median to the final slot so the Lomuto partition
 * can use it. This guards against the O(n^2) blowup on sorted input.
 *
 * Parameters:
 *   data - array being sorted.
 *   low  - inclusive index of the range start.
 *   high - inclusive index of the range end.
 *
 * Returns: the chosen pivot value (also now parked at data[high]).
 */
static int32_t medianOfThree(int32_t *data, int low, int high) {
    int mid = low + (high - low) / 2; /* avoids overflow vs (low+high)/2 */

    /* Sort the three sample positions relative to each other. */
    if (data[mid] < data[low])  swapInts(&data[mid], &data[low]);
    if (data[high] < data[low]) swapInts(&data[high], &data[low]);
    if (data[high] < data[mid]) swapInts(&data[high], &data[mid]);

    /* The median now sits at mid; park it at high for partitioning. */
    swapInts(&data[mid], &data[high]);
    return data[high];
}

/*
 * Lomuto partition: rearrange data[low..high] so every element less than the
 * pivot precedes it and every element greater follows it.
 *
 * Parameters:
 *   data - array being sorted.
 *   low  - inclusive index of the range start.
 *   high - inclusive index of the range end (pivot lives here on entry).
 *
 * Returns: the final resting index of the pivot.
 */
static int partition(int32_t *data, int low, int high) {
    int32_t pivot = medianOfThree(data, low, high);
    int boundary = low - 1; /* last index known to hold a value < pivot */

    for (int scan = low; scan < high; scan++) {
        if (data[scan] < pivot) {
            boundary++;
            swapInts(&data[boundary], &data[scan]);
        }
    }

    /* Drop the pivot just past the "less than" region. */
    swapInts(&data[boundary + 1], &data[high]);
    return boundary + 1;
}

/*
 * Recursively sort the inclusive range data[low..high] in ascending order.
 *
 * Parameters:
 *   data - array being sorted.
 *   low  - inclusive index of the range start.
 *   high - inclusive index of the range end.
 *
 * Returns: nothing. Average complexity O(n log n), worst case O(n^2).
 */
void quicksortRange(int32_t *data, int low, int high) {
    while (low < high) {
        int split = partition(data, low, high);

        /* Recurse into the smaller side, loop on the larger side, to keep
         * stack depth bounded at O(log n) even on skewed partitions. */
        if (split - low < high - split) {
            quicksortRange(data, low, split - 1);
            low = split + 1;
        } else {
            quicksortRange(data, split + 1, high);
            high = split - 1;
        }
    }
}

/*
 * Convenience entry point that sorts an entire array.
 *
 * Parameters:
 *   data  - array to sort in place.
 *   count - number of elements; values < 2 are already sorted.
 *
 * Returns: nothing.
 */
void quicksort(int32_t *data, int count) {
    if (count > 1) {
        quicksortRange(data, 0, count - 1);
    }
}
