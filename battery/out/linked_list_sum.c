#include <cstdio>
#include <cstdlib>

long push_front(long a, int b);
int length(long a);
long sum(long a);


struct Struct0 {
    char _pad0[8];
    long value;
};
int main(int argc, char** argv) {
    struct Struct0* obj;
    int i;
    int len;
    long value2;
    obj = 0;
    i = 1;
    while (i <= 6) {
        obj = push_front(obj, i * 10);
        i++;
    }
    len = length(obj);
    printf("len=%d sum=%ld\n", len, sum(obj));
    while (obj != 0) {
        value2 = obj->value;
        free(obj);
        obj = value2;
    }
    return 0;
}

struct Pair {
    int value;
    char _pad4[4];
    long data;
};
long push_front(long a, int b) {
    struct Pair* buf;
    buf = malloc(16);
    buf->value = b;
    buf->data = a;
    return buf;
}

int length(long a) {
    int result;
    struct Struct0* obj;
    result = 0;
    obj = a;
    while (obj != 0) {
        result++;
        obj = obj->value;
    }
    return result;
}

long sum(long a) {
    struct Pair* obj;
    sum = 0;
    obj = a;
    while (obj != 0) {
        sum += obj->value;
        obj = obj->data;
    }
    return sum;
}

