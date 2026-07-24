#include <cstdio>
#include <cstring>

void quicksort(long a, int b, int n2);
int partition(long arr, int a, int arr2);
void swap_int(int* obj3, int* obj4);


static const int table_init[11] = {
    9, 3, 7, 1, 8, 2, 6, 5, 4, 0, 3 
};
static const char str[] = " ";
static const char str2[] = "\n";

int main(int argc, char** argv) {
    int table[11];
    int n;
    int i;
    memcpy(table, table_init, 44);
    n = 11;
    quicksort(table, 0, n - 1);
    i = 0;
    while (i < n) {
        printf("%d%s", table[i], (i + 1 < n ? str : str2));
        i++;
    }
    return 0;
}

void quicksort(long a, int b, int n2) {
    int n;
    int v12;
    n = n2;
    if (b < n) {
        v12 = partition(a, b, n);
        quicksort(a, b, v12 - 1);
        quicksort(a, v12 + 1, n);
        return;
    }
    return;
}

int partition(long arr, int a, int arr2) {
int n;
int v12;
int v8;
int i;
    n = arr2;
    v12 = arr[n];
    v8 = a - 1;
    i = a;
    while (i < n) {
        if (!(arr[i] > v12)) {
            v8++;
            swap_int(arr + (v8 << 2), arr + (i << 2));
        }
        i++;
    }
    swap_int(arr + (v8 + 1 << 2), arr + (n << 2));
    return (v8 + 1);
}

void swap_int(int* obj3, int* obj4) {
    int v12;
    v12 = *obj3;
    *obj3 = *obj4;
    *obj4 = v12;
    return;
}

