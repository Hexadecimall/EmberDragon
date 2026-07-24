#include <cstring>
#include <string>
#include <vector>

using namespace std;

long encode(long a);
int decode(long a, long b);
int decodeChar(long a);

static const long data_init[5] = {
    4503599627395552, 4503599627395555, 4503599627395560, 4503599627395565, 4503599627395570 
};
static const int k = 25079;

class Record0 {
public:
    long value;
    long data;
    long item;
    // unsigned char& std::vector<unsigned char>::emplace_back<unsigned char>(unsigned char&&)::'lambda'()::operator_call() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = (*obj->item + 1);
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // unsigned char& std::vector<unsigned char>::emplace_back<unsigned char>(unsigned char&&)::'lambda0'()::operator_call() const
    long operator_call() {
        long* obj;
        long t93;
        obj = this;
        t93 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = t93;
        return t93;
    }
    // std::vector<unsigned char>& std::vector<std::vector<unsigned char>>::emplace_back<std::vector<unsigned char>>(std::vector<unsigned char>&&)::'lambda'()::operator_call() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = (*obj->item + 24);
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // std::vector<unsigned char>& std::vector<std::vector<unsigned char>>::emplace_back<std::vector<unsigned char>>(std::vector<unsigned char>&&)::'lambda0'()::operator_call() const
    long operator_call() {
        long* obj;
        long t95;
        obj = this;
        t95 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = t95;
        return t95;
    }
    // std::vector<unsigned char>& std::vector<std::vector<unsigned char>>::emplace_back<std::vector<unsigned char> const&>(std::vector<unsigned char> const&)::'lambda'()::operator_call() const
    long operator_call() {
        long* obj;
        obj = this;
        *obj->item = (*obj->item + 24);
        return (__emplace_back_assume_capacity(this->data, this->value));
    }
    // std::vector<unsigned char>& std::vector<std::vector<unsigned char>>::emplace_back<std::vector<unsigned char> const&>(std::vector<unsigned char> const&)::'lambda0'()::operator_call() const
    long operator_call() {
        long* obj;
        long t97;
        obj = this;
        t97 = __emplace_back_slow_path(this->item, this->data);
        *obj->value = t97;
        return t97;
    }
};

long b64::encode(std::vector<unsigned char> const& a) {
    long v105;
    long sum;
    long n;
    long i;
    int v8;
    int v12;
    int v44;
    long v32;
    int v28;
    int v4;
    int v24;
    basic_string(v105);
    reserve(v105, ((((a.size()) + 2) / 3) << 2));
    sum = 0;
    n = ((a.size()) / 3);
    i = 0;
    while (i < n) {
        v8 = (a[sum])->f0;
        v12 = (((a[(sum + 1)])->f0 << 8) | v8);
        v44 = (v12 | (a[(sum + 2)])->f0);
        sum += 3;
        push_back(v105, *(k + ((v44 >> 18) & 63)));
        push_back(v105, *(k + ((v44 >> 12) & 63)));
        push_back(v105, *(k + ((v44 >> 6) & 63)));
        push_back(v105, *(k + (v44 & 63)));
        i++;
    }
    v32 = ((a.size()) - sum);
    if (v32 == 1) {
        v28 = ((a[sum])->f0 << 16);
        push_back(v105, *(k + ((v28 >> 18) & 63)));
        push_back(v105, *(k + ((v28 >> 12) & 63)));
        push_back(v105, 61);
        push_back(v105, 61);
    } else {
        if (v32 == 2) {
            v4 = (a[sum])->f0;
            v24 = (((a[(sum + 1)])->f0 << 8) | v4);
            push_back(v105, *(k + ((v24 >> 18) & 63)));
            push_back(v105, *(k + ((v24 >> 12) & 63)));
            push_back(v105, *(k + ((v24 >> 6) & 63)));
            push_back(v105, 61);
        }
    }
}

int b64::decode(std::string const& a, std::vector<unsigned char>& b) {
    long t27;
    char v87;
    long sum;
    long v24;
    int i;
    char v47;
    long v16;
    int v88;
    int decodechar;
    int v36;
    int v92;
    int v96;
    int v100;
    char v35;
    char v34;
    char v33;
    int v12;
    b.clear();
    t27 = a.size();
    if (!((t27 - ((t27 / 4) * 4)) == 0)) {
        v87 = ((0 & 1) & 1);
    } else {
        sum = 0;
        v24 = sum;
        while (v24 < (a.size())) {
            i = 0;
            while (i < 4) {
                v47 = (a[(sum + i)])->f0;
                if (v47 == 61) {
                    v16 = (sum + 4);
                    if (v16 == (a.size())) {
                        if (i >= 2) goto L1;
                    }
                    v87 = ((0 & 1) & 1);
                    goto L2;
                    L1:
                    *(&v88 + (i << 2)) = 0;
                    0++;
                } else {
                    if (0 != 0) {
                        v87 = ((0 & 1) & 1);
                        goto L2;
                    }
                    decodechar = (decodeChar(v47));
                    if (!((decodechar & (1<<31)) == 0)) {
                        v87 = ((0 & 1) & 1);
                        goto L2;
                    }
                    *(&v88 + (i << 2)) = decodechar;
                }
                i++;
            }
            v36 = ((((v92 << 12) | v88) | v96) | v100);
            v35 = ((v36 >> 16) & 255);
            push_back(b, &v35);
            if (0 < 2) {
                v34 = ((v36 >> 8) & 255);
                push_back(b, &v34);
            }
            if (0 < 1) {
                v33 = v36;
                push_back(b, &v33);
            }
            sum += 4;
        }
        v87 = ((1 & 1) & 1);
    }
    L2:
    v12 = v87;
    return (v12 & 1);
}

int b64::decodeChar(char a) {
    int result;
    if (a >= 65) {
        if (a <= 90) {
            return (a - 65);
        }
    } else {
        if (a >= 97) {
            if (a <= 122) {
                return ((a - 97) + 26);
            }
        } else {
            if (a >= 48) {
                if (a <= 57) {
                    return ((a - 48) + 52);
                }
            } else {
                if (a == 43) {
                    result = 62;
                } else {
                    if (a == 47) {
                        result = 63;
                    } else {
                        result = -1;
                    }
                }
            }
        }
    }
}

struct Struct0 {
    long value;
};
struct Struct1 {
    long value;
    long data;
    long item;
    long member;
    char _pad32[48];
    long value80;
    long data88;
    char _pad96[32];
    long item128;
    long member136;
    char _pad144[32];
    long value176;
    long data184;
    char _pad192[32];
    long item224;
    long member232;
    char _pad240[32];
    long value272;
};
int main(int argc, char** argv) {
    struct Struct1* obj2;
    long v240;
    int ret;
    long data[5];
    long v80;
    long v104;
    long v144;
    long v192;
    long v336;
    long v288;
    int i;
    char v283;
    int v272;
    long v256;
    long v248;
    int v36;
    long v216;
    char v191;
    int v20;
    long v168;
    long v160;
    int j;
    long v128;
    long v120;
    struct Struct0* obj;
    long v96;
    int v4;
    int v68;
    long __addr0;
    long __addr77;
    obj2 = &v240;
    ret = 0;
    data = &v80;
    vector(&v80);
    vector(&v104);
    push_back(data, (&v104));
    obj2->item224 = &__addr77;
    obj2->member232 = 1;
    vector(&v144, obj2->item224, obj2->member232);
    push_back(&v80, &v144);
    obj2->value176 = &__addr77;
    obj2->data184 = 2;
    vector(&v192, obj2->value176, obj2->data184);
    push_back(&v80, &v192);
    obj2->item128 = &__addr77;
    obj2->member136 = 3;
    vector(&v240, obj2->item128, obj2->member136);
    push_back(&v80, &v240);
    obj2->value80 = &__addr0;
    obj2->data88 = 5;
    vector(&v336, obj2->value80, obj2->data88);
    push_back(&v80, &v336);
    vector(&v288);
    i = 0;
    while (i < 130) {
        v283 = (((i * 37) + 11) & 255);
        push_back(&v288, &v283);
        i++;
    }
    push_back(&v80, &v288);
    v272 = 0x811c9dc5;
    obj2->member = &v80;
    obj2->item = (begin(obj2->member));
    obj2->data = (end(obj2->member));
    while (v256 != v248) {
        obj2->value = (operator_mul(&v256));
        encode(obj2->value);
        vector(&v192);
        v36 = (decode(&v216, (&v192)));
        v191 = v36;
        if (v191) {
            v20 = (v192 == obj2->value);
            if (v20) {
                0++;
            }
        }
        v168 = (begin((&v216)));
        v160 = (end((&v216)));
        while (v168 != v160) {
            v272 *= 0x1000193;
            operator_inc(&v168);
        }
        operator_inc(&v256);
    }
    j = 0;
    memcpy(data, data_init, 40);
    vector(&v128);
    v120 = (data);
    obj = v120;
    v104 = (v120 + 40);
    while (obj != v104) {
        v96 = obj->value;
        std::string v72 = v96;
        v4 = (decode(&v72, &v128));
        if (!((v4) != 0)) {
            j++;
        }
        obj += 8;
    }
    v68 = ((0 * 10) + j);
    ret = ((v272 ^ v68) & 127);
    return ret;
}

