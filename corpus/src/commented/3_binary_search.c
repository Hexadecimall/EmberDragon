/*
 * binary_search.c
 *
 * A family of binary-search routines over a sorted ascending array of signed
 * integers: an exact-match lookup plus the two "bound" queries (lower_bound /
 * upper_bound) that together locate the range of equal keys in O(log n).
 */

#include <stddef.h>
#include <stdint.h>

/*
 * Find an index of `target` within a sorted array.
 *
 * Parameters:
 *   data   - array sorted in ascending order.
 *   count  - number of elements.
 *   target - value to locate.
 *
 * Returns: an index i such that data[i] == target, or -1 if the value is
 *          absent. If duplicates exist, the returned index is unspecified
 *          among them. Runs in O(log n).
 */
int binarySearch(const int32_t *data, int count, int32_t target) {
    int lo = 0;
    int hi = count - 1; /* inclusive upper bound of the search window */

    while (lo <= hi) {
        /* Computed this way to avoid integer overflow on large indices. */
        int mid = lo + (hi - lo) / 2;

        if (data[mid] == target) {
            return mid;
        } else if (data[mid] < target) {
            lo = mid + 1; /* target is in the right half */
        } else {
            hi = mid - 1; /* target is in the left half */
        }
    }

    return -1; /* search window collapsed without a match */
}

/*
 * Lower bound: the index of the first element not less than `target`.
 *
 * Parameters:
 *   data   - array sorted in ascending order.
 *   count  - number of elements.
 *   target - value to bound.
 *
 * Returns: the smallest index i with data[i] >= target, or `count` if every
 *          element is strictly less than target. Runs in O(log n).
 */
int lowerBound(const int32_t *data, int count, int32_t target) {
    int lo = 0;
    int hi = count; /* half-open: hi is one past the last candidate */

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (data[mid] < target) {
            lo = mid + 1; /* mid is too small to be the answer */
        } else {
            hi = mid;     /* mid might be the answer; keep it in range */
        }
    }

    return lo;
}

/*
 * Upper bound: the index of the first element strictly greater than `target`.
 *
 * Parameters:
 *   data   - array sorted in ascending order.
 *   count  - number of elements.
 *   target - value to bound.
 *
 * Returns: the smallest index i with data[i] > target, or `count` if no
 *          element exceeds target. Runs in O(log n).
 */
int upperBound(const int32_t *data, int count, int32_t target) {
    int lo = 0;
    int hi = count;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (data[mid] <= target) {
            lo = mid + 1; /* skip elements equal to or below target */
        } else {
            hi = mid;
        }
    }

    return lo;
}

/*
 * Count how many times `target` occurs in the sorted array. Implemented as the
 * width of the [lowerBound, upperBound) range, so it costs two log-time
 * searches rather than a linear scan.
 *
 * Parameters:
 *   data   - array sorted in ascending order.
 *   count  - number of elements.
 *   target - value to count.
 *
 * Returns: the number of occurrences of target (0 if absent).
 */
int countOccurrences(const int32_t *data, int count, int32_t target) {
    int first = lowerBound(data, count, target);
    int past  = upperBound(data, count, target);
    return past - first;
}
