#include <stdio.h>

static void bubble_sort(int *a, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (a[j] > a[j + 1]) {
                int t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
            }
}

static void insertion_sort(int *a, int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i], j = i - 1;
        while (j >= 0 && a[j] > key) { a[j + 1] = a[j]; j--; }
        a[j + 1] = key;
    }
}

static void selection_sort(int *a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++)
            if (a[j] < a[min]) min = j;
        int t = a[i]; a[i] = a[min]; a[min] = t;
    }
}

int main(void) {
    int s1[] = {5, 2, 9, 1, 7, 3};
    int s2[] = {8, 4, 0, 6, 2, 1};
    int s3[] = {3, 1, 4, 1, 5, 9};
    int n = 6;
    bubble_sort(s1, n);
    insertion_sort(s2, n);
    selection_sort(s3, n);
    for (int i = 0; i < n; i++) printf("%d ", s1[i]);
    printf("| ");
    for (int i = 0; i < n; i++) printf("%d ", s2[i]);
    printf("| ");
    for (int i = 0; i < n; i++) printf("%d ", s3[i]);
    printf("\n");
    return 0;
}
