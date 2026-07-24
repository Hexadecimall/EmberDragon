#include <stdint.h>
#include <stddef.h>

typedef struct StatSummary {
    int32_t count;
    int64_t total;
    int64_t minimum;
    int64_t maximum;
    int64_t mean;
} StatSummary;

void summarize(const int64_t *samples, int32_t length, StatSummary *summary) {
    summary->count = length;
    summary->total = 0;
    if (length == 0) {
        summary->minimum = 0;
        summary->maximum = 0;
        summary->mean = 0;
        return;
    }
    summary->minimum = samples[0];
    summary->maximum = samples[0];
    for (int32_t i = 0; i < length; i++) {
        int64_t value = samples[i];
        summary->total += value;
        if (value < summary->minimum) {
            summary->minimum = value;
        }
        if (value > summary->maximum) {
            summary->maximum = value;
        }
    }
    summary->mean = summary->total / length;
}

int64_t variance(const int64_t *samples, int32_t length) {
    if (length == 0) {
        return 0;
    }
    int64_t sum = 0;
    for (int32_t i = 0; i < length; i++) {
        sum += samples[i];
    }
    int64_t mean = sum / length;
    int64_t squaredError = 0;
    for (int32_t i = 0; i < length; i++) {
        int64_t diff = samples[i] - mean;
        squaredError += diff * diff;
    }
    return squaredError / length;
}

int64_t median(int64_t *samples, int32_t length) {
    for (int32_t i = 1; i < length; i++) {
        int64_t key = samples[i];
        int32_t j = i - 1;
        while (j >= 0 && samples[j] > key) {
            samples[j + 1] = samples[j];
            j--;
        }
        samples[j + 1] = key;
    }
    if (length == 0) {
        return 0;
    }
    if (length % 2 == 1) {
        return samples[length / 2];
    }
    int64_t lower = samples[length / 2 - 1];
    int64_t upper = samples[length / 2];
    return (lower + upper) / 2;
}

int32_t countAbove(const int64_t *samples, int32_t length, int64_t threshold) {
    int32_t hits = 0;
    for (int32_t i = 0; i < length; i++) {
        if (samples[i] > threshold) {
            hits++;
        }
    }
    return hits;
}

int64_t range(const StatSummary *summary) {
    return summary->maximum - summary->minimum;
}
