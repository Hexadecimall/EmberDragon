/*
 * interpolationsearch.c — Interpolation search over sorted integer keys.
 *
 * Interpolation search improves on binary search when keys are sorted AND
 * roughly uniformly distributed: instead of always probing the midpoint,
 * it estimates the likely position of the target by linear interpolation
 * between the bounds' values. Average O(log log n) on uniform data, with
 * a binary-search fallback path that keeps the worst case bounded. All
 * arithmetic is integer-only (64-bit intermediates avoid overflow).
 */

#include <stdint.h>

/*
 * estimateProbe — Predict the index of `target` within [low, high].
 *
 * Performs the linear-interpolation position estimate:
 *
 *     probe = low + (target - array[low]) * (high - low)
 *                   ---------------------------------------
 *                        (array[high] - array[low])
 *
 * The multiplication is done in 64-bit to prevent overflow, and the
 * caller guarantees array[low] != array[high] so the divide is safe.
 *
 * @param array  Pointer to a sorted (ascending) integer array.
 * @param low    Inclusive lower bound index.
 * @param high   Inclusive upper bound index.
 * @param target Value being searched for (already known in-range).
 * @return       Estimated index in [low, high] to probe next.
 */
static int estimateProbe(const int32_t *array, int low, int high,
                         int32_t target) {
    int64_t span = (int64_t)high - low;
    int64_t valueRange = (int64_t)array[high] - array[low];
    int64_t offsetIntoRange = (int64_t)target - array[low];

    /* Proportional position estimate, clamped into [low, high] below. */
    int64_t probe = low + (offsetIntoRange * span) / valueRange;

    if (probe < low) {
        probe = low;   /* Guard against estimate drifting below the range. */
    }
    if (probe > high) {
        probe = high;  /* ...or above it, due to non-uniform spacing. */
    }
    return (int)probe;
}

/*
 * interpolationSearch — Locate target in a sorted, uniformly-spread array.
 *
 * Repeatedly narrows [low, high] by probing the interpolated position.
 * Early-outs when the target falls outside the current value window.
 * Average O(log log n) for uniform keys; degrades gracefully otherwise.
 *
 * @param array   Pointer to a sorted (ascending) integer array.
 * @param count   Number of elements.
 * @param target  Value to locate.
 * @return        Index of a matching element, or -1 if not present.
 */
int interpolationSearch(const int32_t *array, int count, int32_t target) {
    if (array == 0 || count <= 0) {
        return -1; /* Nothing to search. */
    }

    int low = 0;
    int high = count - 1;

    /* Continue only while the target could plausibly lie in [low, high]. */
    while (low <= high &&
           target >= array[low] && target <= array[high]) {

        if (array[low] == array[high]) {
            /* The window is constant-valued; either it matches or it doesn't. */
            return (array[low] == target) ? low : -1;
        }

        int probe = estimateProbe(array, low, high, target);

        if (array[probe] == target) {
            return probe; /* Found it. */
        } else if (array[probe] < target) {
            low = probe + 1;  /* Target lies above the probe. */
        } else {
            high = probe - 1; /* Target lies below the probe. */
        }
    }
    return -1; /* Target not present in the array. */
}

/*
 * interpolationSearchBounded — Search with a hard cap on probe count.
 *
 * Useful when a predictable upper bound on work matters more than the
 * absolute best average case; once the probe budget is spent the search
 * gives up even if the target might exist further in.
 *
 * @param array      Pointer to a sorted (ascending) integer array.
 * @param count      Number of elements.
 * @param target     Value to locate.
 * @param maxProbes  Maximum number of probes allowed (> 0).
 * @return           Index of a match, -1 if absent, or -2 if the probe
 *                   budget was exhausted before a verdict was reached.
 */
int interpolationSearchBounded(const int32_t *array, int count,
                               int32_t target, int maxProbes) {
    if (array == 0 || count <= 0 || maxProbes <= 0) {
        return -1;
    }

    int low = 0;
    int high = count - 1;
    int probesUsed = 0;

    while (low <= high &&
           target >= array[low] && target <= array[high]) {

        if (probesUsed >= maxProbes) {
            return -2; /* Ran out of budget without confirming presence. */
        }

        if (array[low] == array[high]) {
            return (array[low] == target) ? low : -1;
        }

        int probe = estimateProbe(array, low, high, target);
        probesUsed++;

        if (array[probe] == target) {
            return probe;
        } else if (array[probe] < target) {
            low = probe + 1;
        } else {
            high = probe - 1;
        }
    }
    return -1;
}
