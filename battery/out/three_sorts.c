#include <cstdio>
#include <cstring>

void bubble_sort(long arr, int a);
void insertion_sort(long arr, int n2);
void selection_sort(long arr, int n2);

const char* s_d = "%d ";
const char* s_str = "| ";

static const int k = 7;
static const int k2 = 2;
static const long data_init[2] = {
    4294967299, 4294967300 
};
static const int k3 = 5;
static const char str[] = "\n";

int main(int argc, char** argv) {
    long ch;
    long ch3;
    long data[2];
    long ch2;
    int n;
    char v80;
    char v112;
    int j;
    int i;
    // neon:  ldr    value0, [v137, #0x0]  // = 0x05000000020000000900000001000000
    ch = k;
    // neon:  ldr    value0, [v137, #0x0]  // = 0x08000000040000000000000006000000
    ch3 = k2;
    // neon:  ldr    value0, [v137, #0x0]  // = 0x03000000010000000400000001000000
    memcpy(data, data_init, 16);  // data assembled on the stack via NEON
    ch2 = k3;
    n = 6;
    bubble_sort(v80, n);
    insertion_sort(&v80, n);
    selection_sort(data, n);
    k = 0;
    while (k < n) {
        printf(&s_d, (&v112)[k << 2]);
        k++;
    }
    printf(&s_str);
    j = 0;
    while (j < n) {
        printf(&s_d, (&v80)[j << 2]);
        j++;
    }
    printf(&s_str);
    i = 0;
    while (i < n) {
        printf(&s_d, data[i << 2]);
        i++;
    }
    printf(str);
    return 0;
}

void bubble_sort(long arr, int a) {
int j;
int i;
int v8;
    j = 0;
    while (j < a - 1) {
        i = 0;
        while (i < a - 1 - j) {
            if (!(arr[i] <= arr[i + 1])) {
                v8 = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = v8;
            }
            i++;
        }
        j++;
    }
    return;
}

void insertion_sort(long arr, int n2) {
int n;
int i;
int v12;
int v8;
int v4;
    n = n2;
    i = 1;
    while (i < n) {
        v12 = arr[i];
        v8 = i - 1;
        v4 = 0;
        while (!((v8 & 1<<31) != 0)) {
            v4 = (arr[v8] <= v12 ? 0 : 1);
            if (v4) {
                arr[v8 + 1] = arr[v8];
                v8--;
            }
        }
        arr[v8 + 1] = v12;
        i++;
    }
    return;
}

void selection_sort(long arr, int n2) {
int n;
int i;
int v12;
int j;
int v4;
    n = n2;
    i = 0;
    while (i < n - 1) {
        v12 = i;
        j = i + 1;
        while (j < n) {
            if (!(arr[j] >= arr[v12])) {
                v12 = j;
            }
            j++;
        }
        v4 = arr[i];
        arr[i] = arr[v12];
        arr[v12] = v4;
        i++;
    }
    return;
}

