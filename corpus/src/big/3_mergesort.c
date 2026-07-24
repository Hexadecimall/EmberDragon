/*
 * mergesort.c — Stable, top-down merge sort for 32-bit integers.
 *
 * Sorts an array ascending using classic divide-and-conquer merge sort.
 * The merge step is stable (equal keys keep their original order), which
 * makes this suitable for sorting records by a secondary key first.
 * A single scratch buffer is allocated once and reused across all merges.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * mergeHalves — Merge two adjacent sorted runs into one sorted run.
 *
 * The left run is array[left..mid] and the right run is array[mid+1..right],
 * each already sorted. Results are assembled in `scratch` and copied back.
 * Stability is preserved by taking from the left run on ties.
 *
 * @param array    Pointer to the integer array being sorted.
 * @param scratch  Temporary buffer at least `right - left + 1` long.
 * @param left     Inclusive start of the left run.
 * @param mid      Inclusive end of the left run.
 * @param right    Inclusive end of the right run.
 */
static void mergeHalves(int32_t *array, int32_t *scratch,
                        int left, int mid, int right) {
    int i = left;      /* Read cursor into the left run.  */
    int j = mid + 1;   /* Read cursor into the right run. */
    int k = 0;         /* Write cursor into the scratch buffer. */

    /* Drain whichever run currently exposes the smaller head element. */
    while (i <= mid && j <= right) {
        if (array[i] <= array[j]) {
            /* "<=" (not "<") keeps equal keys in original order: stable. */
            scratch[k++] = array[i++];
        } else {
            scratch[k++] = array[j++];
        }
    }

    /* At most one of these tails still has elements left to copy. */
    while (i <= mid) {
        scratch[k++] = array[i++];
    }
    while (j <= right) {
        scratch[k++] = array[j++];
    }

    /* Copy the merged run back over the original slice. */
    for (int t = 0; t < k; t++) {
        array[left + t] = scratch[t];
    }
}

/*
 * mergesortRange — Recursively sort the inclusive range [left, right].
 *
 * Splits the range in half, sorts each half, then merges. Runs in
 * O(n log n) time and O(n) auxiliary space for the shared scratch buffer.
 *
 * @param array    Pointer to the integer array.
 * @param scratch  Shared temporary buffer (size >= array length).
 * @param left     Inclusive lower bound.
 * @param right    Inclusive upper bound.
 */
static void mergesortRange(int32_t *array, int32_t *scratch,
                           int left, int right) {
    if (left >= right) {
        return; /* A run of length <= 1 is trivially sorted. */
    }
    int mid = left + (right - left) / 2; /* Overflow-safe split point. */
    mergesortRange(array, scratch, left, mid);
    mergesortRange(array, scratch, mid + 1, right);

    /* Skip the merge when the two runs are already in order. */
    if (array[mid] <= array[mid + 1]) {
        return;
    }
    mergeHalves(array, scratch, left, mid, right);
}

/*
 * mergeSortArray — Sort an entire array ascending in stable O(n log n) time.
 *
 * @param array  Pointer to the integer array.
 * @param count  Number of elements.
 * @return       0 on success, -1 if the scratch buffer could not be
 *               allocated (the array is left untouched in that case).
 */
int mergeSortArray(int32_t *array, int count) {
    if (array == 0 || count < 2) {
        return 0; /* Empty or single-element arrays need no work. */
    }
    int32_t *scratch = (int32_t *)malloc((size_t)count * sizeof(int32_t));
    if (scratch == 0) {
        return -1; /* Out of memory; report failure without mutating input. */
    }
    mergesortRange(array, scratch, 0, count - 1);
    free(scratch);
    return 0;
}
