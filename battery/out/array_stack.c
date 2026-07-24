#include <iostream>

using namespace std;

void init(struct Struct0* obj2);
long push(struct Struct0* obj, int a);
long pop(struct Struct0* obj2, int* obj3);


static const char str[] = "\n";

int main(int argc, char** argv) {
    char buf[64];
    int i;
    long sum;
    int v4;
    int top;
    init(buf);
    i = 0;
    while (i < 12) {
        if (!(push(buf, i * i) != 0)) {
            cout << "full at " << i << str;
        }
        i++;
    }
    sum = 0;
    while (pop(buf, &v4)) {
        sum += v4;
    }
    cout << "drained sum=" << sum << " top=" << top << str;
    return 0;
}

struct Struct0 {
    char _pad0[32];
    int value;
};
void init(Stack& obj2) {
    struct Struct0* obj;
    obj = obj2;
    obj->value = 0;
    return;
}

long push(Stack& obj, int a) {
struct Struct0* buf;
char v31;
    buf = obj;
    if (buf->value >= 8) {
        v31 = 0 & 1 & 1;
        return (v31 & 1);
    } else {
        buf->value++;
        buf[buf->value] = a;
        v31 = 1 & 1 & 1;
        return (v31 & 1);
    }
    return (v31 & 1);
}

long pop(Stack& obj2, int& obj3) {
struct Struct0* buf;
char v31;
    buf = obj2;
    if (buf->value == 0) {
        v31 = 0 & 1 & 1;
        return (v31 & 1);
    } else {
        buf->value--;
        *obj3 = buf[buf->value - 1];
        v31 = 1 & 1 & 1;
        return (v31 & 1);
    }
    return (v31 & 1);
}

