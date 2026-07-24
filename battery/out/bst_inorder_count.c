#include <iostream>

using namespace std;

long insert(struct Struct0* obj3, int a);
void inorder(struct Struct0* obj4, int* obj5, long* obj6);


static const char str[] = "\n";

int main(int argc, char** argv) {
    long sum2;
    int* obj;
    char v64;
    long v24;
    int v20;
    char __addr0[64];
    // neon:  ldr    value0, [v105, #0x0]  // = 0x320000001e0000004600000014000000
    // neon:  ldr    value0, [v105, #0x10]  // = 0x280000003c000000500000000a000000
    sum2 = 0;
    obj = &v64;
    v24 = &v64 + 32;
    while (obj != v24) {
        v20 = *obj;
        sum2 = insert(sum2, v20);
        obj += 4;
    }
    inorder(sum2, __addr0, __addr0);
    cout << "nodes=" << 0 << " sum=" << 0 << str;
    return 0;
}

struct Struct0 {
    int value;
    char _pad4[4];
    long data;
    long item;
};
long insert(TNode* obj3, int a) {
    struct Struct0* obj2;
    struct Struct0* obj;
    long result;
    obj2 = obj3;
    if (obj2 == 0) {
        obj = operator_new(24);
        obj->value = a;
        obj->item = 0;
        obj->data = 0;
        return obj;
    } else {
        if (a < obj2->value) {
            obj2->data = insert(obj2->data, a);
        } else {
            obj2->item = insert(obj2->item, a);
        }
        return obj2;
    }
    return result;
}

void inorder(TNode const* obj4, int& obj5, long& obj6) {
    struct Struct0* obj2;
    obj2 = obj4;
    if (obj2 == 0) {
        return;
    } else {
        inorder(obj2->data, obj5, obj6);
        *obj5 = *obj5 + 1;
        *obj6 = *obj6 + obj2->value;
        inorder(obj2->item, obj5, obj6);
        return;
    }
    return;
}

