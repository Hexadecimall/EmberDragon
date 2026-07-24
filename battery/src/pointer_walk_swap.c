/* Pointer-walk over an array with a moving pointer + in-place reverse via swap-by-pointer. */
#include <stdio.h>

static void swap_ints(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static long walk_sum(const int *arr, int n) {
    long s = 0;
    const int *end = arr + n;
    for (const int *p = arr; p < end; p++)
        s += *p;
    return s;
}

static void reverse(int *arr, int n) {
    int *lo = arr;
    int *hi = arr + n - 1;
    while (lo < hi) {
        swap_ints(lo, hi);
        lo++;
        hi--;
    }
}

int main(void) {
    int arr[] = {1, 2, 3, 4, 5, 6, 7};
    int n = (int)(sizeof(arr) / sizeof(arr[0]));
    printf("sum=%ld\n", walk_sum(arr, n));
    reverse(arr, n);
    printf("reversed:");
    for (int *p = arr; p < arr + n; p++)
        printf(" %d", *p);
    printf("\n");
    return 0;
}
