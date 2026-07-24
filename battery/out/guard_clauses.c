#include <cstring>
#include <iostream>

using namespace std;

int classify_triangle(long a, long b, long c, long d, long e);
long name_for(int a, long b, long c);


static const long data_init[18] = {
    3, 4, 5, 5, 5, 5, 5, 5, 8, 1, 2, 3, -1, 4, 5, 10,
    1, 1 
};

struct Struct0 {
    long value;
    long data;
    long item;
};
int main(int argc, char** argv) {
    long data[18];
    long v56;
    long v48;
    long v40;
    struct Struct0* obj;
    int triangle;
    long v8;
    memcpy(data, data_init, 144);
    v56 = data;
    v48 = v56;
    v40 = v56 + 144;
    while (v48 != v40) {
        obj = v48;
        triangle = classify_triangle(obj->value, obj->data, obj->item);
        cout << obj->value;
        operator_lsh44;
        operator_lshobj->data;
        operator_lsh44;
        operator_lshobj->item;
        v8 = operator_lsh" -> ";
        v8 << name_for(triangle);
        operator_lsh10;
        v48 += 24;
    }
    return 0;
}

int classify_triangle(long a, long b, long c) {
    int result;
    if (!(a > 0 && b > 0 && c > 0)) {
        return -1;
    }
    if (!(!(a + b <= c) && !(a + c <= b) && b + c > a)) {
        return -2;
    }
    if (a == b) {
        if (b == c) {
            return 3;
        }
    } else {
        if (!(a != b && b != c && a != c)) {
            return 2;
        }
        return 1;
    }
    return result;
}

long name_for(int a) {
    long result;
    if (a != 2) {
        if (a != 1) {
            if (a == 1) goto L1;
            if (a == 2) goto L2;
            if (a == 3) goto L3;
            goto L4;
        }
        return "invalid (non-positive side)";
    } else {
        return "invalid (fails inequality)";
        L1:
        return "scalene";
        L2:
        return "isosceles";
        L3:
        return "equilateral";
        L4:
        return "unknown";
    }
    return result;
}

