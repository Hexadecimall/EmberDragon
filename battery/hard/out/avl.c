#include <cstdio>
#include <cstring>
#include <vector>

using namespace std;

long insert(struct Pair2* obj, long arr, long a, long b, long c, long d, long e, long f);
long rebalance(struct Struct2* obj, long arr, long a, long b, long c);
long erase(struct Struct2* obj, long arr, long a);


static const int k = 1;

class AVL {
public:
    long value;
    long data;
    long item;
    long AVL::check(int arr, std::vector<int>& obj2) const {
        long* v26;
        long* obj;
        long v28;
        long* v30;
        long* v31;
        long v27;
        long* v25;
        long v0;
        long v24;
        if (!((arr & (1<<31)) != 0)) {
            v26 = obj;
            v28 = arr;
            this = obj2;
            if ((check(*((obj->value + (arr << 4)) + 8))) == 0) goto loc_1000009c8;
            v30 = this->data;
            v31 = (v26->value + (v28 << 4));
            if (this->data >= this->item) goto loc_1000009dc;
            v30->f4 = v31->value;
            v27 = this;
            loc_100000958:
            obj2->data = v30;
            if ((check(v26, *((v26->value + (v28 << 4)) + 12))) == 0) goto loc_1000009c8;
            v30 = v26->value;
            v31 = *((v26->value + (v28 << 4)) + 8);
            v25 = (v26->value + (v28 << 4));
            if ((*((v26->value + (v28 << 4)) + 8) & (1<<31)) != 0) goto loc_1000009ac;
            v31 = (*((v30 + (v31 << 4)) + 4) + 1);
            v25 = v25->f12;
            if ((v25->f12 & (1<<31)) != 0) goto loc_1000009b8;
            loc_100000998:
            v30 = *((v30 + (v25 << 4)) + 4);
        } else {
            loc_1000009ac:
            v31 = 1;
            v25 = v25->f12;
            if ((v25->f12 & (1<<31)) == 0) goto loc_100000998;
            loc_1000009b8:
            v30 = 0;
        }
        loc_1000009c8:
        return obj;
        loc_1000009dc:
        operator_call(&v0);
        v27 = this;
        v30 = v24;
        goto loc_100000958;
    }
    // AVL::Node& std::vector<AVL::Node>::emplace_back<AVL::Node>(AVL::Node&&)::'lambda0'()::operator_call() const
    long operator_call() {
        long v6;
        long* obj;
        long v1;
        long v2;
        long* v3;
        long v4;
        long t16;
        long v5;
        v6 = (((*(obj->item + 8) - *obj->item) >> 4) + 1);
        v1 = *obj->item;
        v2 = (*(obj->item + 8) - *obj->item);
        v3 = obj->item;
        v4 = ((*(obj->item + 8) - *obj->item) >> 4);
        if (!(((((*(obj->item + 8) - *obj->item) >> 4) + 1) >> 60) != 0)) {
            v6 = (((v3->item - v1) < 0x7ffffffffffffff0) ? ((((v3->item - v1) >> 3) >= v6) ? ((v3->item - v1) >> 3) : v6) : 0xfffffffffffffff);
            if (((((v3->item - v1) < 0x7ffffffffffffff0) ? ((((v3->item - v1) >> 3) >= v6) ? ((v3->item - v1) >> 3) : v6) : 0xfffffffffffffff) >> 60) != 0) goto loc_100000ef4;
            t16 = operator_new(v6 << 4);
            memcpy(((t16 + v2) - (v4 << 4)), v1, v2);
            v3->value = ((t16 + v2) - (v4 << 4));
            v3->data = ((t16 + v2) + 16);
            v3->item = (t16 + (v6 << 4));
            v5 = ((t16 + v2) + 16);
            if (v1 != 0) {
                operator_delete(v1);
            }
            *this->value = v5;
            return obj;
        }
        __throw_length_error();
        loc_100000ef4:
        __throw_bad_array_new_length();
    }
    // int& std::vector<int>::emplace_back<int const&>(int const&)::'lambda0'()::operator_call() const
    long operator_call() {
        long v6;
        long* obj;
        long v1;
        long v2;
        long* v3;
        long v4;
        long t23;
        long v5;
        v6 = (((*(obj->item + 8) - *obj->item) >> 2) + 1);
        v1 = *obj->item;
        v2 = (*(obj->item + 8) - *obj->item);
        v3 = obj->item;
        v4 = ((*(obj->item + 8) - *obj->item) >> 2);
        if (!(((((*(obj->item + 8) - *obj->item) >> 2) + 1) >> 62) != 0)) {
            v6 = (((v3->item - v1) < 0x7ffffffffffffffc) ? ((((v3->item - v1) >> 1) >= v6) ? ((v3->item - v1) >> 1) : v6) : 0x3fffffffffffffff);
            if (((((v3->item - v1) < 0x7ffffffffffffffc) ? ((((v3->item - v1) >> 1) >= v6) ? ((v3->item - v1) >> 1) : v6) : 0x3fffffffffffffff) >> 62) != 0) goto loc_100001160;
            t23 = operator_new(v6 << 2);
            *((t23 + v2) + 4) = *obj->data;
            memcpy(((t23 + v2) - (v4 << 2)), v1, v2);
            v3->value = ((t23 + v2) - (v4 << 2));
            v3->data = (t23 + v2);
            v3->item = (t23 + (v6 << 2));
            v5 = (t23 + v2);
            if (v1 != 0) {
                operator_delete(v1);
            }
            *this->value = v5;
            return obj;
        }
        __throw_length_error();
        loc_100001160:
        __throw_bad_array_new_length();
    }
};

struct Pair {
    int value;
    char _pad4[12];
    char data;
};
struct Struct0 {
    long value;
    long data;
    long item;
    long member;
    long value-32;
    long data-24;
    long item-16;
    long member-8;
    int value-4;
    long data0;
    int item4;
    long member8;
    long value16;
    long data24;
    long item32;
    long member40;
    long value48;
    long data56;
};
int main(int argc, char** argv) {
    long v96;
    long v97;
    long v98;
    long v99;
    long v100;
    long t0;
    long v64;
    v96 = 0x75bcd15;
    v97 = 2000;
    v98 = 0xa7c5ac5;
    v99 = 100000;
    v100 = 0x75bcd15;
    t0 = insert(&v64, ((((v100 ^ v100) ^ (v100 ^ v100)) ^ ((v100 ^ v100) ^ (v100 ^ v100))) - ((((((v100 ^ v100) ^ (v100 ^ v100)) ^ ((v100 ^ v100) ^ (v100 ^ v100))) >> 5) * v98) >> 39) * v99));
    v97--;
    v100 = (((v100 ^ v100) ^ (v100 ^ v100)) ^ ((v100 ^ v100) ^ (v100 ^ v100)));
    while (v97 != 1) {
    }
    v97 = 2000;
    v98 = 0xa7c5ac5;
    v99 = 100000;
        v96 = (((v96 ^ v96) ^ (v96 ^ v96)) ^ ((v96 ^ v96) ^ (v96 ^ v96)));
        if ((((((v96 ^ v96) ^ (v96 ^ v96)) ^ ((v96 ^ v96) ^ (v96 ^ v96))) - ((((((v96 ^ v96) ^ (v96 ^ v96)) ^ ((v96 ^ v96) ^ (v96 ^ v96))) >> 5) * v98) >> 39) * v99) >> 4) >= 3124) continue;
        v88 = (erase(&v64));
    }
    v101 = ((v90 - v91) >> 2);
    v92 = (v90 - v91);
    v96 = (check(&v64, &v40));
    if (v90 != v91) {
        v102 = v91->value;
        v93 = (v92 - 4);
        if (v92 != 4) goto loc_100000660;
        v97 = 1;
    } else {
        v97 = 1;
        v98 = 1;
        goto loc_100000840;
        loc_100000660:
        v92 = (v101 - 1);
        v94 = (v91 + 4);
        v95 = 0xf4243;
        v97 = 1;
        v102 = (v94->data0 + v102 * v95);
        v92--;
        v94 += 4;
        v97 = (((v94->data0 <= v94->value-4) ? 0 : (0 + 1)) & v97);
        while (v92 != 1) {
        }
    }
    if (v93 < 11) {
        v94 = v91;
        v98 = 1;
    } else {
        v92 = ((v93 >> 2) + 1);
        if (v93 < 252) {
            v93 = 0;
            v98 = 1;
        } else {
            v93 = (v92 & 0x7fffffffffffffc0);
            v94 = (v91 + 128);
            v95 = (v92 & 0x7fffffffffffffc0);
            v94 += 256;
            v95 -= 64;
            v96 = v94->item-16;
            v97 = v94->member-8;
            while (v95 != 64) {
            }
            v98 = (v94 ^ 1);
            if (v92 == v93) goto loc_100000840;
            if (v92 == 0) goto loc_100000820;
        }
        v91 += (v93 << 2);
        v93 -= (v92 & 0x7ffffffffffffffc);
        v94 = (v91 + ((v92 & 0x7ffffffffffffffc) << 2));
        v95 = (v92 & 0x7ffffffffffffffc);
        v93 += 4;
        while (v93 != 4) {
        }
        v98 = (v91 ^ 1);
        if (v92 == v95) {
            goto loc_100000840;
            loc_100000820:
            v94 = (v91 + (v93 << 2));
        }
    }
    v91 = 49999;
    v98 = (((v94->item4 <= v91) ? 0 : (0 + 1)) & v98);
    while (v94 != v90) {
    }
    loc_100000840:
    printf("balanced=%d ordered=%d no_low=%d remaining=%zu checksum=%lld\n", (v98 & 1));
    if (v40 != 0) {
        operator_delete();
    }
    if (v64 != 0) {
        operator_delete();
    }
    return (((v96 & v97) & v98) ^ 1);

struct Pair2 {
    char* data;
    long value;
};
struct Struct1 {
    char _pad0[8];
    int value;
    int data;
    char item;
};
long AVL::insert(int obj, int arr) {
    struct Struct1* v45;
    long v16;
    long v40;
    long tail;
    long call;
    long unrecovered;
    long target;
    if (!((arr & (1<<31)) != 0)) {
        v45 = (obj->data + (arr << 4));
        if (a >= *(obj->data + (arr << 4))) goto L2;
        *((obj->data + (arr << 4)) + 8) = (insert(v45->value));
    } else {
        v45 = *(k + 8);
        if (*(k + 8) >= *(k + 16)) goto L3;
        L1:
        obj->value = v45;
        return arr;
        L2:
        if (*(k + 8) <= *(k + 16)) return arr;
        *((obj->data + (arr << 4)) + 12) = (insert(v45->data));
    }
    goto L4;
    L3:
    operator_call(&v16);
    v45 = v40;
    goto L1;
    L4:
    ; // -> L4 (tail-call / unrecovered target)
}

struct Struct2 {
    long value;
};
struct Struct3 {
    char _pad0[4];
    int value;
    int data;
    int item;
};
long AVL::rebalance(int obj) {
    long v6;
    struct Struct3* v7;
    long v1;
    long v2;
    struct Struct3* v3;
    struct Struct3* v4;
    long v5;
    v6 = obj->value;
    v7 = (obj->value + (arr << 4));
    v1 = *((obj->value + (arr << 4)) + 8);
    if (!((*((obj->value + (arr << 4)) + 8) & (1<<31)) != 0)) {
        v2 = v7->item;
        v3 = *((v6 + (v1 << 4)) + 4);
        if ((v7->item & (1<<31)) != 0) goto L2;
        L1:
        v4 = *((v6 + (v2 << 4)) + 4);
    } else {
        v2 = v7->item;
        v3 = 0;
        if ((v7->item & (1<<31)) == 0) goto L1;
        L2:
        v4 = 0;
    }
    v7->value = (((v3 > v4) ? v3 : v4) + 1);
    if (!((arr & (1<<31)) != 0)) {
        if (!((v1 & (1<<31)) != 0)) {
            v3 = *((v6 + (v1 << 4)) + 4);
            if ((v2 & (1<<31)) != 0) goto L4;
            L3:
            v4 = *((v6 + (v2 << 4)) + 4);
        } else {
            v3 = 0;
            if ((v2 & (1<<31)) == 0) goto L3;
            L4:
            v4 = 0;
        }
        v3 -= v4;
        if (!((v3 - v4) < 2)) {
            if ((v1 & (1<<31)) != 0) goto L13;
            v2 = *((v6 + (v1 << 4)) + 8);
            v3 = (v6 + (v1 << 4));
            if ((*((v6 + (v1 << 4)) + 8) & (1<<31)) != 0) goto L11;
            v2 = v3->item;
            v4 = *((v6 + (v2 << 4)) + 4);
            if ((v3->item & (1<<31)) != 0) goto L12;
            L5:
            if (v4 >= *((v6 + (v2 << 4)) + 4)) goto L13;
            L6:
            v3->item = *((v6 + (v2 << 4)) + 8);
            *((v6 + (v2 << 4)) + 8) = v1;
            v1 = v3->data;
            v4 = (v6 + (v2 << 4));
            v5 = *((v6 + (v2 << 4)) + 8);
            if ((v3->data & (1<<31)) != 0) goto L14;
            v1 = *((v6 + (v1 << 4)) + 4);
            if ((v5 & (1<<31)) != 0) goto L15;
            L7:
            v5 = *((v6 + (v5 << 4)) + 4);
        } else {
            if (v3 > 2) goto L29;
            if ((v2 & (1<<31)) != 0) goto L18;
            v1 = *((v6 + (v2 << 4)) + 8);
            v3 = (v6 + (v2 << 4));
            if ((*((v6 + (v2 << 4)) + 8) & (1<<31)) != 0) goto L16;
            v4 = *((v6 + (v1 << 4)) + 4);
            v5 = v3->item;
            if ((v3->item & (1<<31)) != 0) goto L17;
            L8:
            if (v4 <= *((v6 + (v5 << 4)) + 4)) goto L18;
            L9:
            v3->data = *((v6 + (v1 << 4)) + 12);
            *((v6 + (v1 << 4)) + 12) = v2;
            v4 = (v6 + (v1 << 4));
            v5 = *((v6 + (v1 << 4)) + 12);
            if ((*((v6 + (v1 << 4)) + 12) & (1<<31)) != 0) goto L23;
            v2 = *((v6 + (v5 << 4)) + 4);
            v5 = v3->item;
            if ((v3->item & (1<<31)) != 0) goto L24;
            L10:
            v5 = *((v6 + (v5 << 4)) + 4);
            goto L25;
            L11:
            v2 = v3->item;
            v4 = 0;
            if ((v3->item & (1<<31)) == 0) goto L5;
            L12:
            if (v4 < 0) goto L6;
            L13:
            goto L20;
            L14:
            v1 = 0;
            if ((v5 & (1<<31)) == 0) goto L7;
            L15:
            v5 = 0;
        }
        v3->value = (((v1 > v5) ? v1 : v5) + 1);
        v1 = (((v1 > v5) ? v1 : v5) + 1);
        v3 = v4->item;
        if (!((v4->item & (1<<31)) != 0)) {
            v3 = *((v6 + (v3 << 4)) + 4);
            goto L19;
            L16:
            v4 = 0;
            v5 = v3->item;
            if ((v3->item & (1<<31)) == 0) goto L8;
            L17:
            if (v4 > 0) goto L9;
            L18:
            v1 = v2;
        } else {
            v3 = 0;
            L19:
            v4->value = (((v1 > v3) ? v1 : v3) + 1);
            v1 = v2;
            L20:
            v7->data = *((v6 + (v1 << 4)) + 12);
            *((v6 + (v1 << 4)) + 12) = arr;
            v2 = *((v6 + (v1 << 4)) + 12);
            v3 = (v6 + (v1 << 4));
            if (!((*((v6 + (v1 << 4)) + 12) & (1<<31)) != 0)) {
                v2 = v7->item;
                v4 = *((v6 + (v2 << 4)) + 4);
                if ((v7->item & (1<<31)) != 0) goto L22;
                L21:
                v5 = *((v6 + (v2 << 4)) + 4);
            } else {
                v2 = v7->item;
                v4 = 0;
                if ((v7->item & (1<<31)) == 0) goto L21;
                L22:
                v5 = 0;
            }
            v7->value = (((v4 > v5) ? v4 : v5) + 1);
            v7 = v3->data;
            v2 = v1;
            v4 = (((v4 > v5) ? v4 : v5) + 1);
            if (!((v3->data & (1<<31)) != 0)) {
                v7 = *((v6 + (v7 << 4)) + 4);
            } else {
                v7 = 0;
            }
            v7 = ((v7 > v4) ? v7 : v4);
            goto L28;
            L23:
            v2 = 0;
            v5 = v3->item;
            if ((v3->item & (1<<31)) == 0) goto L10;
            L24:
            v5 = 0;
            L25:
            v3->value = (((v2 > v5) ? v2 : v5) + 1);
            v2 = (((v2 > v5) ? v2 : v5) + 1);
            v3 = v4->data;
            if (!((v4->data & (1<<31)) != 0)) {
                v3 = *((v6 + (v3 << 4)) + 4);
            } else {
                v3 = 0;
            }
            v4->value = (((v3 > v2) ? v3 : v2) + 1);
        }
        v7->item = *((v6 + (v1 << 4)) + 8);
        *((v6 + (v1 << 4)) + 8) = arr;
        v2 = *((v6 + (v1 << 4)) + 8);
        v3 = (v6 + (v1 << 4));
        v4 = v7->data;
        if (!((v7->data & (1<<31)) != 0)) {
            v4 = *((v6 + (v4 << 4)) + 4);
            if ((v2 & (1<<31)) != 0) goto L27;
            L26:
            v5 = *((v6 + (v2 << 4)) + 4);
        } else {
            v4 = 0;
            if ((v2 & (1<<31)) == 0) goto L26;
            L27:
            v5 = 0;
        }
        v7->value = (((v4 > v5) ? v4 : v5) + 1);
        v7 = v3->item;
        v2 = v1;
        v4 = (((v4 > v5) ? v4 : v5) + 1);
        if (!((v3->item & (1<<31)) != 0)) {
            v7 = *((v6 + (v7 << 4)) + 4);
        } else {
            v7 = 0;
        }
        v7 = ((v4 > v7) ? v4 : v7);
        L28:
        *((v6 + (v2 << 4)) + 4) = (v7 + 1);
    }
    L29:
    return arr;
}

struct Struct4 {
    int value;
    char _pad4[4];
    int data;
    int item;
};
long AVL::erase(int arr, int a) {
    struct Struct4* v5;
    long v6;
    long v1;
    long v2;
    long tail;
    long call;
    long unrecovered;
    long target;
    if (!((arr & (1<<31)) != 0)) {
        v5 = (obj->value + (arr << 4));
        v6 = obj->value;
        if (a >= *(obj->value + (arr << 4))) goto L2;
        *((obj->value + (arr << 4)) + 8) = (erase(v5->data));
    } else {
        do {
            L1:
            return arr;
            L2:
            if (a > *(obj->value + (arr << 4))) {
                goto L3;
            }
            v1 = v5->data;
        } while ((v5->data & (1<<31)) != 0);
        if ((arr & (1<<31)) != 0) goto L4;
        v2 = arr;
        v1 = v2;
        v2 = *((v6 + (v2 << 4)) + 8);
        while ((*((v6 + (v2 << 4)) + 8) & (1<<31)) == 0) {
        }
        v5->value = *(v6 + (v1 << 4));
        L3:
        *((obj->value + (arr << 4)) + 12) = (erase());
    }
    goto L5;
    L4:
    goto L1;
    L5:
    ; // -> L5 (tail-call / unrecovered target)
}

