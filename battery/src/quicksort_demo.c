/* In-place recursive quicksort with Lomuto partition over a fixed int array. */
#include <stdio.h>

static void swap_int(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static int partition(int arr[], int lo, int hi) {
    int pivot = arr[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap_int(&arr[i], &arr[j]);
        }
    }
    swap_int(&arr[i + 1], &arr[hi]);
    return i + 1;
}

static void quicksort(int arr[], int lo, int hi) {
    if (lo < hi) {
        int p = partition(arr, lo, hi);
        quicksort(arr, lo, p - 1);
        quicksort(arr, p + 1, hi);
    }
}

int main(void) {
    int data[] = {9, 3, 7, 1, 8, 2, 6, 5, 4, 0, 3};
    int n = (int)(sizeof(data) / sizeof(data[0]));
    quicksort(data, 0, n - 1);
    for (int i = 0; i < n; i++) {
        printf("%d%s", data[i], (i + 1 < n) ? " " : "\n");
    }
    return 0;
}
