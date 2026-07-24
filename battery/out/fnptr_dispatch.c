#include <cstring>
#include <iostream>

using namespace std;

long op_add(int a, int b);
long op_sub(int a, int b);
long op_mul(int a, int b);
long op_max(int a, int b);


static const char str[] = "(";
static const char str2[] = ",";
static const char str3[] = "\n";
static const int k = 16975616;
static const int k2 = 50791171;
static const long data_init[8] = {
    4503599627375368, 4503599627372376, 4503599627375372, 4503599627372408, 4503599627375376, 4503599627372440, 4503599627375380, 4503599627372472 
};

int main(int argc, char** argv) {
long data[8];
long ch;
int ch2;
int n;
long sum;
int i;
int v24;
int v20;
int v16;
int v12;
long str;
    memcpy(data, data_init, 64);
    ch = k;
    ch2 = k2;
    n = 4;
    sum = 0;
    i = 0;
    while (i < n) {
        v24 = (&ch)[i * 3 + 0] - (&ch)[i * 3 + 0] / 4 * 4;
        v20 = (&ch)[i * 3 + 1];
        v16 = (&ch)[i * 3 + 2];
        v12 = *(data + (v24 << 4))[8]();
        cout << data[(long)(v24) << 4] << str << v20 << str2 << v16 << ")=" << v12 << str3;
        sum += v12;
        i++;
    }
    cout << "acc=" << sum << str3;
    return 0;
}

long op_add(int a, int b) {
    return (a + b);
}

long op_sub(int a, int b) {
    return (a - b);
}

long op_mul(int a, int b) {
    return (a * b);
}

long op_max(int a, int b) {
    int result;
    if (a > b) {
        return a;
    } else {
        return b;
    }
    return result;
}

