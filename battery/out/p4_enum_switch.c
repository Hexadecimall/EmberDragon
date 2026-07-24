#include <cstdio>

long name(int a);
long apply(int a, int b, int c);


int main(int argc, char** argv) {
    int* obj;
    char v64;
    long v32;
    int v28;
    long v16;
    // neon:  ldr    value0, [v105, #0x0]  // = 0x00000000010000000200000003000000
    obj = &v64;
    v32 = &v64 + 28;
    while (obj != v32) {
        v28 = *obj;
        v16 = name(v28);
        printf("%-7s(12,5) = %d\n", v16, apply(v28, 12, 5));
        obj += 4;
    }
    return 0;
}

long name(Op a) {
    if (a != 0) {
        if (a == 1) goto L1;
        if (a == 2) goto L2;
        if (a - 3 < 1) goto L3;
        if (a == 5) goto L4;
    } else {
        return "add";
        L1:
        return "sub";
        L2:
        return "mul";
        L3:
        return "divlike";
        L4:
        return "neg";
    }
    return "nop";
}

long apply(Op a, int b, int c) {
    int result;
    int ret;
    switch (a) {
    case 0: {
        return (b + c);
    }
    case 1: {
        return (b - c);
    }
    case 2: {
        return (b * c);
    }
    case 3: {
        if (c != 0) {
            result = b / c;
        } else {
            result = 0;
        }
        return result;
    }
    case 4: {
        if (c != 0) {
            ret = b - b / c * c;
        } else {
            ret = 0;
        }
        return ret;
    }
    case 5: {
        return (0 - b);
    }
    default: {
        return b;
    }
    }
}

