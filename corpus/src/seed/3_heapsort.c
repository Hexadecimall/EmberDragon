#include <stdint.h>

/* Heapsort of sensor sample readings using a max-heap built in place. */

static void exchangeSamples(int32_t *samples, int a, int b) {
    int32_t held = samples[a];
    samples[a] = samples[b];
    samples[b] = held;
}

static void heapifyDown(int32_t *samples, int heapSize, int root) {
    while (1) {
        int leftChild = 2 * root + 1;
        int rightChild = 2 * root + 2;
        int largest = root;

        if (leftChild < heapSize && samples[leftChild] > samples[largest]) {
            largest = leftChild;
        }
        if (rightChild < heapSize && samples[rightChild] > samples[largest]) {
            largest = rightChild;
        }
        if (largest == root) {
            break;
        }
        exchangeSamples(samples, root, largest);
        root = largest;
    }
}

static void buildMaxHeap(int32_t *samples, int count) {
    for (int node = count / 2 - 1; node >= 0; node--) {
        heapifyDown(samples, count, node);
    }
}

void heapsortSamples(int32_t *samples, int count) {
    if (count <= 1) {
        return;
    }
    buildMaxHeap(samples, count);
    for (int end = count - 1; end > 0; end--) {
        exchangeSamples(samples, 0, end);
        heapifyDown(samples, end, 0);
    }
}

int32_t rangeSpread(const int32_t *samples, int count) {
    if (count == 0) {
        return 0;
    }
    int32_t minValue = samples[0];
    int32_t maxValue = samples[0];
    for (int i = 1; i < count; i++) {
        if (samples[i] < minValue) {
            minValue = samples[i];
        }
        if (samples[i] > maxValue) {
            maxValue = samples[i];
        }
    }
    return maxValue - minValue;
}

int32_t medianOfSorted(const int32_t *samples, int count) {
    if (count == 0) {
        return 0;
    }
    return samples[count / 2];
}
