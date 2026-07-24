#include <cstdio>
#include <cstring>
#include <vector>

using namespace std;

long kahn(struct Pair4* obj, struct Struct10* obj2, long a, long b, long c);
long run(struct Struct12* obj, long obj2, long n);
long is_valid_topo(struct Pair10* obj, struct Pair10* obj2);
long DfsTopo_dtor(struct Struct15* obj);
long DAG_dtor(struct Struct17* obj);
long visit(struct Pair10* obj, long arr, long a, long b, long c);

const char* s_main = "main";
const char* s_toposort_cpp = "toposort.cpp";

class Record0 {
public:
    long value;
    // int& std::vector<int>::emplace_back<int const&>(int const&)::'lambda0'()::operator_call() const
    long operator_call() {
        long v6;
        long* obj;
        long v1;
        long v2;
        long* v3;
        long v4;
        long t94;
        long v5;
        v6 = (((*(obj->f16 + 8) - *obj->f16) >> 2) + 1);
        v1 = *obj->f16;
        v2 = (*(obj->f16 + 8) - *obj->f16);
        v3 = obj->f16;
        v4 = ((*(obj->f16 + 8) - *obj->f16) >> 2);
        if (!(((((*(obj->f16 + 8) - *obj->f16) >> 2) + 1) >> 62) != 0)) {
            v6 = (((v3->f16 - v1) < 0x7ffffffffffffffc) ? ((((v3->f16 - v1) >> 1) >= v6) ? ((v3->f16 - v1) >> 1) : v6) : 0x3fffffffffffffff);
            if (((((v3->f16 - v1) < 0x7ffffffffffffffc) ? ((((v3->f16 - v1) >> 1) >= v6) ? ((v3->f16 - v1) >> 1) : v6) : 0x3fffffffffffffff) >> 62) != 0) goto loc_100001af0;
            t94 = operator_new(v6 << 2);
            *((t94 + v2) + 4) = *obj->f8;
            memcpy(((t94 + v2) - (v4 << 2)), v1, v2);
            v3->value = ((t94 + v2) - (v4 << 2));
            v3->f8 = (t94 + v2);
            v3->f16 = (t94 + (v6 << 2));
            v5 = (t94 + v2);
            if (v1 != 0) {
                operator_delete(v1);
            }
            *this->value = v5;
            return obj;
        }
        __throw_length_error();
        loc_100001af0:
        __throw_bad_array_new_length();
    }
};

struct Struct0 {
    long value;
    long data;
    long item;
    long member;
    long value32;
    long data40;
    long item48;
    long member56;
    long value64;
    long data72;
    long item80;
    long member88;
};
struct Struct1 {
    long value;
    long data;
    int item;
};
struct Struct2 {
    long value;
    long data;
    long item;
    long member;
    long value32;
    long data40;
    long item48;
    long member56;
    long value64;
};
struct Struct3 {
    int value;
    int data;
    int item;
};
struct Pair {
    long value;
    int data;
};
struct Pair2 {
    int value;
    int data;
};
struct Struct4 {
    int value;
};
struct Buffer {
    char _pad0[32];
    long value;
    long size;
    char _pad48[8];
    long data;
    long item;
    char _pad72[8];
    long member;
    long value88;
    char _pad96[8];
    long data104;
    long item112;
    char _pad120[8];
    long member128;
    long value136;
    char _pad144[8];
    long data152;
    long item160;
};
struct Struct5 {
    char _pad0[16];
    int count;
};
struct Struct6 {
    int value;
    char _pad4[24];
    int data;
    long item;
    char _pad40[16];
    long member;
    char _pad64[16];
    long value80;
    char _pad88[16];
    long data104;
    char _pad112[16];
    long item128;
    char _pad136[16];
    long member152;
};
struct Struct7 {
    long value;
    int data;
    int item;
    int member;
    int value12;
    int data16;
    int item20;
    int member24;
    int value28;
};
struct Pair3 {
    long value;
    int data;
};
int main(int argc, char** argv) {
    long v160;
    struct Struct0* t0;
    struct Struct1* t1;
    struct Struct5* obj2;
    struct Pair2* t2;
    struct Pair2* t3;
    long v104;
    long v24;
    struct Buffer* obj4;
    struct Pair3* v297;
    struct Struct6* v293;
    long v296;
    long v298;
    struct Struct4* obj3;
    long t6;
    long t8;
    long v208;
    struct Struct7* v295;
    long v240;
    long v290;
    long* v291;
    long v292;
    struct Struct2* t13;
    struct Struct3* t14;
    long obj;
    long* t15;
    long* t16;
    long* t17;
    struct Pair* t18;
    long v80;
    long v56;
    long* v294;
    long v112;
    long v184;
    long v264;
    v160 = 0;
    t0 = operator_new(192);
    t0->value = t0;
    t0->data = t0;
    t0->item = t0;
    t0->member = t0;
    t0->value32 = t0;
    t0->data40 = t0;
    t0->item48 = t0;
    t0->member56 = t0;
    t0->value64 = t0;
    t0->data72 = t0;
    t0->item80 = t0;
    t0->member88 = t0;
    t1 = operator_new(32);
    t1->value = t1;
    t1->data = t1;
    obj2 = t1;
    t2 = operator_new(4);
    t2->data = 3;
    t0->data = t2;
    t0->item = t2;
    t0->value = t2;
    t1->item = 1;
    t3 = operator_new(8);
    t3->value = t2->value;
    t3->data = 4;
    t0->value = t3;
    t0->data = (t3 + 8);
    t0->item = (t3 + 8);
    operator_delete(t2);
    t0->data = (t3 + 8);
    obj2->count++;
    v104 = 3;
    v24 = obj4->value;
    v297 = obj4->value;
    v293 = obj4;
    v296 = 3;
    if (obj4->value < obj4->size) {
        v297->data = 3;
        L1:
        v293->item = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 4;
        v24 = obj4->data;
        v297 = obj4->data;
        v298 = 4;
        v293 = obj4;
        if (obj4->data >= obj4->item) goto loc_100000cc8;
        v297->data = v298;
        L2:
        v293->member = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 7;
        v24 = obj4->data;
        v297 = obj4->data;
        v298 = 7;
        v293 = obj4;
        if (obj4->data >= obj4->item) goto loc_100000cd8;
        v297->data = v298;
        L3:
        v293->member = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 5;
        v24 = obj4->member;
        v297 = obj4->member;
        v298 = 5;
        v293 = obj4;
        if (obj4->member >= obj4->value88) goto loc_100000ce8;
        v297->data = v298;
        L4:
        v293->value80 = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 6;
        v24 = obj4->member;
        v297 = obj4->member;
        v298 = 6;
        v293 = obj4;
        if (obj4->member >= obj4->value88) goto loc_100000cf8;
        v297->data = v298;
        L5:
        v293->value80 = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 6;
        v24 = obj4->data104;
        v297 = obj4->data104;
        v298 = 6;
        v293 = obj4;
        if (obj4->data104 >= obj4->item112) goto loc_100000d08;
        v297->data = v298;
        L6:
        v293->data104 = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 7;
        v24 = obj4->member128;
        v297 = obj4->member128;
        v298 = 7;
        v293 = obj4;
        if (obj4->member128 >= obj4->value136) goto loc_100000d18;
        v297->data = v298;
        L7:
        v293->item128 = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v104 = 7;
        v24 = obj4->data152;
        v297 = obj4->data152;
        v298 = 7;
        v293 = obj4;
        if (obj4->data152 >= obj4->item160) goto loc_100000d28;
        v297->data = v298;
        L8:
        v293->member152 = v297;
        *(obj2 + (v104 << 2)) = (*(obj2 + (v104 << 2)) + 1);
        v297 = ((((v104 - obj2) >> 3) * 0xaaaaaaaaaaaaaaab) << 32);
        v298 = (((v104 - obj2) >> 3) * 0xaaaaaaaaaaaaaaab);
        v293 = (kahn(&obj4, &obj3));
        if (!(((((v104 - obj2) >> 3) * 0xaaaaaaaaaaaaaaab) << 32) == 0)) {
            v298 >>= 62;
            v296 = v298;
            if ((v298 >> 62) != 0) goto loc_100000e04;
            t6 = operator_new(v297 >> 30);
            v160 = t6;
            t6 + (v296 << 2);
            bzero(v297 >> 30);
        }
        t8 = run(&(&obj4), &v208);
        v297 = (v293 & t8);
        if (!(v293 & t8)) goto loc_100000d38;
        v297 -= v298;
        if ((v297 - v298) != 32) goto loc_100000d48;
        if ((v297 - v298) != 32) goto loc_100000d58;
        if ((is_valid_topo(&obj4, &obj3)) == 0) goto loc_100000d68;
        if (!(is_valid_topo(&obj4, &v208))) goto loc_100000d78;
        v295 = obj3;
        if (obj3->value == 0) {
            if (v295->item == 1) {
                if (v295->member == 2) {
                    if (v295->value12 == 3) {
                        if (v295->data16 == 4) {
                            if (v295->item20 == 5) {
                                if (v295->member24 == 6) {
                                    v297 = v295->value28;
                                    if (v295->value28 == 7) {
                                        v297 = (((v297 - v296) >> 3) * 0xaaaaaaab00000000);
                                        if (!((((v297 - v296) >> 3) * 0xaaaaaaab00000000) == 0)) {
                                            if ((v297 & (1<<63)) != 0) goto loc_100000e0c;
                                            bzero(v297 >> 30);
                                            v293 = (operator_new(v297 >> 30));
                                        } else {
                                            v293 = 0;
                                        }
                                        v297 = v240;
                                        if (v295 != v240) {
                                            v298 = 24;
                                            v290 = v295->data;
                                            v291 = *(v296 + v295->data * v298);
                                            v292 = *((v296 + v295->data * v298) + 8);
                                            while (!(*(v296 + v295->data * v298) == *((v296 + v295->data * v298) + 8))) {
                                                *(v293 + (v291->f4 << 2)) = ((*(v293 + (v291->f4 << 2)) > (*(v293 + (v290 << 2)) + 1)) ? *(v293 + (v291->f4 << 2)) : (*(v293 + (v290 << 2)) + 1));
                                                while (v291 != v292) {
                                                }
                                                v295 += 4;
                                                if ((v295 + 4) != v297) continue;
                                            }
                                        }
                                        if (v293->value != 0) goto loc_100000d9c;
                                        if (v293->data != 3) goto loc_100000dac;
                                        v24 = &v104;
                                        t13 = operator_new(72);
                                        t13->value = t13;
                                        t13->data = t13;
                                        t13->item = t13;
                                        t13->member = t13;
                                        t13->value64 = 0;
                                        t14 = operator_new(12);
                                        t14->value = 0;
                                        t14->item = 0;
                                        obj = t14;
                                        t15 = operator_new(4);
                                        t15->f4 = 1;
                                        t13->data = t15;
                                        t13->item = t15;
                                        t13->value = t15;
                                        t14->data = 1;
                                        t16 = operator_new(4);
                                        t16->f4 = 2;
                                        t13->value32 = t16;
                                        t13->data40 = t16;
                                        t13->member = t16;
                                        t14->item = 1;
                                        t17 = operator_new(4);
                                        t17->f4 = 0;
                                        t13->member56 = t17;
                                        t13->value64 = t17;
                                        t13->item48 = t17;
                                        t14->value = 1;
                                        v24 = &v104;
                                        t18 = operator_new(12);
                                        t18->value = 0;
                                        t18->data = 0;
                                        v297 = (t18 + 12);
                                        v298 = t16;
                                        if ((kahn(&v104, &v80)) != 0) goto loc_100000dd0;
                                        if ((run(&v24, &v80)) != 0) goto loc_100000de0;
                                        printf("toposort ok: n=%zu depth[7]=%d\n");
                                        if (v56 != 0) {
                                            operator_delete();
                                        }
                                        if (0 != 0) {
                                            operator_delete();
                                        }
                                        if (v80 != 0) {
                                            operator_delete();
                                        }
                                        if (obj != 0) {
                                            operator_delete();
                                        }
                                        v294 = v104;
                                        if (v104 != 0) {
                                            v297 = v112;
                                            if (v294 != v112) {
                                                v295 = v297;
                                                    if (v295->value == 0) continue;
                                                    v297->value = argc;
                                                    operator_delete();
                                                }
                                            }
                                            v112 = v294;
                                            operator_delete();
                                        }
                                        operator_delete(v293);
                                        if (v184 != 0) {
                                            operator_delete();
                                        }
                                        if (v160 != 0) {
                                            operator_delete();
                                        }
                                        if (v208 != 0) {
                                            operator_delete();
                                        }
                                        if (obj3 != 0) {
                                            v240 = argc;
                                            operator_delete();
                                        }
                                        if (obj2 != 0) {
                                            operator_delete();
                                        }
                                        v293 = obj4;
                                        if (obj4 != 0) {
                                            v297 = v264;
                                            if (v293 != v264) {
                                                v294 = v297;
                                                    if (v294->fm24 == 0) continue;
                                                    v297->value = argc;
                                                    operator_delete();
                                                }
                                            }
                                            v264 = v293;
                                            operator_delete();
                                        }
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        v289 = (__assert_rtn(&s_main, &s_toposort_cpp, 130, "kahn_order[i] == expected[i]"));
    } else {
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_100000604;
        loc_100000cc8:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_100000650;
        loc_100000cd8:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_10000069c;
        loc_100000ce8:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_1000006e8;
        loc_100000cf8:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_100000734;
        loc_100000d08:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_100000780;
        loc_100000d18:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_1000007cc;
        loc_100000d28:
        operator_call(&(&obj4));
        v297 = v24;
        goto loc_100000818;
        loc_100000d38:
        goto loc_100000d84;
        loc_100000d48:
        goto loc_100000d84;
        loc_100000d58:
        goto loc_100000d84;
        loc_100000d68:
        goto loc_100000d84;
        loc_100000d78:
        loc_100000d84:
        v289 = (__assert_rtn(&s_main, &s_toposort_cpp));
        loc_100000d9c:
        goto loc_100000db8;
        loc_100000dac:
        loc_100000db8:
        v289 = (__assert_rtn(&s_main, &s_toposort_cpp));
        loc_100000dd0:
        goto loc_100000dec;
        loc_100000de0:
        loc_100000dec:
        v289 = (__assert_rtn(&s_main, &s_toposort_cpp));
        loc_100000e04:
        v289 = (__throw_length_error());
        loc_100000e0c:
        v289 = (__throw_length_error());
    __builtin_trap();
    v294 = argc;
        operator_delete(v293);
        if (v208 == 0) continue;
    loc_100000f1c:
    v240 = argc;
    operator_delete();
    loc_100000f24:
    _Unwind_Resume(v294);
    v216 = argc;
    operator_delete();
    if (obj3 == 0) goto loc_100000f24;
    goto loc_100000f1c;

struct Pair4 {
    int value;
    char _pad4[4];
    long data;
};
struct Struct8 {
    long value;
    long data;
    int item;
    int member;
    char _pad8[24];
    long value32;
    long data40;
};
struct Struct9 {
    long value;
    long data;
    char _pad0[16];
    char item;
    char _pad17[15];
    long member;
    long value40;
};
struct Pair5 {
    int value;
    char _pad4[12];
    char data;
};
struct Pair6 {
    int value;
    char _pad0[8];
    int data;
};
struct Struct10 {
    long value;
    long data;
    long item;
};
struct Pair7 {
    char* data;
    char _pad8[16];
    long value;
};
struct Struct11 {
    int value;
    int data;
    int item;
};
struct Pair8 {
    int value;
    char _pad0[4];
    int data;
};
// (anonymous namespace)::kahn((anonymous namespace)::DAG const&, std::vector<int>&)
long kahn(struct Pair4* obj, struct Struct10* obj2, long a, long b, long c) {
    struct Pair8* v122;
    struct Struct10* v114;
    struct Pair7* v115;
    long v119;
    long v113;
    struct Pair2* v123;
    long v106;
    struct Struct11* v116;
    struct Struct8* v107;
    struct Struct9* v108;
    struct Pair5* v109;
    long v88;
    int v36;
    long v72;
    long v110;
    long v111;
    long v117;
    long v118;
    v122 = ((((obj->data - obj->value) >> 3) * 0xaaaaaaaaaaaaaaab) << 32);
    v114 = obj2;
    v115 = obj;
    v119 = (((obj->data - obj->value) >> 3) * 0xaaaaaaaaaaaaaaab);
    if (!(((((obj->data - obj->value) >> 3) * 0xaaaaaaaaaaaaaaab) << 32) == 0)) {
        if ((v122 & (1<<63)) != 0) goto loc_10000144c;
        bzero(v122 >> 30);
        v113 = (operator_new(v122 >> 30));
        if (v119 < 1) goto L2;
        L1:
        v122 = (v119 & 0x7fffffff);
        v123 = v115->value;
        v106 = 0;
        if ((v119 & 0x7fffffff) < 4) goto L4;
        if ((v113 - v123) < 63) goto L4;
        if (v122 >= 16) goto L3;
        v106 = 0;
    } else {
        v113 = 0;
        if (v119 >= 1) goto L1;
        L2:
        v114->data = v114->value;
        v115 = v114->value;
        v116 = v114->value;
        goto loc_1000013e8;
        L3:
        v106 = (v119 & 0x7ffffff0);
        v107 = (v123 + 32);
        v108 = (v113 + 32);
        v109 = (v119 & 0x7ffffff0);
        v108->value = v107->value;
        v108->data = v107->data;
        v108->member = v107->value32;
        v108->value40 = v107->data40;
        v109 -= 16;
        while (v109 != 16) {
        }
        if (v122 == v106) goto L5;
        if (v119 == 0) goto L4;
    }
    v106 = (v119 & 0x7ffffffc);
    v107 = (v106 - (v119 & 0x7ffffffc));
    v108 = (v113 + (v106 << 2));
    v109 = (v123 + (v106 << 2));
    v107 += 4;
    while (v107 != 4) {
    }
    if (v122 != v106) {
        L4:
        v122 -= v106;
        v123 += (v106 << 2);
        v107 = (v113 + (v106 << 2));
        v107->member = v123->data;
        v122--;
        while (v122 != 1) {
        }
    }
    L5:
    v88 = 0;
    v36 = 0;
    v122 = 0;
    v123 = 0;
    goto L7;
    L6:
    v107->item = v106;
    v36++;
    v123 = (v36 + 1);
    while (!((v36 + 1) >= v119)) {
        L7:
        if (*(v113 + (v123 << 2)) != 0) continue;
        if (v122 >= v88) goto loc_100001140;
        v122->data = v123;
        L8:
        v123 = (((v122 - v72) >> 2) - 2);
        if (((v122 - v72) >> 2) < 2) continue;
        v123 >>= 1;
        v106 = v122->value;
        v107 = v122;
        v108 = (obj + ((v123 >> 1) << 2));
        v109 = *(obj + ((v123 >> 1) << 2));
        if (*(obj + ((v123 >> 1) << 2)) <= v122->value) continue;
    }
    v114->data = v114->value;
    v116 = v114->value;
    if (obj != v122) {
            v36 = obj->value;
            v123 = (((v122 - obj) >> 2) - 2);
            v106 = obj->value;
            v107 = ((v122 - obj) >> 2);
            if (!(((v122 - obj) >> 2) < 2)) {
                v108 = (v123 >> 1);
                v109 = obj;
                v110 = 0;
                goto L10;
                    L9:
                    v109->value = v123->value;
                    v109 = v123;
                    if (v110 > v108) goto L11;
                    L10:
                    v123 = ((v109 + (v110 << 2)) + 4);
                    v110 = ((v110 << 1) + 2);
                    v111 = v110;
                } while (((v110 << 1) + 2) < v107);
                v110 = v111;
                goto L9;
                L11:
                v107 = (v122 - 4);
                if (v123 != (v122 - 4)) {
                    v123->value = v107->item;
                    v107->item = v106;
                    v106 = ((((v123 - obj) + 4) >> 2) - 2);
                    if ((((v123 - obj) + 4) >> 2) < 2) goto L12;
                    v106 >>= 1;
                    v107 = v123->value;
                    v108 = (obj + ((v106 >> 1) << 2));
                    v109 = *(obj + ((v106 >> 1) << 2));
                    if (*(obj + ((v106 >> 1) << 2)) <= v123->value) goto L12;
                    v123->value = v109;
                    v123 = v108;
                    while (v106 != 0) {
                        v106 = ((v106 - 1) >> 1);
                        v108 = (obj + (((v106 - 1) >> 1) << 2));
                        v109 = *(obj + (((v106 - 1) >> 1) << 2));
                        if (*(obj + (((v106 - 1) >> 1) << 2)) > v107) continue;
                    }
                    v123->value = v107;
                } else {
                    v123->value = v106;
                }
            }
            L12:
            v122 = v114->data;
            if (v114->data >= v114->item) goto loc_1000013c8;
            v122->data = v36;
            L13:
            v114->data = v122;
            v122 = (v115->data + v36 * 24);
            v117 = *((v115->data + v36 * 24) + 8);
            v118 = *(v115->data + v36 * 24);
            goto L16;
            L14:
            v116->data = v106;
            L15:
            v118 += 4;
            L16:
            if (v118 == v117) continue;
        }
        v115 = v114->data;
        v116 = v114->value;
        if (obj != 0) {
            loc_1000013e0:
            v80 = obj;
            operator_delete();
        }
        loc_1000013e8:
        if (v113 != 0) {
            operator_delete(v113);
        }
        return obj;
    v115 = v116;
    if (obj != 0) goto loc_1000013e0;
    goto loc_1000013e8;
    loc_10000144c:
    __throw_length_error();
    loc_100001450:
    v105 = (__throw_bad_array_new_length());
    goto loc_10000145c;
    loc_100001458:
    v105 = (__throw_length_error());
    loc_10000145c:
    __builtin_trap();
    v114 = obj;
    if (v72 == 0) {
        if (v113 != 0) goto loc_100001490;
        loc_10000147c:
        v105 = (_Unwind_Resume(v114));
    }
    v80 = obj;
    operator_delete();
    if (v113 == 0) goto loc_10000147c;
    loc_100001490:
    operator_delete(v113);
    _Unwind_Resume(v114);
    loc_100001118:
    v107->item = v109;
    v107 = v108;
    if (v123 == 0) goto loc_1000010ac;
    v123 = ((v123 - 1) >> 1);
    v108 = (obj + (((v123 - 1) >> 1) << 2));
    v109 = *(obj + (((v123 - 1) >> 1) << 2));
    if (*(obj + (((v123 - 1) >> 1) << 2)) > v106) goto loc_100001118;
    goto loc_1000010ac;
    loc_100001140:
    operator_call(&v40);
    v122 = v64;
    goto loc_1000010e4;
    if (v116 >= v122) goto loc_100001330;
    v116->item = v120;
    loc_1000012d8:
    v80 = v116;
    v122 = v72;
    v123 = (((v116 - v72) >> 2) - 2);
    if (((v116 - v72) >> 2) < 2) goto loc_1000012a8;
    v123 >>= 1;
    v106 = v116->value;
    v107 = (v122 + ((v123 >> 1) << 2));
    v108 = *(v122 + ((v123 >> 1) << 2));
    if (*(v122 + ((v123 >> 1) << 2)) <= v116->value) goto loc_1000012a8;
    loc_100001308:
    v116->data = v108;
    v116 = v107;
    if (v123 == 0) goto loc_1000012a4;
    v123 = ((v123 - 1) >> 1);
    v107 = (v122 + (((v123 - 1) >> 1) << 2));
    v108 = *(v122 + (((v123 - 1) >> 1) << 2));
    if (*(v122 + (((v123 - 1) >> 1) << 2)) > v106) goto loc_100001308;
    goto loc_1000012a4;
    loc_100001330:
    v123 = (((v116 - v72) >> 2) + 1);
    v107 = v72;
    v116 -= v72;
    if (((((v116 - v72) >> 2) + 1) >> 62) != 0) goto loc_100001458;
    v123 = (((v122 - v107) < 0x7ffffffffffffffc) ? ((((v122 - v107) >> 1) >= v123) ? ((v122 - v107) >> 1) : v123) : 0x3fffffffffffffff);
    if (((((v122 - v107) < 0x7ffffffffffffffc) ? ((((v122 - v107) >> 1) >= v123) ? ((v122 - v107) >> 1) : v123) : 0x3fffffffffffffff) >> 62) != 0) goto loc_100001450;
    t69 = operator_new(v123 << 2);
    *((t69 + v116) + 4) = v120;
    v88 = v16;
    v105 = (memcpy(((t69 + v116) - (v16 << 2)), v24, v116));
    v116 = (t69 + v116);
    v120 = v24;
    if (v24 == 0) goto loc_1000012d8;
    v105 = (operator_delete(v120));
    goto loc_1000012d8;
    loc_1000013c8:
    v105 = (operator_call(&v40));
    v122 = v64;
    goto loc_10000128c;

struct Struct12 {
    char* data;
    char _pad8[24];
    long value;
    long data2;
};
struct Pair9 {
    long value;
    long data;
};
struct Struct13 {
    long value;
    char* data;
    char _pad16[16];
    long data2;
    long item;
};
struct Struct14 {
    int value;
    int data;
    char _pad4[4];
    long item;
};
// (anonymous namespace)::DfsTopo::run(std::vector<int>&)
long run(struct Struct12* obj, long obj2, long n) {
    struct Struct14* v12;
    long v13;
    struct Struct13* v8;
    long ccmp;
    long chained;
    long compare;
    long v1;
    long v2;
    struct Pair9* v3;
    struct Pair9* v4;
    long v5;
    obj->data2 = obj->value;
    v12 = obj->data;
    v13 = *obj->data;
    if (!((((*(obj->data + 8) - *obj->data) >> 3) * 0xaaaaaaab) < 1)) {
        v8 = obj;
            if (*(v8->data + (0 << 2)) != 0) continue;
            if ((visit(v8, 0)) == 0) goto loc_10000167c;
            v12 = v8->value;
            v13 = *v8->value;
        }
        /* ccmp v8->data2, (v8->item - 4)  (chained != compare) */
        v12 = (v8->item - 4);
        if (v8->data2 >= v8->item) goto loc_100001660;
        v13 = (obj2 + 4);
        v1 = ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) == obj2) ? ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) - ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) == obj2) ? obj2 : (obj2 + 1))) >> 3) : (((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) - ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) == obj2) ? obj2 : (obj2 + 1))) >> 3) + 1));
        v2 = (n - 8);
        if (((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) == obj2) ? ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) - ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) == obj2) ? obj2 : (obj2 + 1))) >> 3) : (((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) - ((((((n - 8) >= (obj2 + 4)) ? (n - 8) : (obj2 + 4)) - 4) == obj2) ? obj2 : (obj2 + 1))) >> 3) + 1)) >= 19) goto L1;
        v13 = obj2;
    } else {
        goto loc_100001660;
        L1:
        v2 = (((((v2 >= v13) ? v2 : v13) - 4) == obj2) ? (((((v2 >= v13) ? v2 : v13) - 4) - (((((v2 >= v13) ? v2 : v13) - 4) == obj2) ? obj2 : (obj2 + 1))) >> 3) : ((((((v2 >= v13) ? v2 : v13) - 4) - (((((v2 >= v13) ? v2 : v13) - 4) == obj2) ? obj2 : (obj2 + 1))) >> 3) + 1));
        if (obj2 < n) {
            v13 = obj2;
            if (((n - (v2 << 2)) - 4) < (v13 + (v2 << 2))) goto loc_100001640;
        }
        v12 -= (((v1 + 1) & 0x7ffffffffffffff8) << 2);
        v13 = (obj2 + (((v1 + 1) & 0x7ffffffffffffff8) << 2));
        v1++;
        v2 = ((v1 + 1) & 0x7ffffffffffffff8);
        v3 = (obj2 + 16);
        v4 = (n - 16);
        v5 = ((v1 + 1) & 0x7ffffffffffffff8);
        v3->value = v4->data;
        v3->data = v4->value;
        v4->value = v3->data;
        v4->data = v3->value;
        v3 += 32;
        v4 -= 32;
        v5 -= 8;
        while ((v5 - 8) != 0) {
        }
        if (v1 == v2) goto loc_100001660;
    }
    loc_100001640:
    v13 += 4;
    v13->fm4 = v12->data;
    v12->value = v13->fm4;
    v13 += 4;
    while (v13 < v12) {
    }
    loc_100001660:
    if (obj2 != v10) {
        __assign_with_size(obj2, ((n - obj2) >> 2));
    }
    loc_10000167c:
    return obj;

struct Pair10 {
    long value;
    long data;
};
// (anonymous namespace)::is_valid_topo((anonymous namespace)::DAG const&, std::vector<int> const&)
long is_valid_topo(struct Pair10* obj, struct Pair10* obj2) {
    long v8;
    struct Pair10* ret;
    long v6;
    long v7;
    long v4;
    long v0;
    long v9;
    long* v1;
    long v2;
    long v3;
    v8 = ((((obj->data - obj->value) >> 3) * 0xaaaaaaaaaaaaaaab) << 32);
    ret = obj2;
    v6 = obj->value;
    v7 = (((obj->data - obj->value) >> 3) * 0xaaaaaaaaaaaaaaab);
    if (!(((((obj->data - obj->value) >> 3) * 0xaaaaaaaaaaaaaaab) << 32) == 0)) {
        if ((v8 & (1<<63)) != 0) goto L4;
        memset(255, (v8 >> 30));
        v4 = (operator_new(v8 >> 30));
    } else {
        v4 = 0;
    }
    v8 = ret->value;
    v0 = (ret->data - ret->value);
    if (!(((ret->data - ret->value) >> 2) < 1)) {
        v9 = 0;
        v0 = ((v0 >> 2) & 0x7fffffff);
        *(v4 + (*(v8 + (v9 << 2)) << 2)) = v9;
        v9++;
        while (v0 != (v9 + 1)) {
        }
    }
    if (v7 >= 1) {
        v8 = 0;
        v9 = (v7 & 0x7fffffff);
        v0 = 24;
        goto L2;
        L1:
        v8++;
        while (!((v8 + 1) == v9)) {
            L2:
            v1 = *(v6 + v8 * v0);
            v2 = *((v6 + v8 * v0) + 8);
            if (*(v6 + v8 * v0) == *((v6 + v8 * v0) + 8)) continue;
            v3 = *(v4 + (v8 << 2));
            while (v3 < *(v4 + (v1->f0 << 2))) {
                v1 += 4;
                if ((v1 + 4) != v2) continue;
            }
        }
    } else {
        ret = 1;
        if (v4 == 0) goto L3;
    }
    operator_delete(v4);
    L3:
    return ret;
    L4:
    __throw_length_error();
    goto L1;
}

struct Struct15 {
    char _pad0[32];
    long value;
};
struct Struct16 {
    char _pad0[8];
    long value;
    long data;
    char _pad24[16];
    long item;
};
// (anonymous namespace)::DfsTopo::DfsTopo_dtor()
long DfsTopo_dtor(struct Struct15* obj) {
    struct Struct16* ret;
    ret = obj;
    if (obj->value != 0) {
        ret->item = obj;
        operator_delete();
    }
    if (ret->value != 0) {
        ret->data = obj;
        operator_delete();
    }
    return ret;
}

struct Struct17 {
    char _pad0[24];
    long value;
};
struct Struct18 {
    long value;
    long data;
    char _pad16[16];
    long item;
};
// (anonymous namespace)::DAG::DAG_dtor()
long DAG_dtor(struct Struct17* obj) {
    struct Struct18* v1;
    long v2;
    long* v4;
    long* v3;
    v1 = obj;
    if (obj->value != 0) {
        v1->item = obj;
        operator_delete();
    }
    v2 = v1->value;
    if (v1->value != 0) {
        v4 = v1->data;
        if (v2 != v1->data) {
            v3 = v4;
                if (v3->fm24 == 0) continue;
                v4->fm16 = obj;
                operator_delete();
            }
        }
        v1->data = v2;
        operator_delete();
    }
    return v1;

struct Buffer2 {
    char _pad0[8];
    char* data;
    char _pad16[24];
    long value;
    long size;
};
// (anonymous namespace)::DfsTopo::visit(int)
long visit(struct Pair10* obj, long arr, long a, long b, long c) {
    long* v45;
    struct Buffer2* v41;
    long* v43;
    long v44;
    long v16;
    long v40;
    *(obj->data + (arr << 2)) = 1;
    v45 = obj->data;
    v41 = obj;
    v43 = *(*obj->value + arr * 24);
    v44 = *((*obj->value + arr * 24) + 8);
    if (!(*(*obj->value + arr * 24) != *((*obj->value + arr * 24) + 8))) {
        goto L4;
        L1:
        if ((visit(v41)) == 0) goto L6;
        L2:
        v43 += 4;
        if ((v43 + 4) == v44) goto L3;
    }
    v45 = *(v41->data + (v43->f0 << 2));
    if (*(v41->data + (v43->f0 << 2)) == 0) goto L1;
    if (v45 != 1) goto L2;
    goto L6;
    L3:
    v45 = v41->data;
    L4:
    *(v45 + (arr << 2)) = 2;
    v45 = v41->value;
    if (v41->value < v41->size) {
        v45->f4 = arr;
        L5:
        v41->value = v45;
        L6:
        return obj;
    }
    operator_call(&v16);
    v45 = v40;
    goto L5;
}

