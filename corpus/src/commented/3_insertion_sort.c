/*
 * insertion_sort.c
 *
 * Insertion sort specialized for sorting an array of records by an integer
 * key. Insertion sort is stable, adaptive (nearly linear on almost-sorted
 * input), and a common base case for hybrid sorts on small subarrays.
 */

#include <stdint.h>

/*
 * A simple record sorted by `key`. The `payload` rides along untouched so we
 * can confirm the sort actually moves whole records, not just keys.
 */
typedef struct Record {
    int32_t key;     /* the value the order is determined by */
    int32_t payload; /* arbitrary data carried with the key  */
} Record;

/*
 * Sort an array of records ascending by key using straight insertion.
 *
 * Each iteration grows a sorted prefix records[0..i-1] by lifting out the next
 * element and shifting larger keys one slot right until the gap is the correct
 * landing spot. The '>' comparison (not '>=') makes the sort stable.
 *
 * Parameters:
 *   records - array to sort in place.
 *   count   - number of records; values < 2 are already sorted.
 *
 * Returns: nothing. Time O(n^2) worst case, O(n) on already-sorted input.
 */
void insertionSort(Record *records, int count) {
    for (int i = 1; i < count; i++) {
        Record current = records[i]; /* the element being inserted */
        int j = i - 1;               /* scan back through the sorted prefix */

        /* Slide every record with a strictly larger key up by one position. */
        while (j >= 0 && records[j].key > current.key) {
            records[j + 1] = records[j];
            j--;
        }

        /* j+1 is now the open slot where `current` belongs. */
        records[j + 1] = current;
    }
}

/*
 * Binary insertion sort variant: find the insertion point with a binary search
 * over the sorted prefix, then shift. This does fewer comparisons than the
 * linear scan above, though the number of element moves is unchanged.
 *
 * Parameters:
 *   records - array to sort in place.
 *   count   - number of records.
 *
 * Returns: nothing. Comparisons O(n log n), moves still O(n^2).
 */
void binaryInsertionSort(Record *records, int count) {
    for (int i = 1; i < count; i++) {
        Record current = records[i];

        /* Locate the first index in [0, i) whose key exceeds current.key. */
        int lo = 0;
        int hi = i;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (records[mid].key <= current.key) {
                lo = mid + 1; /* keep equals to the left for stability */
            } else {
                hi = mid;
            }
        }

        /* Shift the tail [lo, i) right by one to open a slot at lo. */
        for (int j = i; j > lo; j--) {
            records[j] = records[j - 1];
        }
        records[lo] = current;
    }
}

/*
 * Report whether an array of records is sorted non-decreasing by key.
 * Useful as a post-condition check on the routines above.
 *
 * Parameters:
 *   records - array to inspect.
 *   count   - number of records.
 *
 * Returns: 1 if sorted (or fewer than two elements), 0 otherwise.
 */
int isSortedByKey(const Record *records, int count) {
    for (int i = 1; i < count; i++) {
        if (records[i - 1].key > records[i].key) {
            return 0; /* found an out-of-order adjacent pair */
        }
    }
    return 1;
}
