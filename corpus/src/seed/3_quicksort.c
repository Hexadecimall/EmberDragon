#include <stdio.h>

/* In-place quicksort over an array of inventory item prices. */

static void swapPrices(int *prices, int left, int right) {
    int held = prices[left];
    prices[left] = prices[right];
    prices[right] = held;
}

static int choosePivotIndex(int low, int high) {
    int middle = low + (high - low) / 2;
    return middle;
}

static int partitionPrices(int *prices, int low, int high) {
    int pivotIndex = choosePivotIndex(low, high);
    int pivotValue = prices[pivotIndex];
    swapPrices(prices, pivotIndex, high);

    int store = low;
    for (int scan = low; scan < high; scan++) {
        if (prices[scan] < pivotValue) {
            swapPrices(prices, store, scan);
            store = store + 1;
        }
    }
    swapPrices(prices, store, high);
    return store;
}

void quicksortRange(int *prices, int low, int high) {
    if (low >= high) {
        return;
    }
    int split = partitionPrices(prices, low, high);
    quicksortRange(prices, low, split - 1);
    quicksortRange(prices, split + 1, high);
}

void sortInventory(int *prices, int count) {
    if (count <= 1) {
        return;
    }
    quicksortRange(prices, 0, count - 1);
}

int isNonDecreasing(const int *prices, int count) {
    for (int i = 1; i < count; i++) {
        if (prices[i - 1] > prices[i]) {
            return 0;
        }
    }
    return 1;
}

int countDistinct(const int *prices, int count) {
    if (count == 0) {
        return 0;
    }
    int distinct = 1;
    for (int i = 1; i < count; i++) {
        if (prices[i] != prices[i - 1]) {
            distinct = distinct + 1;
        }
    }
    return distinct;
}
