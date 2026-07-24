#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    int result;
    char buf6[64];
    char buf7[64];
    char buf4[64];
    char buf5[64];
    char buf2[64];
    char buf3[64];
    int k2;
    int j2;
    int v80;
    int v84;
    int i2;
    int m;
    int l;
    int v72;
    int v76;
    int* ptr;
    long sum;
    int k;
    int j;
    char buf[64];
    int ptr2;
    long index2;
    long v152;
    int v148;
    long index;
    long v32;
    long v24;
    int i;
    long index3;
    long v104;
    long v0;
    char __addr0[64];
    char __addr1[64];
    result = 0;
    vector(buf6, 4);
    vector(buf7, 4, buf6);
    vector(buf4, 4);
    vector(buf5, 4, buf4);
    vector(buf2, 4, __addr0);
    vector(buf3, 4, buf2);
    k2 = 0;
    while (k2 < 4) {
        j2 = 0;
        while (j2 < 4) {
            v80 = j2 + (k2 << 2) + 1;
            operator_index(buf7, k2);
            *operator_index(j2) = v80;
            v84 = (k2 == j2 ? 2 : 0);
            operator_index(buf5, k2);
            *operator_index(j2) = v84;
            j2++;
        }
        k2++;
    }
    i2 = 0;
    while (i2 < 4) {
        m = 0;
        while (m < 4) {
            l = 0;
            while (l < 4) {
                operator_index(buf7, i2);
                v72 = *operator_index(l);
                operator_index(buf5, l);
                v76 = v72 * *operator_index(m);
                operator_index(buf3, i2);
                ptr = operator_index(m);
                *ptr = *ptr + v76;
                l++;
            }
            m++;
        }
        i2++;
    }
    sum = 0;
    k = 0;
    while (k < 4) {
        j = 0;
        while (j < 4) {
            if (k == j) {
                operator_index(buf3, k);
                sum += *operator_index(j);
            }
            j++;
        }
        k++;
    }
    vector(buf, 31, __addr1);
    ptr2 = 2;
    while (!(ptr2 * ptr2 > 30)) {
        index2 = operator_index(buf, ptr2);
        v152 = index2;
        if (operator_bool(&v152)) {
            v148 = ptr2 * ptr2;
            while (v148 <= 30) {
                index = operator_index(buf, v148);
                v148 += ptr2;
            }
        }
        ptr2++;
    }
    v32 = cout << "trace(2A) = ";
    v24 = v32 << sum;
    v24 << "\nprimes<=30:";
    i = 2;
    while (i <= 30) {
        index3 = operator_index(buf, i);
        v104 = index3;
        if (operator_bool(&v104)) {
            v0 = cout << 32;
            v0 << i;
        }
        i++;
    }
    cout << 10;
    return 0;
}

