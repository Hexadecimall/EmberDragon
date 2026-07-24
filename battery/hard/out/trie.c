#include <cstdio>
#include <cstring>

static long g_3c6ef361[8192];   // unresolved data global — real bytes pending (lifter)


struct Struct0 {
    long value;
    long data;
    long item;
    long member;
    long value32;
    long data40;
    char _pad48[48];
    long item96;
    long member104;
};
struct Struct1 {
    int value;
    int data;
    int item;
    int member;
    char _pad16[84];
    int value100;
};
int main(int argc, char** argv) {
    struct Struct0* t0;
    long v113;
    struct Struct1* v116;
    long v119;
    long v120;
    long v121;
    long* v122;
    long v123;
    struct Struct0* v124;
    long v125;
    long v126;
    long v117;
    long v88;
    long v118;
    long v115;
    char v111;
    long v114;
    long t2;
    long t3;
    t0 = operator_new(112);
    t0->value = t0;
    t0->data = t0;
    t0->item = t0;
    t0->member = t0;
    t0->value32 = t0;
    t0->data40 = t0;
    t0->item96 = -1;
    t0->member104 = 0;
    v113 = 0x9e3779b1;
    v116 = t0;
    v119 = 0x3c6ef35f;
    v120 = 0;
    v121 = (t0 + 112);
    v122 = g_3c6ef361;
    v123 = 0;
    v124 = (t0 + 112);
    v125 = 112;
        v126 = (0x3c6ef35f + v120 * v113);
        v117 = (v122 - ((v119 * 0xaaaaaaab) >> 34) * 6);
        v112 = (push_back(&v88, ((((v126 ^ v126) ^ (v126 ^ v126)) & 3) + 97)));
        v126 = (((v126 ^ v126) ^ (v126 ^ v126)) ^ ((v126 ^ v126) ^ (v126 ^ v126)));
        v117--;
        v118 = ((v126 ^ v126) ^ (v126 ^ v126));
        while (v117 != 1) {
        }
        v115 = ((v111 < 0) ? v111 : v111);
        v122 = ((v111 < 0) ? v113 : &v88);
        if (((v111 >= 0) ? v111 : v111) == 0) {
            v113 = -1;
            v114 = 0xb6db6db7;
            v118 = v115;
            v123 = 0;
            goto L2;
                L1:
                *((v116 + *((v116 + v123 * v125) + (v117 << 2)) * v125) + 104) = (*((v116 + *((v116 + v123 * v125) + (v117 << 2)) * v125) + 104) + 1);
                v118--;
                v122++;
                v123 = *((v116 + v123 * v125) + (v117 << 2));
                while (v118 != 1) {
                    L2:
                    v126 = (v116 + v123 * v125);
                    v117 = (v122->f0 - 97);
                    if ((*((v116 + v123 * v125) + ((v122->f0 - 97) << 2)) & (1<<31)) == 0) continue;
                }
                *(v126 + (v117 << 2)) = (((v124 - v116) >> 4) * v114);
            } while (v124 < v121);
            v126 = (1 + ((v124 - v116) >> 4) * 0x6db6db6db6db6db7);
            v113 = 0x6db6db6db6db6db7;
            v114 = 0x249249249249249;
            v124 -= v116;
            if ((1 + ((v124 - v116) >> 4) * 0x6db6db6db6db6db7) >= 0x249249249249249) goto loc_100000a30;
            v121 = (((((v121 - v116) >> 4) * v113) < 0x124924924924924) ? ((((((v121 - v116) >> 4) * v113) << 1) >= v126) ? ((((v121 - v116) >> 4) * v113) << 1) : v126) : v114);
            if ((((((v121 - v116) >> 4) * v113) < 0x124924924924924) ? ((((((v121 - v116) >> 4) * v113) << 1) >= v126) ? ((((v121 - v116) >> 4) * v113) << 1) : v126) : v114) >= v114) goto loc_100000a38;
            t2 = operator_new((v121 << 7) - (v121 << 4));
            *(t2 + v124) = t2;
            *((t2 + v124) + 8) = t2;
            *((t2 + v124) + 16) = t2;
            *((t2 + v124) + 24) = t2;
            *((t2 + v124) + 32) = t2;
            *((t2 + v124) + 40) = t2;
            *((t2 + v124) + 96) = -1;
            *((t2 + v124) + 104) = 0;
            t3 = memcpy(((t2 + v124) + (((v124 * 0xb6db6db6db6db6db) >> 5) + ((v124 * 0xb6db6db6db6db6db) >> 63)) * v125), v116, v124);
            v121 = (t2 + v121 * v125);
            v124 = ((t2 + v124) + 112);
            if (v116 != 0) {
                v112 = (operator_delete(v116));
            }
            v113 = -1;
            v114 = 0xb6db6db7;
            goto L1;
        } else {
            v126 = 0;
        }
        *((v116 + v126 * v125) + 108) = (*((v116 + v126 * v125) + 108) + 1);
        v113 = 0x9e3779b1;
        v122 = v84;
        if ((v111 & (1<<31)) == 0) continue;
        operator_delete(v88);
        v113 = 0x9e3779b1;
    v126 = v116->value;
    if (!((v116->value & (1<<31)) != 0)) {
        v84 = *((v116 + v126 * 112) + 104);
    } else {
        v84 = 0;
    }
    v126 = v116->data;
    v125 = 0x3c6ef35f;
    if (!((v116->data & (1<<31)) != 0)) {
        v64 = *((v116 + v126 * 112) + 104);
        v126 = v116->item;
        if ((v116->item & (1<<31)) != 0) goto loc_100000820;
        loc_100000800:
        v56 = *((v116 + v126 * 112) + 104);
    } else {
        v64 = 0;
        v126 = v116->item;
        if ((v116->item & (1<<31)) == 0) goto loc_100000800;
        loc_100000820:
        v56 = 0;
    }
    v72 = v123;
    v126 = v116->member;
    if (!((v116->member & (1<<31)) != 0)) {
        v52 = *((v116 + v126 * 112) + 104);
    } else {
        v52 = 0;
    }
    v119 = 112;
    v120 = 0;
    v122 = 0;
    v123 = g_3c6ef361;
    goto loc_10000088c;
    loc_100000868:
    operator_delete();
    v113 = 0x9e3779b1;
    v120 = (v121 + v120);
    v122++;
    v123 += 0x9e3779b1;
    v125 += 0x9e3779b1;
    while (!((v122 + 1) == 3000)) {
        loc_10000088c:
        v104 = 0;
        v126 = (0x3c6ef35f + v122 * v113);
        v117 = v125;
        v121 = (v123 - ((v125 * 0xaaaaaaab) >> 34) * 6);
        v112 = (push_back(&v88, ((((v126 ^ v126) ^ (v126 ^ v126)) & 3) + 97)));
        v126 = (((v126 ^ v126) ^ (v126 ^ v126)) ^ ((v126 ^ v126) ^ (v126 ^ v126)));
        v121--;
        while (v121 != 1) {
        }
        v126 = v111;
        v127 = ((v111 < 0) ? argc : v118);
        v113 = ((v111 < 0) ? v114 : v111);
        if (((v111 >= 0) ? v114 : v111) == 0) {
            v114 = 0;
            v125 = v117;
            v114 = *(((v116 + v114 * v119) + (v127->f0 << 2)) - 388);
            while (!((*(((v116 + v114 * v119) + (v127->f0 << 2)) - 388) & (1<<31)) != 0)) {
                v127++;
                v113--;
                if (v113 != 1) continue;
            }
        } else {
            v114 = 0;
            v125 = v117;
        }
        loc_100000934:
        v121 = ((*((v116 + v114 * v119) + 108) <= 0) ? 0 : (0 + 1));
        if ((v126 & (1<<31)) == 0) continue;
        goto loc_100000868;
    }
    v126 = v116->value100;
    if (!((v116->value100 & (1<<31)) != 0)) {
        v126 = *((v116 + v126 * 112) + 100);
        if (!((*((v116 + v126 * 112) + 100) & (1<<31)) != 0)) {
            v126 = *((v116 + v126 * 112) + 100);
            if (!((*((v116 + v126 * 112) + 100) & (1<<31)) != 0)) {
                v117 = *((v116 + v126 * 112) + 104);
                goto loc_1000009a4;
            }
        }
        v117 = 0;
    } else {
        v117 = 0;
    }
    loc_1000009a4:
    printf("nodes=%zu total_prefix=%d present=%d none=%d chars=%lld\n", (((v124 - v116) >> 4) * 0x6db6db6db6db6db7));
    /* ccmp v120, 3000  (chained == compare) */
    /* ccmp v117, #0  (chained == compare) */
    operator_delete(v116);
    return ((((v114 + v64) + (v64 + v84)) == 3000) ? 0 : (0 + 1));
    loc_100000a30:
    v112 = (__throw_length_error());
    goto loc_100000a3c;
    loc_100000a38:
    v112 = (__throw_bad_array_new_length());
    loc_100000a3c:
    __builtin_trap();
    v117 = argc;
    if (!((v111 & (1<<31)) == 0)) {
        operator_delete(v88);
        if (v116 == 0) goto loc_100000a78;
        loc_100000a64:
        operator_delete(v116);
        _Unwind_Resume(v117);
    }
    if (v116 != 0) goto loc_100000a64;
    loc_100000a78:
    _Unwind_Resume(v117);
    goto loc_100000934;

