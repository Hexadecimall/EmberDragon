#include <cstdio>

long fold(long a, long b, long arr);


int main(int argc, char** argv) {
    long acc;
    int i;
    long v40;
    long v32;
    long v24;
    long v16;
    acc = argc;
    i = 0;
    while (i < 20) {
        v40 = i - 7;
        v32 = i * i - i * i / 11 * 11;
        v24 = 5 ^ i << 1;
        acc += fold(v40, v32, v24);
        acc = (acc ^ acc >> 13) * 40503 + i;
        if (i) {
            v16 = v40 + v24;
        } else {
            v16 = v32 - v40;
        }
        acc -= v16;
        i++;
    }
    printf("acc=%lld\n", acc);
    return (acc & 127);
}

long fold(long a, long b, long arr) {
    long ptr;
    long v40;
    long v32;
    long v56;
    long v24;
    long v16;
    long v8;
    long v48;
    ptr = a * 3 + b - (arr << 2);
    v40 = ptr * ptr;
    if (a != 0) {
        v32 = b - b / (arr | 1) * (arr | 1);
    } else {
        v32 = 0;
    }
    ptr = v40 - v32;
    v56 = (a & b | b ^ arr) + ~a;
    if (a > b) {
        v24 = a - b;
    } else {
        v24 = b - a;
    }
    v16 = v24;
    if (arr != 0) {
        if (a != 0) {
            v8 = arr / (a == 0 ? 0 : 1);
            v48 = v16 + v8;
            return (ptr / (v56 | 1) + (v48 << (a & 3)) - (b >> 1));
        }
    } else {
        v8 = 7;
        v48 = v16 + v8;
        return (ptr / (v56 | 1) + (v48 << (a & 3)) - (b >> 1));
    }
    v48 = v16 + v8;
    return (ptr / (v56 | 1) + (v48 << (a & 3)) - (b >> 1));
}

