/*
 * mergesort.c
 *
 * A stable, top-down merge sort for arrays of long integers. Sorting is
 * performed out of place using a single scratch buffer that the caller's
 * memory comes from malloc; the public entry point allocates and frees it.
 */

#include <stdlib.h>

/*
 * Merge two adjacent sorted runs, src[left..mid] and src[mid+1..right], into
 * dst[left..right] in ascending order. Stability is preserved by taking from
 * the left run whenever the two front elements compare equal.
 *
 * Parameters:
 *   src   - source array holding the two sorted runs.
 *   dst   - destination array receiving the merged run.
 *   left  - inclusive start of the first run.
 *   mid   - inclusive end of the first run; second run starts at mid + 1.
 *   right - inclusive end of the second run.
 *
 * Returns: nothing. Runs in O(right - left) time.
 */
static void mergeRuns(const long *src, long *dst, int left, int mid, int right) {
    int i = left;      /* cursor into the left run  */
    int j = mid + 1;   /* cursor into the right run */
    int k = left;      /* write cursor into dst     */

    while (i <= mid && j <= right) {
        /* '<=' keeps equal elements in their original order (stable). */
        if (src[i] <= src[j]) {
            dst[k++] = src[i++];
        } else {
            dst[k++] = src[j++];
        }
    }

    /* At most one of these tails still has elements to copy. */
    while (i <= mid)   dst[k++] = src[i++];
    while (j <= right) dst[k++] = src[j++];
}

/*
 * Recursively sort buffer[left..right], using scratch as temporary space.
 *
 * Parameters:
 *   buffer  - array to sort in place by the time this returns.
 *   scratch - same-length working buffer; contents are clobbered.
 *   left    - inclusive start index.
 *   right   - inclusive end index.
 *
 * Returns: nothing. Time O(n log n), extra space O(n).
 */
static void sortSpan(long *buffer, long *scratch, int left, int right) {
    if (left >= right) {
        return; /* zero or one element: already sorted */
    }

    int mid = left + (right - left) / 2;

    /* Sort each half independently. */
    sortSpan(buffer, scratch, left, mid);
    sortSpan(buffer, scratch, mid + 1, right);

    /* Merge the two sorted halves into scratch, then copy back. */
    mergeRuns(buffer, scratch, left, mid, right);
    for (int i = left; i <= right; i++) {
        buffer[i] = scratch[i];
    }
}

/*
 * Sort an entire array of longs into ascending order, stably.
 *
 * Parameters:
 *   buffer - array to sort in place.
 *   count  - number of elements; values < 2 leave the array untouched.
 *
 * Returns: 0 on success, or -1 if the scratch allocation failed (in which
 *          case the array is left unmodified).
 */
int mergesort_longs(long *buffer, int count) {
    if (count < 2) {
        return 0;
    }

    long *scratch = malloc((size_t)count * sizeof(long));
    if (scratch == NULL) {
        return -1; /* caller's data is untouched on allocation failure */
    }

    sortSpan(buffer, scratch, 0, count - 1);

    free(scratch);
    return 0;
}
