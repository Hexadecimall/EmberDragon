#include <cstring>
#include <string>

using namespace std;

long matchClass(std::string* obj, struct Struct0* obj2, long a);

static const unsigned char data[64] = {
    0xa4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0xaa, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0xa4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0xb5, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0xc0, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0xc4, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0xc0, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0xc8, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00 
};

class glob {
public:
    char* data;
    long size;
    char _pad16[7];
    char cap;
    long glob::recMatch(std::string const& n, unsigned long obj2, std::string const& a, unsigned long b) {
        long v20;
        long* obj;
        long v10;
        long v14;
        long v12;
        long v13;
        long v15;
        long v16;
        long v17;
        long v18;
        long v19;
        long v9;
        v20 = obj.size();
        v10 = a;
        this = obj2;
        v14 = (obj.size());
        if (n < (obj.size())) {
            v12 = obj;
            v13 = ((this->cap < 0) ? this->size : this->cap);
            v15 = this->cap;
            v16 = ((v20 < 0) ? obj.c_str() : obj);
            v17 = this->cap;
            v18 = this->size;
            v19 = ((this->cap < 0) ? this->data : this);
            goto loc_100000988;
                loc_100000978:
                v9 = (n + 1);
                loc_10000097c:
                v10++;
                if (n >= v14) goto loc_1000009f0;
                loc_100000988:
                v20 = *(v16 + n);
                if (*(v16 + n) == 63) continue;
            }
        }
        v15 = this->cap;
        v17 = this->cap;
        v18 = this->size;
        loc_1000009f0:
        goto loc_100000a4c;
        loc_100000a08:
        if (n < v14) {
            while (!(*(v16 + n) != 42)) {
                v9 = (n + 1);
                if (v14 != (n + 1)) continue;
            }
        } else {
            if (n == v14) {
            } else {
                if (v10 < v13) goto loc_100000a6c;
                loc_100000a48:
            }
        }
        loc_100000a4c:
        return obj;
        loc_100000a6c:
        v14 = n;
        while (!((recMatch(v12, this, v10)) != 0)) {
            v9 = v14;
            v10++;
            if ((v10 + 1) < v13) continue;
        }
        goto loc_100000a4c;
        if (v20 == 42) goto loc_100000a08;
        if (v10 >= v13) goto loc_100000a48;
        if (*(v19 + v10) == v20) goto loc_100000978;
        goto loc_100000a48;
        if (v10 >= v13) goto loc_100000a48;
        v8 = n;
        if (!(matchClass(v12, &v8, *(v19 + v10)))) goto loc_100000a48;
        v9 = v8;
        goto loc_10000097c;
    }
};

int main(int argc, char** argv) {
    long v99;
    long v101;
    long t0;
    long v93;
    long* v100;
    long v94;
    char v87;
    long v95;
    long t3;
    char v63;
    long v92;
    long v64;
    long v40;
    long v102;
    long v48;
    long v103;
    long v104;
    long v90;
    long v91;
    v99 = 0;
    v101 = 0x7ffffffffffffff6;
        t0 = strlen(data[v99]);
        v93 = data[v99];
        v100 = (data + v99);
        if (t0 >= v101) goto loc_1000008d4;
        v94 = argc;
        if (argc < 22) {
            v87 = v94;
        } else {
            ((((v94 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v94 & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000;
            v95 = (operator_new((((v94 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v94 & 0x7ffffffffffffff8) + 8)));
        }
        memmove(v95, v93, v94);
        *(v95 + v94) = 0;
        t3 = strlen(v100->f8);
        v93 = v100->f8;
        if (t3 >= v101) goto loc_1000008d8;
        v94 = argc;
        if (argc < 22) {
            v63 = v94;
        } else {
            ((((v94 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v94 & 0x7ffffffffffffff8) + 8)) | 0x8000000000000000;
            v95 = (operator_new((((v94 & 0x7ffffffffffffff8) + 8) == 24) ? 25 : ((v94 & 0x7ffffffffffffff8) + 8)));
        }
        memmove(v95, v93, v94);
        *(v95 + v94) = 0;
        v92 = v63;
        v94 = v87;
        v95 = (recMatch(&v64, 0, &v40, 0));
        v102 = ((v63 < 0) ? v48 : v63);
        if (((v63 >= 0) ? v48 : v63) == 0) {
            v103 = 0;
            v104 = &v40;
            v90 = v40;
            v91 = ((v92 < 0) ? v40 : &v40);
            v93 = ((v94 < 0) ? v94 : &v64);
            v95 = -1;
            v100 = ((v94 < 0) ? v90 : v94);
            v101 = 0;
            goto L2;
                    L1:
                    if (v101 >= v102) goto loc_1000007e8;
                    L2:
                    if (v103 >= v100) goto loc_10000077c;
                    v104 = *(v93 + v103);
                    if (*(v93 + v103) == 91) goto loc_10000073c;
                } while (v104 == 63);
                if (v104 == 42) {
                    v104 = *(v93 + v103);
                    while (!(*(v93 + v103) != 42)) {
                        v103++;
                        if (v100 != (v103 + 1)) continue;
                    }
                }
                v90 = *(v91 + v101);
            } while (v104 == *(v91 + v101));
            loc_10000077c:
            if (v95 == 1) goto loc_100000848;
            v103 = v95;
            v96++;
            v101 = (v96 + 1);
            goto loc_1000006f4;
            loc_1000007a8:
            if (v95 == 1) goto loc_100000878;
            v103 = v95;
            v96++;
            v101 = (v96 + 1);
            loc_1000007bc:
            v91 = v99;
            v92 = v97;
            v97 = v98;
            v98 = v94;
            v99 = v32;
            goto loc_1000006f4;
        v103 = 0;
        v104 = v72;
        goto loc_1000007f8;
        loc_1000007e0:
        v93 = 1;
        goto loc_10000084c;
        loc_1000007e8:
        v95 = v28;
        v101 = 0x7ffffffffffffff6;
        loc_1000007f8:
        v104 = ((v94 < 0) ? v104 : v94);
        v90 = v94;
        if (v103 < ((v94 < 0) ? v104 : v94)) {
            v90 = ((v90 < 0) ? v64 : &v64);
            while (!(*(v90 + v103) != 42)) {
                v103++;
                if (v104 != (v103 + 1)) continue;
            }
        }
        v93 = ((v103 != v104) ? 0 : (0 + 1));
        if (!((v92 & (1<<31)) == 0)) {
            goto loc_100000860;
            loc_100000848:
            v93 = 0;
            loc_10000084c:
            v94 = v16;
            v95 = v28;
            v101 = 0x7ffffffffffffff6;
            if (!((v92 & (1<<31)) == 0)) {
                loc_100000860:
                operator_delete(v40);
            }
        }
        loc_100000868:
        if ((v94 & (1<<7)) == 0) continue;
        operator_delete(v64);
    loc_1000008d4:
    __throw_length_error();
    loc_1000008d8:
    __builtin_trap();
    v89 = (__throw_length_error());
    v93 = argc;
    if (!((v87 & (1<<31)) == 0)) {
        operator_delete(v64);
    }
    _Unwind_Resume(v93);
    goto loc_1000007e0;
    loc_10000073c:
    v88 = v103;
    v94 = v98;
    v97 = v92;
    v98 = v97;
    v99 = v91;
    if ((matchClass(&v64, &v88, *(v91 + v101))) == 0) goto loc_1000007a8;
    v103 = v88;
    v101++;
    goto loc_1000007bc;
    loc_100000878:
    v93 = 0;
    v94 = v16;
    v95 = v28;
    v97 = v98;
    v98 = v94;
    v99 = v32;
    v101 = 0x7ffffffffffffff6;
    if ((v97 & (1<<31)) == 0) goto loc_100000868;
    goto loc_100000860;

std::string {
    char* data;
    long size;
    char _pad16[7];
    char cap;
};
struct Struct0 {
    long count;
};
long glob::matchClass(std::string const& obj, unsigned long& obj2, char a) {
    long v7;
    long v8;
    long v0;
    long v1;
    long v2;
    long v3;
    long v4;
    long v5;
    long v6;
    long ccmp;
    long chained;
    long compare;
    obj2->count++;
    v7 = obj2->count;
    v8 = (obj2->count + 1);
    v0 = obj.size();
    v1 = obj.size();
    v2 = obj.size();
    if (!((obj2->count + 1) >= (obj.size()))) {
        if (*(((v2 >= 0) ? obj.c_str() : obj) + v8) != 33) {
            obj2->count = (v7 + 2);
            v7 = 1;
            v8 = (v7 + 2);
            v0 = obj.size();
            v1 = obj.size();
            v2 = obj.size();
            goto L1;
        }
    } else {
        v7 = 0;
    }
    L1:
    v2 = ((v2 >= 0) ? 0 : (0 + 1));
    v3 = ((v2 < 0) ? v1 : v0);
    v4 = v2;
    if (v8 < ((v2 < 0) ? v1 : v0)) {
        v0 = *(((v4 < 0) ? obj.c_str() : obj) + v8);
        v1 = (v8 + 1);
        v2 = ((v4 < 0) ? obj.c_str() : obj);
        v4 = (v8 + 2);
        if (!((v8 + 2) >= v3)) {
            if (!(*(v2 + v1) != 45)) {
                v2 = *(v2 + v4);
                if (*(v2 + v4) != 93) goto L6;
            }
        }
        v0 = ((a != v0) ? 0 : (0 + 1));
        L2:
        obj2->count = v1;
        v2 = ((obj.size() >= 0) ? 0 : (0 + 1));
        v3 = (obj.size() & 255);
        v4 = obj.size();
        if (v1 >= ((obj.size() < 0) ? obj.size() : (obj.size() & 255))) goto L4;
        v8 = obj.c_str();
        v5 = ((v2 != 0) ? obj.c_str() : obj);
        while (*(((v2 == 0) ? obj.c_str() : obj) + v1) == 93) {
            v8 = (v1 + 1);
            v2 = *(((v2 != 0) ? v8 : obj) + v1);
            v6 = (v1 + 2);
            if (!((v1 + 2) >= ((v2 != 0) ? v4 : v3))) {
                if (!(*(v5 + v8) != 45)) {
                    v3 = *(v5 + v6);
                    if (*(v5 + v6) != 93) goto L7;
                }
            }
            v0 = (((a != v2) ? 0 : (0 + 1)) | v0);
            L3:
            obj2->count = v8;
            v1 = v8;
            v2 = ((obj.size() >= 0) ? 0 : (0 + 1));
            v3 = (obj.size() & 255);
            v4 = obj.size();
            if (v8 < ((obj.size() < 0) ? obj.size() : (obj.size() & 255))) continue;
        }
    }
    v0 = 0;
    if (v8 >= v3) {
        goto L5;
        L4:
        v8 = v1;
        v0 &= 1;
        if (v8 >= ((v2 != 0) ? v4 : v3)) goto L5;
    }
    if (*(((v2 == 0) ? obj.c_str() : obj) + v8) != 93) {
        obj2->count = (v8 + 1);
    }
    L5:
    return ((v7 == v0) ? 0 : (0 + 1));
    L6:
    /* ccmp a, v2  (chained >= compare) */
    v0 = ((a > v0) ? 0 : (0 + 1));
    v1 = (v8 + 3);
    goto L2;
    L7:
    v8 = (v1 + 3);
    v0 = ((a < v2) ? v0 : (((a > v3) ? 0 : (0 + 1)) | v0));
    goto L3;

