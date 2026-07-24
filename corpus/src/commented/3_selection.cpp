/*
 * selection.cpp
 *
 * Order-statistic selection over arrays of integers: a classic O(n^2)
 * selection sort, plus an average-O(n) quickselect that finds the k-th
 * smallest element without fully sorting the array. Both work in place.
 */

#include <cstdint>

/*
 * A lightweight view over an integer array. Bundling the pointer and length
 * keeps the selection helpers from having to thread a length through every
 * call, and makes the intent ("a span of values") explicit.
 */
class IntSpan {
public:
    int32_t *values; /* elements; the span does not own this storage */
    int      length; /* number of elements addressable from values   */

    IntSpan(int32_t *v, int n) : values(v), length(n) {}

    /* Swap the elements at indices i and j. O(1). */
    void swap(int i, int j) {
        int32_t held = values[i];
        values[i] = values[j];
        values[j] = held;
    }
};

/*
 * Selection sort: order the span ascending by repeatedly finding the minimum
 * of the unsorted suffix and swapping it into place. Useful when writes are
 * expensive, since it performs at most n-1 swaps.
 *
 * Parameters:
 *   span - the array view to sort in place.
 *
 * Returns: nothing. Time O(n^2), exactly n-1 swaps, O(1) extra space.
 */
void selectionSort(IntSpan span) {
    for (int i = 0; i < span.length - 1; i++) {
        int minIndex = i; /* index of the smallest value seen in the suffix */

        for (int j = i + 1; j < span.length; j++) {
            if (span.values[j] < span.values[minIndex]) {
                minIndex = j;
            }
        }

        /* Only touch memory if a smaller element was actually found. */
        if (minIndex != i) {
            span.swap(i, minIndex);
        }
    }
}

/*
 * Lomuto partition around the value at index `pivotIndex`, restricted to the
 * inclusive range [left, right]. Elements smaller than the pivot are moved
 * ahead of it; the pivot ends up at its final sorted position.
 *
 * Parameters:
 *   span       - the array view being partitioned.
 *   left       - inclusive start of the range.
 *   right      - inclusive end of the range.
 *   pivotIndex - index of the chosen pivot within [left, right].
 *
 * Returns: the final index of the pivot value.
 */
static int partitionAround(IntSpan span, int left, int right, int pivotIndex) {
    int32_t pivot = span.values[pivotIndex];

    /* Stash the pivot at the end so the scan has a clean range to work over. */
    span.swap(pivotIndex, right);

    int store = left; /* next slot for an element smaller than the pivot */
    for (int i = left; i < right; i++) {
        if (span.values[i] < pivot) {
            span.swap(store, i);
            store++;
        }
    }

    /* Move the pivot from the end into its sorted slot. */
    span.swap(store, right);
    return store;
}

/*
 * Quickselect: return the value that would occupy index k if the span were
 * sorted ascending (the (k+1)-th smallest). The array is partially reordered
 * in place but not fully sorted.
 *
 * Parameters:
 *   span - the array view to search; reordered as a side effect.
 *   k    - zero-based rank of the element to find; must be in [0, length).
 *
 * Returns: the k-th smallest value, or 0 if k is out of range. Average time
 *          O(n), worst case O(n^2) on adversarial pivots.
 */
int32_t quickselect(IntSpan span, int k) {
    if (k < 0 || k >= span.length) {
        return 0; /* rank out of range: no meaningful answer */
    }

    int left = 0;
    int right = span.length - 1;

    while (left < right) {
        /* Use the midpoint as a cheap, reasonable pivot choice. */
        int pivotIndex = left + (right - left) / 2;
        int pivotRank = partitionAround(span, left, right, pivotIndex);

        if (pivotRank == k) {
            break;              /* pivot landed exactly on the target rank */
        } else if (k < pivotRank) {
            right = pivotRank - 1; /* target is in the left partition  */
        } else {
            left = pivotRank + 1;  /* target is in the right partition */
        }
    }

    return span.values[k];
}
