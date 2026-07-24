/*
 * medianofmedians.cpp — Deterministic linear-time k-th selection.
 *
 * Selects the k-th smallest element using the median-of-medians pivot
 * strategy (the BFPRT algorithm), which guarantees worst-case O(n) time
 * unlike randomized quickselect. The array is split into groups of five,
 * each group's median is found, and the median of those medians becomes a
 * provably good pivot. The input array is permuted in place.
 */

#include <cstdint>

namespace selection {

/*
 * swapAt — Exchange two elements of an array.
 *
 * @param data  Pointer to the integer array.
 * @param i     First index.
 * @param j     Second index.
 */
static void swapAt(int32_t *data, int i, int j) {
    int32_t temp = data[i];
    data[i] = data[j];
    data[j] = temp;
}

/*
 * insertionSortRange — Sort the small inclusive range [lo, hi] ascending.
 *
 * Used only on groups of at most five elements, where insertion sort's
 * simplicity beats anything fancier. Runs in O(m^2) for m = hi - lo + 1,
 * which is constant here.
 *
 * @param data  Pointer to the integer array.
 * @param lo    Inclusive lower bound.
 * @param hi    Inclusive upper bound.
 */
static void insertionSortRange(int32_t *data, int lo, int hi) {
    for (int i = lo + 1; i <= hi; i++) {
        int32_t key = data[i];
        int j = i - 1;
        /* Shift larger elements right to open a slot for `key`. */
        while (j >= lo && data[j] > key) {
            data[j + 1] = data[j];
            j--;
        }
        data[j + 1] = key;
    }
}

/*
 * partitionAround — Lomuto partition using the value at pivotIndex.
 *
 * @param data        Pointer to the integer array.
 * @param lo          Inclusive lower bound.
 * @param hi          Inclusive upper bound.
 * @param pivotIndex  Index of the chosen pivot within [lo, hi].
 * @return            Final index of the pivot after partitioning.
 */
static int partitionAround(int32_t *data, int lo, int hi, int pivotIndex) {
    int32_t pivotValue = data[pivotIndex];
    swapAt(data, pivotIndex, hi); /* Move pivot out of the scan range. */

    int boundary = lo; /* Invariant: data[lo..boundary) < pivotValue. */
    for (int scan = lo; scan < hi; scan++) {
        if (data[scan] < pivotValue) {
            swapAt(data, boundary, scan);
            boundary++;
        }
    }
    swapAt(data, boundary, hi); /* Place pivot at its sorted boundary. */
    return boundary;
}

/* Forward declaration: selectKth and medianOfMediansPivot are mutually
 * recursive — the pivot routine recurses into selectKth on the medians. */
static int selectKth(int32_t *data, int lo, int hi, int k);

/*
 * medianOfMediansPivot — Compute a high-quality pivot index for [lo, hi].
 *
 * Divides the range into groups of five, sorts each group to expose its
 * median, gathers those medians at the front of the range, then selects
 * their median recursively. The resulting pivot guarantees that at least
 * 30% of elements lie on each side — the key to the linear-time bound.
 *
 * @param data  Pointer to the integer array.
 * @param lo    Inclusive lower bound.
 * @param hi    Inclusive upper bound.
 * @return      An index in [lo, hi] whose value is a robust pivot.
 */
static int medianOfMediansPivot(int32_t *data, int lo, int hi) {
    int n = hi - lo + 1;
    if (n <= 5) {
        insertionSortRange(data, lo, hi);
        return lo + n / 2; /* The median of a tiny group is its middle. */
    }

    /* Sort each group of five and move its median to the front block. */
    int numMedians = 0;
    for (int groupStart = lo; groupStart <= hi; groupStart += 5) {
        int groupEnd = groupStart + 4;
        if (groupEnd > hi) {
            groupEnd = hi; /* The final group may be short. */
        }
        insertionSortRange(data, groupStart, groupEnd);

        int medianIndex = groupStart + (groupEnd - groupStart) / 2;
        /* Compact medians into data[lo .. lo+numMedians-1]. */
        swapAt(data, lo + numMedians, medianIndex);
        numMedians++;
    }

    /* Recursively find the median of the gathered medians. */
    int medianRank = numMedians / 2;
    return selectKth(data, lo, lo + numMedians - 1, lo + medianRank);
}

/*
 * selectKth — Internal recursive selection over [lo, hi].
 *
 * @param data  Pointer to the integer array.
 * @param lo    Inclusive lower bound.
 * @param hi    Inclusive upper bound.
 * @param k     Absolute target index (not relative to lo) to select.
 * @return      The index of the k-th smallest element overall.
 */
static int selectKth(int32_t *data, int lo, int hi, int k) {
    while (true) {
        if (lo == hi) {
            return lo; /* One element left; it is the answer. */
        }

        int pivotIndex = medianOfMediansPivot(data, lo, hi);
        int splitPos = partitionAround(data, lo, hi, pivotIndex);

        if (splitPos == k) {
            return splitPos; /* Pivot sits exactly at the wanted rank. */
        } else if (k < splitPos) {
            hi = splitPos - 1; /* Recurse left (loop instead of call). */
        } else {
            lo = splitPos + 1; /* Recurse right. */
        }
    }
}

/*
 * kthSmallest — Public entry point: the k-th smallest value of an array.
 *
 * Guarantees worst-case O(n) time via median-of-medians pivoting. The
 * array is reordered as a side effect of partitioning.
 *
 * @param data   Pointer to the integer array (will be permuted).
 * @param count  Number of elements.
 * @param k      Zero-based rank (0 = minimum, count-1 = maximum).
 * @param ok     Out-param set to true on success, false on invalid input.
 * @return       The k-th smallest value on success; 0 when *ok is false.
 */
int32_t kthSmallest(int32_t *data, int count, int k, bool *ok) {
    if (data == nullptr || count <= 0 || k < 0 || k >= count) {
        if (ok != nullptr) {
            *ok = false; /* Reject empty arrays and out-of-range ranks. */
        }
        return 0;
    }
    if (ok != nullptr) {
        *ok = true;
    }
    int index = selectKth(data, 0, count - 1, k);
    return data[index];
}

} // namespace selection
