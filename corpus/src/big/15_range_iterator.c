/*
 * range_iterator.c — A lazy, strided integer range iterator.
 *
 * Implements a Python-style range as an explicit struct that yields values
 * one at a time without materializing them. Supports positive and negative
 * strides, exposes the classic has-next / next protocol, and offers a few
 * reductions (sum, count) that consume an iterator. Pure integer logic.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * Iterator state over an arithmetic progression.
 *
 * `current` holds the value the next call to range_next() will return.
 * `stop` is exclusive. `step` may be negative for descending ranges but
 * must never be zero (an invariant the constructor enforces).
 */
typedef struct {
    int64_t current; /* next value to emit */
    int64_t stop;    /* exclusive end bound */
    int64_t step;    /* signed stride, non-zero */
} RangeIterator;

/*
 * Construct a range iterator [start, stop) advancing by step.
 *
 * start:  first value to emit (if the range is non-empty).
 * stop:   exclusive upper/lower bound depending on step's sign.
 * step:   stride; a zero step is coerced to 1 to preserve the invariant
 *         that the iterator always makes progress.
 * return: an initialized iterator by value (no allocation).
 */
RangeIterator range_make(int64_t start, int64_t stop, int64_t step) {
    RangeIterator it;
    it.current = start;
    it.stop = stop;
    it.step = (step == 0) ? 1 : step; /* never allow a non-advancing range */
    return it;
}

/*
 * Report whether the iterator has another value to yield.
 *
 * The exclusive bound is interpreted relative to the stride direction:
 * ascending ranges run while current < stop, descending while current > stop.
 *
 * it:     pointer to the iterator (not modified).
 * return: 1 if range_next() would produce a value, 0 if exhausted.
 */
int range_has_next(const RangeIterator *it) {
    if (it->step > 0) {
        return it->current < it->stop;
    }
    /* step < 0: we are counting down toward stop */
    return it->current > it->stop;
}

/*
 * Yield the next value and advance the iterator.
 *
 * Callers should guard with range_has_next(); if invoked on an exhausted
 * iterator this still returns `current` but it will be at or past `stop`.
 *
 * it:     pointer to the iterator (advanced in place).
 * return: the current value prior to advancing.
 */
int64_t range_next(RangeIterator *it) {
    int64_t value = it->current;
    it->current += it->step;     /* move one stride toward stop */
    return value;
}

/*
 * Reset an iterator to begin again from a new start.
 *
 * Reuses the existing stop and step, which is handy for replaying a range
 * without rebuilding it.
 *
 * it:     pointer to the iterator to rewind.
 * start:  the value emission should resume from.
 */
void range_reset(RangeIterator *it, int64_t start) {
    it->current = start;
}

/*
 * Compute how many values the iterator will still yield.
 *
 * Derived arithmetically rather than by iterating, so it is O(1). Returns 0
 * when the range is already empty or points the wrong way.
 *
 * it:     pointer to the iterator (not modified).
 * return: remaining element count.
 */
int64_t range_remaining(const RangeIterator *it) {
    int64_t span = it->stop - it->current;
    /* If span and step disagree in sign the range is empty. */
    if ((it->step > 0 && span <= 0) || (it->step < 0 && span >= 0)) {
        return 0;
    }
    /* Magnitude of the stride; abs without <math.h>. */
    int64_t stride = it->step > 0 ? it->step : -it->step;
    if (span < 0) {
        span = -span;
    }
    /* Ceiling division: any partial final step still counts as one element. */
    return (span + stride - 1) / stride;
}

/*
 * Sum every remaining value the iterator would yield, consuming it.
 *
 * After this call the iterator is exhausted. Uses range_next() so the
 * traversal logic lives in exactly one place.
 *
 * it:     pointer to the iterator (driven to exhaustion).
 * return: the integer sum of the remaining elements (0 if none).
 */
int64_t range_sum(RangeIterator *it) {
    int64_t total = 0;
    while (range_has_next(it)) {
        total += range_next(it);
    }
    return total;
}

/*
 * Collect the remaining values into a caller-supplied buffer.
 *
 * Stops early if the buffer fills, so a small buffer yields a prefix. The
 * iterator is advanced only as far as values are actually written.
 *
 * it:       pointer to the iterator.
 * out:      destination array.
 * capacity: number of slots in out.
 * return:   number of values written.
 */
uint32_t range_collect(RangeIterator *it, int64_t *out, uint32_t capacity) {
    uint32_t written = 0;
    while (written < capacity && range_has_next(it)) {
        out[written++] = range_next(it);
    }
    return written;
}
