/*
 * binarysearch.c — Binary search variants over a sorted integer array.
 *
 * Provides exact-match lookup plus the two boundary queries (lower bound
 * and upper bound) that together describe the range of a repeated key.
 * All routines require the input array to be sorted ascending and run in
 * O(log n) time with O(1) space.
 */

#include <stdint.h>

/*
 * binarySearch — Find the index of a target value in a sorted array.
 *
 * Uses the standard halving search. When the target appears multiple
 * times, the index returned is unspecified (any matching position).
 *
 * @param array   Pointer to a sorted (ascending) integer array.
 * @param count   Number of elements.
 * @param target  Value to locate.
 * @return        Index of a matching element, or -1 if not present.
 */
int binarySearch(const int32_t *array, int count, int32_t target) {
    int low = 0;
    int high = count - 1; /* Inclusive bounds: [low, high]. */

    while (low <= high) {
        /* Overflow-safe midpoint; never computes low+high directly. */
        int mid = low + (high - low) / 2;

        if (array[mid] == target) {
            return mid; /* Hit. */
        } else if (array[mid] < target) {
            low = mid + 1;  /* Target is in the right half. */
        } else {
            high = mid - 1; /* Target is in the left half. */
        }
    }
    return -1; /* Search space exhausted without a match. */
}

/*
 * lowerBound — First index whose value is >= target.
 *
 * Classic half-open binary search. The returned index is the insertion
 * point that keeps the array sorted, and equals `count` when every
 * element is strictly less than target.
 *
 * @param array   Pointer to a sorted (ascending) integer array.
 * @param count   Number of elements.
 * @param target  Value to bound from below.
 * @return        Index in [0, count] of the first element >= target.
 */
int lowerBound(const int32_t *array, int count, int32_t target) {
    int low = 0;
    int high = count; /* Half-open: search range is [low, high). */

    while (low < high) {
        int mid = low + (high - low) / 2;
        if (array[mid] < target) {
            low = mid + 1;  /* mid is too small; discard it and the left. */
        } else {
            high = mid;     /* mid may be the answer; keep it as a candidate. */
        }
    }
    return low; /* low == high == first index not less than target. */
}

/*
 * upperBound — First index whose value is strictly greater than target.
 *
 * Like lowerBound but treats equal elements as "too small", so it lands
 * just past the last occurrence of target.
 *
 * @param array   Pointer to a sorted (ascending) integer array.
 * @param count   Number of elements.
 * @param target  Value to bound from above.
 * @return        Index in [0, count] of the first element > target.
 */
int upperBound(const int32_t *array, int count, int32_t target) {
    int low = 0;
    int high = count;

    while (low < high) {
        int mid = low + (high - low) / 2;
        if (array[mid] <= target) {
            low = mid + 1;  /* Skip past elements equal to target too. */
        } else {
            high = mid;
        }
    }
    return low;
}

/*
 * countOccurrences — Number of times target appears in a sorted array.
 *
 * Computed as the gap between the upper and lower bounds, so it works in
 * O(log n) without scanning the matching run.
 *
 * @param array   Pointer to a sorted (ascending) integer array.
 * @param count   Number of elements.
 * @param target  Value to count.
 * @return        Count of elements equal to target (0 if absent).
 */
int countOccurrences(const int32_t *array, int count, int32_t target) {
    int first = lowerBound(array, count, target);
    int last = upperBound(array, count, target);
    return last - first;
}
