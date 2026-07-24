#include <cstdio>
#include <cstring>
#include <vector>

using namespace std;

long operator_call(struct Pair7* obj);
long main.cold.1(struct Struct8* obj, long a, long b);

const char* s_main = "main";
const char* s_dijkstra_cpp = "dijkstra.cpp";

static const int table[2] = {
    -1, 2147483647 
};

class Record0 {
public:
    long value;
    // int& std::vector<int>::emplace_back<int const&>(int const&)::'lambda0'()::operator_call() const
    long operator_call(long a, long b, long c) {
        long v6;
        long* obj;
        long v1;
        long v2;
        long* v3;
        long v4;
        long t55;
        long v5;
        v6 = (((*(obj->f16 + 8) - *obj->f16) >> 2) + 1);
        v1 = *obj->f16;
        v2 = (*(obj->f16 + 8) - *obj->f16);
        v3 = obj->f16;
        v4 = ((*(obj->f16 + 8) - *obj->f16) >> 2);
        if (!(((((*(obj->f16 + 8) - *obj->f16) >> 2) + 1) >> 62) != 0)) {
            v6 = (((v3->f16 - v1) < 0x7ffffffffffffffc) ? ((((v3->f16 - v1) >> 1) >= v6) ? ((v3->f16 - v1) >> 1) : v6) : 0x3fffffffffffffff);
            if (((((v3->f16 - v1) < 0x7ffffffffffffffc) ? ((((v3->f16 - v1) >> 1) >= v6) ? ((v3->f16 - v1) >> 1) : v6) : 0x3fffffffffffffff) >> 62) != 0) goto loc_100001a34;
            t55 = operator_new(v6 << 2);
            *((t55 + v2) + 4) = *obj->f8;
            memcpy(((t55 + v2) - (v4 << 2)), v1, v2);
            v3->value = ((t55 + v2) - (v4 << 2));
            v3->f8 = (t55 + v2);
            v3->f16 = (t55 + (v6 << 2));
            v5 = (t55 + v2);
            if (v1 != 0) {
                operator_delete(v1);
            }
            *this->value = v5;
            return obj;
        }
        __throw_length_error();
        loc_100001a34:
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
};
struct Pair {
    char value;
    char _pad1[7];
    long data;
};
struct Pair2 {
    int value;
    char _pad4[4];
    long data;
};
struct Pair3 {
    char _pad0[16];
    int value;
    char _pad20[4];
    long data;
};
struct Pair4 {
    long value;
    int data;
};
struct Pair5 {
    long value;
    char _pad8[24];
    long data;
};
struct Buffer {
    char _pad0[8];
    long value;
    long data;
    char _pad24[8];
    long item;
    long member;
    char _pad48[8];
    long value56;
    long size;
    char _pad72[8];
    long data80;
    long item88;
    char _pad96[8];
    long member104;
    long value112;
    char _pad120[8];
    long data128;
    long item136;
};
struct Struct1 {
    long value;
    long data;
    long item;
    long member;
};
struct Struct2 {
    long value;
    long data;
    char item;
    char _pad1[31];
    long member;
    long value40;
};
struct Struct3 {
    long value;
    int data;
    char _pad4[4];
    long item;
    char _pad16[16];
    long member;
};
struct Struct4 {
    int value;
    char _pad4[4];
    int data;
    char _pad12[20];
    long item;
    char _pad40[16];
    long member;
    char _pad64[16];
    long value80;
    char _pad88[16];
    long data104;
    char _pad112[16];
    long item128;
};
struct Struct5 {
    long value;
    long data;
    long item;
};
struct Pair6 {
    int value;
    char _pad0[4];
    int data;
};
struct Struct6 {
    long value;
    int data;
    char _pad0[8];
    long item;
    long member;
    long value24;
    long data32;
    long item40;
};
int main(int argc, char** argv) {
    long v144;
    struct Buffer* buf3;
    struct Struct0* t0;
    struct Pair* t1;
    struct Pair2* t2;
    struct Pair3* t3;
    long v120;
    struct Pair5* buf2;
    struct Struct6* v199;
    struct Struct4* v187;
    struct Pair6* v190;
    struct Pair* v200;
    long buf;
    struct Pair4* t7;
    struct Struct5* v189;
    long v179;
    long v196;
    long v197;
    struct Struct1* v180;
    struct Struct2* v181;
    long v182;
    long v183;
    struct Struct3* v184;
    long v194;
    long* v191;
    struct Pair2* v195;
    long n;
    int v116;
    long v176;
    long v186;
    long v96;
    v144 = &buf3;
    t0 = operator_new(144);
    t0->value = t0;
    t0->data = t0;
    t0->item = t0;
    t0->member = t0;
    t0->value32 = t0;
    t0->data40 = t0;
    t0->item48 = t0;
    t0->member56 = t0;
    buf3 = t0->data;
    t1 = operator_new(16);
    t1->value = 1;
    t1->data = 7;
    t0->data = (t1 + 16);
    t0->item = (t1 + 16);
    t0->value = t1;
    t2 = operator_new(16);
    t2->value = 0;
    t2->data = 7;
    t0->value32 = (t2 + 16);
    t0->data40 = (t2 + 16);
    t0->member = t2;
    t3 = operator_new(32);
    t3->value = 2;
    t3->data = 9;
    t0->value = *t1->value;
    t0->data = (t3 + 32);
    t0->item = (t3 + 32);
    operator_delete(t1);
    t0->data = (t3 + 32);
    v120 = 0;
    buf2 = buf3->value56;
    v199 = buf3->value56;
    v187 = buf3;
    v190 = (t3 + 32);
    if (buf3->value56 < buf3->size) {
        L1:
        v187->member = v199;
        v120 = 5;
        buf2 = buf3->value;
        v199 = buf3->value;
        v187 = buf3;
        if (buf3->value >= buf3->data) goto loc_100001054;
        L2:
        v187->data = v199;
        v120 = 0;
        buf2 = buf3->data128;
        v199 = buf3->data128;
        v187 = buf3;
        if (buf3->data128 >= buf3->item136) goto loc_100001064;
        L3:
        v187->item128 = v199;
        v120 = 2;
        buf2 = buf3->item;
        v199 = buf3->item;
        v187 = buf3;
        if (buf3->item >= buf3->member) goto loc_100001074;
        L4:
        v187->item = v199;
        v120 = 1;
        buf2 = buf3->value56;
        v199 = buf3->value56;
        v187 = buf3;
        if (buf3->value56 >= buf3->size) goto loc_100001084;
        L5:
        v187->member = v199;
        v120 = 3;
        buf2 = buf3->item;
        v199 = buf3->item;
        v187 = buf3;
        if (buf3->item >= buf3->member) goto loc_100001094;
        L6:
        v187->item = v199;
        v120 = 1;
        buf2 = buf3->data80;
        v199 = buf3->data80;
        v187 = buf3;
        if (buf3->data80 >= buf3->item88) goto loc_1000010a4;
        L7:
        v187->value80 = v199;
        v120 = 3;
        buf2 = buf3->value56;
        v199 = buf3->value56;
        v187 = buf3;
        if (buf3->value56 >= buf3->size) goto loc_1000010b4;
        L8:
        v187->member = v199;
        v120 = 2;
        buf2 = buf3->data80;
        v199 = buf3->data80;
        v187 = buf3;
        if (buf3->data80 >= buf3->item88) goto loc_1000010c4;
        L9:
        v187->value80 = v199;
        v120 = 5;
        buf2 = buf3->value56;
        v199 = buf3->value56;
        v187 = buf3;
        if (buf3->value56 >= buf3->size) goto loc_1000010d4;
        L10:
        v187->member = v199;
        v120 = 2;
        buf2 = buf3->data128;
        v199 = buf3->data128;
        v187 = buf3;
        if (buf3->data128 >= buf3->item136) goto loc_1000010e4;
        L11:
        v187->item128 = v199;
        v120 = 4;
        buf2 = buf3->data80;
        v199 = buf3->data80;
        v187 = buf3;
        if (buf3->data80 >= buf3->item88) goto loc_1000010f4;
        L12:
        v187->value80 = v199;
        v120 = 3;
        buf2 = buf3->member104;
        v199 = buf3->member104;
        v187 = buf3;
        if (buf3->member104 >= buf3->value112) goto loc_100001104;
        L13:
        v187->data104 = v199;
        v120 = 5;
        buf2 = buf3->member104;
        v199 = buf3->member104;
        v187 = buf3;
        if (buf3->member104 >= buf3->value112) goto loc_100001114;
        L14:
        v187->data104 = v199;
        v120 = 4;
        buf2 = buf3->data128;
        v199 = buf3->data128;
        v200 = (buf3 + 120);
        v187 = buf3;
        if (buf3->data128 >= buf3->item136) goto loc_100001124;
        L15:
        v187->item128 = v199;
        assign(&buf2, (((v199 - v200) >> 3) * 0xaaaaaaab), table);
        v144 = -1;
        assign(&buf, (((v199 - v200) >> 3) * 0xaaaaaaab), &v144);
        buf2->value = 0;
        t7 = operator_new(16);
        t7->value = 0;
        t7->data = 0;
        v199 = (t7 + 16);
        v187 = t7;
        goto L17;
        L16:
        v199 = v189;
        while (v187 != v199) {
            L17:
            v200 = (((v199 - v187) >> 4) - 2);
            v179 = ((v199 - v187) >> 4);
            v196 = v187->value;
            v197 = v187->data;
            if (!(((v199 - v187) >> 4) < 2)) {
                v180 = (v200 >> 1);
                v181 = v187;
                v182 = 0;
                goto L19;
                    L18:
                    v181 = v200;
                    if (v182 > v180) goto L20;
                    L19:
                    v200 = ((v181 + (v182 << 4)) + 16);
                    v182 = ((v182 << 1) + 2);
                    v183 = v182;
                    v184 = (v181 + (v182 << 4));
                } while (((v182 << 1) + 2) < v179);
                v182 = v183;
                goto L18;
                L20:
                v179 = (v199 - 16);
                if (v200 != (v199 - 16)) {
                    v179 = ((((v200 - v187) + 16) >> 4) - 2);
                    if ((((v200 - v187) + 16) >> 4) < 2) goto L21;
                    v179 >>= 1;
                    v180 = v200->value;
                    v182 = (v187 + ((v179 >> 1) << 4));
                    if (*(v187 + ((v179 >> 1) << 4)) <= v200->value) goto L21;
                    v181 = v200->data;
                    v200 = v182;
                    while (v179 != 0) {
                        v179 = ((v179 - 1) >> 1);
                        v182 = (v187 + (((v179 - 1) >> 1) << 4));
                        if (*(v187 + (((v179 - 1) >> 1) << 4)) > v180) continue;
                    }
                    v200->value = v180;
                    v200->data = v181;
                }
            }
            L21:
            v189 = (v199 - 16);
            v194 = v197;
            if (v196 > *(buf2 + (v197 << 3))) goto L16;
            v191 = *((buf3 + v194 * argc) + 8);
            v195 = *(buf3 + v194 * argc);
            if (!(*(buf3 + v194 * argc) != *((buf3 + v194 * argc) + 8))) {
                goto L16;
                L22:
                v199 = v189;
                L23:
                v189 = v199;
                v195 += 16;
                if ((v195 + 16) == v191) continue;
            }
        }
        operator_delete(v187);
        v199 = buf2;
        if (buf2->value != 0) goto loc_100001134;
        if (v199->item != 7) goto loc_100001144;
        if (v199->member != 9) goto loc_100001154;
        if (v199->value24 != 20) goto loc_100001164;
        if (v199->data32 != 20) goto loc_100001174;
        if (v199->item40 != 11) goto loc_100001184;
        n = 0;
        v116 = 4;
        v199 = 4;
        v191 = 0;
        v176 = v191;
        while (v191 < n) {
            v191->f4 = v199;
            L24:
            v116 = *(buf + (v116 << 2));
            v199 = *(buf + (v116 << 2));
            if (*(buf + (v116 << 2)) != 1) continue;
        }
        operator_call(&v144);
        v191 = v176;
        goto L24;
        v189 = v120;
        v190 = (v191 - v120);
        if (v191 != v120) {
            if ((v190 & (1<<63)) != 0) goto loc_1000011f0;
            v199 = (v190 - 4);
            v187 = (operator_new(v190));
            if (!((v190 - 4) < 60)) {
                if (v187 >= v191) goto loc_100000fb4;
                if (v189 >= (v187 + v190)) goto loc_100000fb4;
            }
            v199 = v191;
            v190 = v187;
            L25:
            v190->data = v199->data;
            while (v199 != v189) {
            }
            L26:
            if (v189 != 0) {
                L27:
                operator_delete(v189);
            }
            L28:
            if (v187->value != 0) goto loc_100001020;
            if (v190->value != 4) goto loc_100001020;
            v199 = ((v190 - v187) >> 2);
            if (!(((v190 - v187) >> 2) < 2)) {
                v200 = 0;
                v179 = (v187 - 4);
                v180 = buf3;
                v181 = 1;
                v182 = 24;
                v183 = 0x7fffffffffffffff;
                v184 = *(v180 + *(v179 + (v181 << 2)) * v182);
                while (!(*(v180 + *(v179 + (v181 << 2)) * v182) == *((v180 + *(v179 + (v181 << 2)) * v182) + 8))) {
                    v186 = 0x7fffffffffffffff;
                        if (v184->data != argc) continue;
                        v186 = ((v184->item < v186) ? v184->item : v186);
                    }
                    if (v186 == v183) break;
                    v200 = (v186 + v200);
                    v181++;
                    if ((v181 + 1) != v199) continue;
                }
            } else {
                v200 = 0;
            }
            if (v200 != buf2->data) goto loc_1000011a8;
            printf("dijkstra ok: dist[4]=%lld path_len=%zu\n");
            operator_delete(v187);
            if (buf != 0) {
                operator_delete();
            }
            if (buf2 != 0) {
                operator_delete();
            }
            v187 = buf3;
            if (buf3 != 0) {
                v199 = v96;
                if (v187 != v96) {
                    v189 = v199;
                        if (v189->value == 0) continue;
                        v199->value = argc;
                        operator_delete();
                    }
                }
                v96 = v187;
                operator_delete();
            }
            return 0;
        v187 = 0;
        if (v189 != 0) goto loc_100000e40;
        goto loc_100000e4c;
        loc_100000fb4:
        v199 = (v191 - ((((v199 >> 2) + 1) & 0x7ffffffffffffff0) << 2));
        v200 = ((v199 >> 2) + 1);
        v179 = (((v199 >> 2) + 1) & 0x7ffffffffffffff0);
        v180 = (v191 - 32);
        v181 = (v187 + 32);
        v182 = (((v199 >> 2) + 1) & 0x7ffffffffffffff0);
        v190 = (v187 + ((((v199 >> 2) + 1) & 0x7ffffffffffffff0) << 2));
        v181->value = v180->member;
        v181->data = v180->item;
        v181->member = v180->data;
        v181->value40 = v180->value;
        v180 -= 64;
        v182 -= 16;
        while (v182 != 16) {
        }
        if (v200 != v179) goto loc_100000e2c;
        goto loc_100000e3c;
        loc_100001020:
        v177 = (__assert_rtn(&s_main, &s_dijkstra_cpp, 103, "path.front() == 0 && path.back() == 4"));
    } else {
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000654;
        loc_100001054:
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000694;
        loc_100001064:
        operator_call(&v144);
        v199 = buf2;
        goto loc_1000006d4;
        loc_100001074:
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000718;
        loc_100001084:
        operator_call(&v144);
        v199 = buf2;
        goto loc_10000075c;
        loc_100001094:
        operator_call(&v144);
        v199 = buf2;
        goto loc_1000007a0;
        loc_1000010a4:
        operator_call(&v144);
        v199 = buf2;
        goto loc_1000007e4;
        loc_1000010b4:
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000828;
        loc_1000010c4:
        operator_call(&v144);
        v199 = buf2;
        goto loc_10000086c;
        loc_1000010d4:
        operator_call(&v144);
        v199 = buf2;
        goto loc_1000008b0;
        loc_1000010e4:
        operator_call(&v144);
        v199 = buf2;
        goto loc_1000008f0;
        loc_1000010f4:
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000934;
        loc_100001104:
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000978;
        loc_100001114:
        operator_call(&v144);
        v199 = buf2;
        goto loc_1000009bc;
        loc_100001124:
        operator_call(&v144);
        v199 = buf2;
        goto loc_100000a00;
        loc_100001134:
        goto loc_100001190;
        loc_100001144:
        goto loc_100001190;
        loc_100001154:
        goto loc_100001190;
        loc_100001164:
        goto loc_100001190;
        loc_100001174:
        goto loc_100001190;
        loc_100001184:
        loc_100001190:
        v177 = (__assert_rtn(&s_main, &s_dijkstra_cpp));
        loc_1000011a8:
        v177 = (__assert_rtn(&s_main, &s_dijkstra_cpp, 114, "walked == dist[4]"));
        v177 = (__assert_rtn(&s_main, &s_dijkstra_cpp, 111, "step != kInf"));
        loc_1000011f0:
        v177 = (__throw_length_error());
        loc_1000011f8:
        v177 = (__throw_length_error());
        loc_100001200:
        v177 = (__throw_bad_array_new_length());
    __builtin_trap();
        v189 = argc;
        if (v120 == 0) {
            if (buf != 0) goto loc_100001294;
            loc_10000123c:
            if (buf2 != 0) goto loc_1000012a4;
            loc_100001244:
            if (buf3 != 0) break;
        }
        v128 = argc;
        operator_delete();
        if (buf == 0) goto loc_10000123c;
        loc_100001294:
        v48 = argc;
        operator_delete();
        if (buf2 == 0) goto loc_100001244;
        loc_1000012a4:
        v72 = argc;
        operator_delete();
    } while (buf3 == 0);
    main.cold.1(&v96, &buf3);
    _Unwind_Resume(v189);
    v199 = buf2;
    if (*(buf2 + (v194 << 3)) == 0x7fffffffffffffff) goto loc_100000bd8;
    v200 = v195->value;
    v192 = (v195->data + v196);
    if ((v195->data + v196) >= *(v199 + (v195->value << 3))) goto loc_100000bd8;
    *(v199 + (v200 << 3)) = v192;
    *(buf + (v200 << 2)) = v197;
    v190 = v195->value;
    if (v189 >= b) goto loc_100000c98;
    v189->data = v192;
    v189->item = v190;
    loc_100000c34:
    v199 = (v189 + 16);
    v200 = ((((v189 + 16) - v187) >> 4) - 2);
    if ((((v189 + 16) - v187) >> 4) < 2) goto loc_100000bdc;
    v200 >>= 1;
    v179 = v189->data;
    v181 = (v187 + ((v200 >> 1) << 4));
    if (*(v187 + ((v200 >> 1) << 4)) <= v189->data) goto loc_100000bdc;
    v180 = v189->item;
    loc_100000c64:
    v189 = v181;
    if (v200 == 0) goto loc_100000c90;
    v200 = ((v200 - 1) >> 1);
    v181 = (v187 + (((v200 - 1) >> 1) << 4));
    if (*(v187 + (((v200 - 1) >> 1) << 4)) > v179) goto loc_100000c64;
    loc_100000c90:
    v189->data = v179;
    v189->item = v180;
    goto loc_100000bdc;
    loc_100000c98:
    v199 = (((v189 - v187) >> 4) + 1);
    v179 = ((v189 - v187) >> 4);
    v189 -= v187;
    if (((((v189 - v187) >> 4) + 1) >> 60) != 0) goto loc_1000011f8;
    v32 = v179;
    v200 = (((b - v187) < argv) ? ((((b - v187) >> 3) >= v199) ? ((b - v187) >> 3) : v199) : a);
    if (((((b - v187) < argv) ? ((((b - v187) >> 3) >= v199) ? ((b - v187) >> 3) : v199) : a) >> 60) != 0) goto loc_100001200;
    v24 = v200;
    t8 = operator_new(v200 << 4);
    *(t8 + v189) = v192;
    *((t8 + v189) + 8) = v190;
    memcpy(((t8 + v189) - (v32 << 4)), v187, v189);
    operator_delete(v187);
    v198 = (t8 + (v24 << 4));
    v187 = ((t8 + v189) - (v32 << 4));
    v189 = (t8 + v189);
    v190 = (t8 + (v24 << 4));
    goto loc_100000c34;

struct Pair7 {
    char _pad0[8];
    long value;
    char* data;
};
struct Struct7 {
    long value;
    long data;
    long item;
};
// (anonymous namespace)::Edge& std::vector<(anonymous namespace)::Edge>::emplace_back<(anonymous namespace)::Edge>((anonymous namespace)::Edge&&)::'lambda0'()::operator_call() const
long operator_call(struct Pair7* obj) {
    long v6;
    long v1;
    long v2;
    struct Struct7* v3;
    long v4;
    long t50;
    long* v0;
    long v5;
    v6 = (((*(obj->data + 8) - *obj->data) >> 4) + 1);
    v1 = *obj->data;
    v2 = (*(obj->data + 8) - *obj->data);
    v3 = obj->data;
    v4 = ((*(obj->data + 8) - *obj->data) >> 4);
    if (!(((((*(obj->data + 8) - *obj->data) >> 4) + 1) >> 60) != 0)) {
        v6 = (((v3->item - v1) < 0x7ffffffffffffff0) ? ((((v3->item - v1) >> 3) >= v6) ? ((v3->item - v1) >> 3) : v6) : 0xfffffffffffffff);
        if (((((v3->item - v1) < 0x7ffffffffffffff0) ? ((((v3->item - v1) >> 3) >= v6) ? ((v3->item - v1) >> 3) : v6) : 0xfffffffffffffff) >> 60) != 0) goto L1;
        t50 = operator_new(v6 << 4);
        memcpy(((t50 + v2) - (v4 << 4)), v1, v2);
        v3->value = ((t50 + v2) - (v4 << 4));
        v3->data = ((t50 + v2) + 16);
        v3->item = (t50 + (v6 << 4));
        v0 = obj;
        v5 = ((t50 + v2) + 16);
        if (v1 != 0) {
            operator_delete(v1);
        }
        *v0->f0 = v5;
        return obj;
    }
    __throw_length_error();
    L1:
    __throw_bad_array_new_length();
}

struct Struct8 {
    long value;
};
long main.cold.1(struct Struct8* obj, long a, long b) {
    long* v5;
    long* v2;
    long* v4;
    v5 = obj->value;
    v2 = obj;
    if (a != obj->value) {
        v4 = v5;
        while (v4->fm24 != 0) {
            v5->fm16 = obj;
            operator_delete();
            v5 = v4;
            if (v4 != a) continue;
        }
    }
    v2->f0 = a;
    return (operator_delete());

