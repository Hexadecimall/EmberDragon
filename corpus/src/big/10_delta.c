/*
 * Delta encoding for monotonic and slowly-varying integer series.
 *
 * Instead of storing absolute values, this module stores the difference
 * between each element and its predecessor. For sorted timestamps or sampled
 * sensor readings the deltas are small and clustered near zero, which makes a
 * downstream entropy or varint coder far more effective. All math is done in
 * unsigned 32-bit arithmetic so wrap-around is well defined and reversible.
 */

#include <stdint.h>
#include <stddef.h>

/*
 * A fixed-stride view over an integer column, used by the framing helpers.
 *
 * `base` is the first absolute value; `deltas` holds the successive
 * differences. Storing the base separately lets the decoder reconstruct the
 * series without a sentinel.
 */
typedef struct {
    uint32_t base;
    uint32_t *deltas;  /* length == count - 1, owned by the caller. */
    size_t    count;
} DeltaSeries;

/*
 * Convert an absolute series into first-difference form, in place is not used;
 * the result is written to deltas which must hold (count-1) elements.
 *
 * values/count : input column (count >= 1).
 * deltas       : output differences; values[i] - values[i-1] mod 2^32.
 * Returns the base value (values[0]) the decoder needs as a starting point,
 * or 0 if count is 0. O(n).
 */
uint32_t deltaEncode(const uint32_t *values, size_t count, uint32_t *deltas) {
    if (count == 0) {
        return 0;
    }
    for (size_t i = 1; i < count; i++) {
        /* Unsigned subtraction wraps cleanly and is exactly undone by addition. */
        deltas[i - 1] = values[i] - values[i - 1];
    }
    return values[0];
}

/*
 * Rebuild the absolute series from a base and its deltas.
 *
 * base         : first element of the original series.
 * deltas/count : the (count-1)-length difference array followed implicitly by
 *                count, the number of output elements.
 * out          : destination of length count.
 * Returns the number of elements written (count). O(n).
 */
size_t deltaDecode(uint32_t base, const uint32_t *deltas, size_t count,
                   uint32_t *out) {
    if (count == 0) {
        return 0;
    }
    out[0] = base;
    /* Running prefix sum reverses the first-difference transform. */
    for (size_t i = 1; i < count; i++) {
        out[i] = out[i - 1] + deltas[i - 1];
    }
    return count;
}

/*
 * Apply delta-of-delta (second-order) encoding for series with a steady slope.
 *
 * A column that increases by a near-constant step (e.g. evenly spaced
 * timestamps) has nearly identical first deltas; differencing them again
 * concentrates the data even harder around zero. The first stored delta is
 * kept as-is; subsequent entries are deltas of deltas.
 *
 * values/count : input column.
 * dod          : output of length (count-1).
 * Returns the base value, or 0 if count is 0. O(n).
 */
uint32_t deltaOfDeltaEncode(const uint32_t *values, size_t count, uint32_t *dod) {
    if (count == 0) {
        return 0;
    }
    uint32_t prevDelta = 0;
    for (size_t i = 1; i < count; i++) {
        uint32_t d = values[i] - values[i - 1];
        if (i == 1) {
            dod[0] = d; /* The very first delta has no predecessor to diff against. */
        } else {
            dod[i - 1] = d - prevDelta;
        }
        prevDelta = d;
    }
    return values[0];
}

/*
 * Reverse deltaOfDeltaEncode().
 *
 * base/dod/count mirror the encoder's outputs; out receives count elements.
 * Returns the number of elements written. O(n).
 */
size_t deltaOfDeltaDecode(uint32_t base, const uint32_t *dod, size_t count,
                          uint32_t *out) {
    if (count == 0) {
        return 0;
    }
    out[0] = base;
    uint32_t prevDelta = 0;
    for (size_t i = 1; i < count; i++) {
        /* Recover the first delta, then accumulate the second-order terms. */
        uint32_t d = (i == 1) ? dod[0] : prevDelta + dod[i - 1];
        out[i] = out[i - 1] + d;
        prevDelta = d;
    }
    return count;
}

/*
 * Report whether a series is non-decreasing, a common precondition that makes
 * delta values guaranteed non-negative and thus cheaper to pack.
 *
 * Returns 1 if values is sorted ascending (or has <2 elements), else 0. O(n).
 */
int isMonotonic(const uint32_t *values, size_t count) {
    for (size_t i = 1; i < count; i++) {
        if (values[i] < values[i - 1]) {
            return 0;
        }
    }
    return 1;
}
