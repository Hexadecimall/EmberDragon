/*
 * quickselect.c — k-th order statistic via Quickselect (Hoare's algorithm).
 *
 * Finds the k-th smallest element of an array without fully sorting it,
 * using the partition step of quicksort but recursing into only one side.
 * Average O(n) time. A randomized pivot (xorshift PRNG) makes adversarial
 * O(n^2) inputs unlikely. The input array is permuted as a side effect.
 */

#include <stdint.h>

/*
 * nextRandom — Tiny xorshift32 pseudo-random generator.
 *
 * Deterministic, fast, and library-free — used only to pick pivots, so it
 * does not need cryptographic quality. The state is advanced in place.
 *
 * @param state  Address of the 32-bit PRNG state (must be non-zero).
 * @return       The next pseudo-random 32-bit value.
 */
static uint32_t nextRandom(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/*
 * swapInts — Exchange two integer array slots.
 *
 * @param array  Pointer to the integer array.
 * @param i      First index.
 * @param j      Second index.
 */
static void swapInts(int32_t *array, int i, int j) {
    int32_t temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

/*
 * partitionAround — Lomuto partition using the element at pivotIndex.
 *
 * Rearranges array[low..high] so that everything smaller than the pivot
 * precedes it and everything larger follows, returning the pivot's final
 * index. Runs in O(high - low).
 *
 * @param array       Pointer to the integer array.
 * @param low         Inclusive lower bound.
 * @param high        Inclusive upper bound.
 * @param pivotIndex  Index of the chosen pivot within [low, high].
 * @return            Final resting index of the pivot value.
 */
static int partitionAround(int32_t *array, int low, int high, int pivotIndex) {
    int32_t pivotValue = array[pivotIndex];

    /* Stash the pivot at the end so the scan range is clean. */
    swapInts(array, pivotIndex, high);

    int boundary = low; /* Invariant: array[low..boundary) < pivotValue. */
    for (int scan = low; scan < high; scan++) {
        if (array[scan] < pivotValue) {
            swapInts(array, boundary, scan);
            boundary++;
        }
    }

    /* Restore the pivot to the boundary between the partitions. */
    swapInts(array, boundary, high);
    return boundary;
}

/*
 * quickselect — Return the k-th smallest value (0-based) of an array.
 *
 * Partitions around a random pivot and recurses into the only side that
 * can contain rank k, giving O(n) average time. The array is modified in
 * place (its elements get reordered).
 *
 * @param array  Pointer to the integer array (will be permuted).
 * @param count  Number of elements.
 * @param k      Zero-based rank to select (0 = minimum, count-1 = maximum).
 * @return       The k-th smallest value, or 0 with `*ok` unset on misuse.
 *
 * Note: callers needing error signaling should use quickselectChecked.
 */
int32_t quickselect(int32_t *array, int count, int k) {
    /* Clamp k defensively so out-of-range requests stay in bounds. */
    if (k < 0) k = 0;
    if (k >= count) k = count - 1;

    uint32_t rngState = 0x9E3779B9u; /* Fixed seed: reproducible pivots. */
    int low = 0;
    int high = count - 1;

    while (low < high) {
        /* Pick a pivot uniformly at random from the current window. */
        int span = high - low + 1;
        int pivotIndex = low + (int)(nextRandom(&rngState) % (uint32_t)span);

        int splitPos = partitionAround(array, low, high, pivotIndex);

        if (splitPos == k) {
            return array[k]; /* Pivot landed exactly on the target rank. */
        } else if (k < splitPos) {
            high = splitPos - 1; /* The answer is in the left partition. */
        } else {
            low = splitPos + 1;  /* The answer is in the right partition. */
        }
    }
    return array[k]; /* Window collapsed to a single element: that's it. */
}

/*
 * quickselectChecked — quickselect with explicit input validation.
 *
 * @param array  Pointer to the integer array (will be permuted).
 * @param count  Number of elements.
 * @param k      Zero-based rank to select.
 * @param ok     Out-param set to 1 on success, 0 on invalid arguments.
 * @return       The k-th smallest value on success; 0 when *ok == 0.
 */
int32_t quickselectChecked(int32_t *array, int count, int k, int *ok) {
    if (array == 0 || count <= 0 || k < 0 || k >= count) {
        if (ok != 0) {
            *ok = 0; /* Reject empty arrays and out-of-range ranks. */
        }
        return 0;
    }
    if (ok != 0) {
        *ok = 1;
    }
    return quickselect(array, count, k);
}
