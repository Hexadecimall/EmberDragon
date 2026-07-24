/*
 * stats.c — Integer descriptive statistics over an array of samples.
 *
 * Computes count, sum, mean (floored), min, max, range, and the
 * population variance for a buffer of 32-bit integer samples. All
 * results are exact integers; the mean and variance use integer/floored
 * division rather than floating point.
 */

#include <stdint.h>

/* Aggregated summary of a sample set. Filled by stats_summarize. */
typedef struct {
    int32_t count;    /* number of samples examined          */
    int64_t sum;      /* exact sum of all samples            */
    int32_t minimum;  /* smallest sample (0 if count == 0)   */
    int32_t maximum;  /* largest sample  (0 if count == 0)   */
    int64_t mean;     /* floored sum / count                 */
} Summary;

/*
 * Sum every element of the array.
 *   data: pointer to the samples (read only).
 *   n:    number of samples; may be 0.
 * Returns the exact sum as a 64-bit integer to avoid 32-bit overflow.
 * O(n).
 */
int64_t stats_sum(const int32_t *data, int32_t n) {
    int64_t total = 0;
    for (int32_t i = 0; i < n; i++) {
        total += data[i];
    }
    return total;
}

/*
 * Find the smallest element.
 *   data: pointer to the samples.
 *   n:    number of samples.
 * Returns the minimum value, or 0 if n <= 0 (an empty set has no min).
 * O(n).
 */
int32_t stats_min(const int32_t *data, int32_t n) {
    if (n <= 0) return 0;        /* nothing to compare */
    int32_t best = data[0];
    for (int32_t i = 1; i < n; i++) {
        if (data[i] < best) best = data[i];
    }
    return best;
}

/*
 * Find the largest element.
 *   data: pointer to the samples.
 *   n:    number of samples.
 * Returns the maximum value, or 0 if n <= 0. O(n).
 */
int32_t stats_max(const int32_t *data, int32_t n) {
    if (n <= 0) return 0;
    int32_t best = data[0];
    for (int32_t i = 1; i < n; i++) {
        if (data[i] > best) best = data[i];
    }
    return best;
}

/*
 * Compute the floored arithmetic mean.
 *   data: pointer to the samples.
 *   n:    number of samples.
 * Returns floor(sum / n), or 0 if n == 0 to avoid division by zero. O(n).
 */
int64_t stats_mean(const int32_t *data, int32_t n) {
    if (n == 0) return 0;        /* guard against divide-by-zero */
    return stats_sum(data, n) / n;
}

/*
 * Compute the population variance scaled by n (i.e. the sum of squared
 * deviations from the mean). Returning the unscaled sum keeps the result
 * an exact integer; divide by n yourself for the true variance.
 *   data: pointer to the samples.
 *   n:    number of samples.
 * Returns sum_i (data[i] - mean)^2, or 0 if n == 0. O(n).
 */
int64_t stats_sum_squared_deviation(const int32_t *data, int32_t n) {
    if (n == 0) return 0;
    int64_t mean = stats_mean(data, n);
    int64_t accum = 0;
    for (int32_t i = 0; i < n; i++) {
        int64_t delta = (int64_t)data[i] - mean;
        accum += delta * delta;   /* squaring keeps the sign positive */
    }
    return accum;
}

/*
 * Fill a Summary struct with all the basic statistics in one pass-set.
 *   data: pointer to the samples.
 *   n:    number of samples.
 *   out:  destination summary, fully overwritten.
 * Handles the empty case gracefully by zeroing every field. The caller
 * owns `out`.
 */
void stats_summarize(const int32_t *data, int32_t n, Summary *out) {
    out->count   = n;
    out->sum     = stats_sum(data, n);
    out->minimum = stats_min(data, n);
    out->maximum = stats_max(data, n);
    out->mean    = stats_mean(data, n);
}

/*
 * Convenience accessor: the range (max - min) of the sample set.
 *   data: pointer to the samples.
 *   n:    number of samples.
 * Returns max - min, or 0 if the set is empty. O(n).
 */
int64_t stats_range(const int32_t *data, int32_t n) {
    if (n <= 0) return 0;
    return (int64_t)stats_max(data, n) - (int64_t)stats_min(data, n);
}
