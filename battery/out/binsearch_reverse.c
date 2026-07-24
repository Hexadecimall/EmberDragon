#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

int binary_search(long a, int n2);
void reverse_inplace(long a);


static const int matrix_init[4] = {
    1, 3, 5, 7 
};
static const int matrix2_init[4] = {
    9, 11, 13, 15 
};
static const char str[] = "\n";
static const char str2[] = " ";

int main(int argc, char** argv) {
    int ret;
    int matrix[4];
    int matrix2[4];
    char buf[64];
    long v136;
    int* obj;
    long v120;
    int v116;
    int search;
    int v112;
    long v56;
    long v48;
    long v40;
    long v32;
    long v80;
    long v72;
    char __addr7[64];
    ret = 0;
    // neon:  ldr    value0, [v265, #0x0]  // = 0x01000000030000000500000007000000
    memcpy(matrix, matrix_init, 16);  // data assembled on the stack via NEON
    // neon:  ldr    value0, [v265, #0x10]  // = 0x090000000b0000000d0000000f000000
    memcpy(matrix2, matrix2_init, 16);  // data assembled on the stack via NEON
    vector(buf, matrix, 8);
    v136 = __addr7;
    obj = begin(&v136);
    v120 = end(&v136);
    while (obj != v120) {
        v116 = *obj;
        search = binary_search(buf, v116);
        v112 = search;
        v56 = cout << "find ";
        v48 = v56 << v116;
        v40 = v48 << " -> ";
        v32 = v40 << v112;
        v32 << str;
        obj += 4;
    }
    reverse_inplace(buf);
    cout << "reversed:";
    v80 = begin(buf);
    v72 = end(buf);
    while (v80 != v72) {
        operator_inc&v80;
    }
    cout << str;
    ret = 0;
    return ret;
}

int binary_search(std::vector<int> const& a, int n2) {
    int n;
    int v8;
    int v4;
    int ret;
    n = n2;
    v8 = 0;
    v4 = a.size() - 1;
    while (v8 <= v4) {
        ret = v8 + (v4 - v8) / 2;
        if (*a[ret] == n) {
            return ret;
        }
        if (*a[ret] < n) {
            v8 = ret + 1;
        } else {
            v4 = ret - 1;
        }
    }
    return -1;
}

void reverse_inplace(std::vector<int>& a) {
    int i;
    int n;
    int v12;
    int v4;
    int v8;
    i = 0;
    n = a.size() - 1;
    while (i < n) {
        v12 = *a[i];
        v4 = *a[n];
        *a[i] = v4;
        v8 = v12;
        *a[n] = v8;
        i++;
        n--;
    }
    return;
}

