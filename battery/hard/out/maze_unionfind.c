#include <cstdio>
#include <cstring>

long maze_reachable(struct Pair3* obj, long a, long b, long c, long arr, long arr2, long d, long e);
long maze_bfs(struct Pair4* obj, long a, long data, long b, long c);
long Maze_dtor(struct Struct4* obj);
long DisjointSet_dtor(struct Struct5* obj);

static long g_100002000[8192];   // unresolved data global — real bytes pending (lifter)
static long g_100002076[8192];   // unresolved data global — real bytes pending (lifter)

const char* s_main = "main";
const char* s_maze_unionfind_cpp = "maze_unionfind.cpp";
const char* s_str_2 = ".........";
const char* s_str_3 = ".#######.";

static const int k = 8;
static const long data[4] = {
    8589934591, 0, 0, 8589934591 
};
static const long data2[2] = {
    0, 8589934591 
};

struct Struct0 {
    int value;
};
struct Struct1 {
    char value;
    char _pad1[7];
    long data;
    char item;
    char _pad17[15];
    long member;
};
struct Pair {
    long value;
    char _pad8[15];
    char flag;
};
struct Pair2 {
    char flag;
    long value;
};
struct Struct2 {
    int value;
    char _pad4[4];
    long data;
    char _pad16[7];
    char item;
};
int main(int argc, char** argv) {
    struct Struct1* t0;
    long obj;
    long* t1;
    struct Struct2* v311;
    long* v312;
    long* v299;
    long* v300;
    struct Pair* v301;
    struct Pair* v303;
    struct Pair* v304;
    long* v305;
    long* v307;
    long* v302;
    long* v306;
    long ccmp;
    long chained;
    long compare;
    char v151;
    long v128;
    char v175;
    char v199;
    long v48;
    long v296;
    t0 = operator_new(40);
    t0->value = t0;
    t0->data = t0;
    t0->member = 0;
    obj = t0;
    t1 = operator_new(40);
    *(*(t1->f4 + 20) + 36) = 0;
    t0->member = k;
    **g_100002076 = 1;
    v311 = t0;
    v312 = (t0 + 12);
    v299 = (t0 + 8);
    v300 = (t0 + 20);
    v301 = (t0 + 28);
    v303 = (t0 + 24);
    v304 = 0;
    v305 = (t0 + 4);
    v307 = t0;
    v305->f0 = *(v307 + (v304 << 2));
    v302 = *(v307 + (v304 << 2));
    v304 = *(v307 + (*(v307 + (v304 << 2)) << 2));
    v305 = (v307 + (*(v307 + (v304 << 2)) << 2));
    while (*(v307 + (*(v307 + (v304 << 2)) << 2)) != *(v307 + (v304 << 2))) {
    }
    v305 = v299->f0;
    if (v299->f0 == 2) {
        v304 = 2;
    } else {
        v306 = v299;
        v306->f0 = *(v307 + (v305 << 2));
        v304 = *(v307 + (v305 << 2));
        v305 = *(v307 + (*(v307 + (v305 << 2)) << 2));
        v306 = (v307 + (*(v307 + (v305 << 2)) << 2));
        while (*(v307 + (*(v307 + (v305 << 2)) << 2)) != *(v307 + (v305 << 2))) {
        }
    }
    if (v302 == v304) {
        v302 = 9;
    } else {
        *(v307 + (((*(argc + (v302 << 2)) < *(argc + (v304 << 2))) ? v302 : v304) << 2)) = ((*(argc + (v302 << 2)) < *(argc + (v304 << 2))) ? v304 : v302);
        v302 = *(argc + (((*(argc + (v302 << 2)) < *(argc + (v304 << 2))) ? v304 : v302) << 2));
        v305 = ((*(argc + (v302 << 2)) < *(argc + (v304 << 2))) ? v304 : v302);
        if (!(*(argc + (((*(argc + (v302 << 2)) < *(argc + (v304 << 2))) ? v304 : v302) << 2)) != *(argc + (((*(argc + (v302 << 2)) < *(argc + (v304 << 2))) ? v302 : v304) << 2)))) {
            *(argc + (v305 << 2)) = (v302 + 1);
        }
        v302 = 8;
    }
    v305 = v312->f0;
    if (v312->f0 == 3) {
        v304 = 3;
    } else {
        v306 = v312;
        v306->f0 = *(v307 + (v305 << 2));
        v304 = *(v307 + (v305 << 2));
        v305 = *(v307 + (*(v307 + (v305 << 2)) << 2));
        v306 = (v307 + (*(v307 + (v305 << 2)) << 2));
        while (*(v307 + (*(v307 + (v305 << 2)) << 2)) != *(v307 + (v305 << 2))) {
        }
    }
    v306 = v311->value;
    if (v311->value == 4) {
        v305 = 4;
    } else {
        argv->value = *(v307 + (v306 << 2));
        v305 = *(v307 + (v306 << 2));
        v306 = *(v307 + (*(v307 + (v306 << 2)) << 2));
        while (*(v307 + (*(v307 + (v306 << 2)) << 2)) != *(v307 + (v306 << 2))) {
        }
    }
    if (v304 != v305) {
        *(v307 + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v304 : v305) << 2)) = ((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304);
        v304 = *(argc + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304) << 2));
        v306 = ((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304);
        if (!(*(argc + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304) << 2)) != *(argc + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v304 : v305) << 2)))) {
            *(argc + (v306 << 2)) = (v304 + 1);
        }
        v302--;
    }
    v305 = v300->f0;
    if (v300->f0 == 5) {
        v304 = 5;
    } else {
        v306 = v300;
        v306->f0 = *(v307 + (v305 << 2));
        v304 = *(v307 + (v305 << 2));
        v305 = *(v307 + (*(v307 + (v305 << 2)) << 2));
        v306 = (v307 + (*(v307 + (v305 << 2)) << 2));
        while (*(v307 + (*(v307 + (v305 << 2)) << 2)) != *(v307 + (v305 << 2))) {
        }
    }
    v306 = v303->value;
    if (v303->value == 6) {
        v305 = 6;
    } else {
        argv->value = *(v307 + (v306 << 2));
        v305 = *(v307 + (v306 << 2));
        v306 = *(v307 + (*(v307 + (v306 << 2)) << 2));
        while (*(v307 + (*(v307 + (v306 << 2)) << 2)) != *(v307 + (v306 << 2))) {
        }
    }
    if (v304 != v305) {
        *(v307 + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v304 : v305) << 2)) = ((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304);
        v304 = *(argc + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304) << 2));
        v306 = ((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304);
        if (!(*(argc + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v305 : v304) << 2)) != *(argc + (((*(argc + (v304 << 2)) < *(argc + (v305 << 2))) ? v304 : v305) << 2)))) {
            *(argc + (v306 << 2)) = (v304 + 1);
        }
        v302--;
    }
    v305 = v303->value;
    if (v303->value == 6) {
        v304 = 6;
    } else {
        v303->value = *(v307 + (v305 << 2));
        v303 = (v307 + (*(v307 + (v305 << 2)) << 2));
        v304 = *(v307 + (v305 << 2));
        v305 = *(v307 + (*(v307 + (v305 << 2)) << 2));
        while (*(v307 + (*(v307 + (v305 << 2)) << 2)) != *(v307 + (v305 << 2))) {
        }
    }
    v305 = v301->value;
    if (v301->value == 7) {
        v303 = 7;
    } else {
        v306 = v301;
        v306->f0 = *(v307 + (v305 << 2));
        v303 = *(v307 + (v305 << 2));
        v305 = *(v307 + (*(v307 + (v305 << 2)) << 2));
        v306 = (v307 + (*(v307 + (v305 << 2)) << 2));
        while (*(v307 + (*(v307 + (v305 << 2)) << 2)) != *(v307 + (v305 << 2))) {
        }
    }
    if (v304 != v303) {
        *(v307 + (((*(argc + (v304 << 2)) < *(argc + (v303 << 2))) ? v304 : v303) << 2)) = ((*(argc + (v304 << 2)) < *(argc + (v303 << 2))) ? v303 : v304);
        v303 = *(argc + (((*(argc + (v304 << 2)) < *(argc + (v303 << 2))) ? v303 : v304) << 2));
        v305 = ((*(argc + (v304 << 2)) < *(argc + (v303 << 2))) ? v303 : v304);
        if (!(*(argc + (((*(argc + (v304 << 2)) < *(argc + (v303 << 2))) ? v303 : v304) << 2)) != *(argc + (((*(argc + (v304 << 2)) < *(argc + (v303 << 2))) ? v304 : v303) << 2)))) {
            *(argc + (v305 << 2)) = (v303 + 1);
        }
        v302--;
    }
    v304 = v301->value;
    if (v301->value == 7) {
        v303 = 7;
    } else {
        v301->value = *(v307 + (v304 << 2));
        v301 = (v307 + (*(v307 + (v304 << 2)) << 2));
        v303 = *(v307 + (v304 << 2));
        v304 = *(v307 + (*(v307 + (v304 << 2)) << 2));
        while (*(v307 + (*(v307 + (v304 << 2)) << 2)) != *(v307 + (v304 << 2))) {
        }
    }
    v304 = v300->f0;
    if (v300->f0 == 5) {
        v301 = 5;
    } else {
        v300->f0 = *(v307 + (v304 << 2));
        v300 = (v307 + (*(v307 + (v304 << 2)) << 2));
        v301 = *(v307 + (v304 << 2));
        v304 = *(v307 + (*(v307 + (v304 << 2)) << 2));
        while (*(v307 + (*(v307 + (v304 << 2)) << 2)) != *(v307 + (v304 << 2))) {
        }
    }
    if (v303 != v301) {
        *(v307 + (((*(argc + (v303 << 2)) < *(argc + (v301 << 2))) ? v303 : v301) << 2)) = ((*(argc + (v303 << 2)) < *(argc + (v301 << 2))) ? v301 : v303);
        v300 = ((*(argc + (v303 << 2)) < *(argc + (v301 << 2))) ? v301 : v303);
        v301 = *(argc + (((*(argc + (v303 << 2)) < *(argc + (v301 << 2))) ? v301 : v303) << 2));
        if (!(*(argc + (((*(argc + (v303 << 2)) < *(argc + (v301 << 2))) ? v301 : v303) << 2)) != *(argc + (((*(argc + (v303 << 2)) < *(argc + (v301 << 2))) ? v303 : v301) << 2)))) {
            *(argc + (v300 << 2)) = (v301 + 1);
        }
        v302--;
    }
    if (v302 == 5) {
        v301 = v307->f0;
        if (v307->f0 != 0) {
            v302 = v307;
            v302->f0 = *(v307 + (v301 << 2));
            v300 = *(v307 + (v301 << 2));
            v301 = *(v307 + (*(v307 + (v301 << 2)) << 2));
            v302 = (v307 + (*(v307 + (v301 << 2)) << 2));
            while (*(v307 + (*(v307 + (v301 << 2)) << 2)) != *(v307 + (v301 << 2))) {
            }
        } else {
            v300 = 0;
        }
        v302 = v299->f0;
        if (v299->f0 == 2) {
            v301 = 2;
        } else {
            v303 = v299;
            v303->value = *(v307 + (v302 << 2));
            v301 = *(v307 + (v302 << 2));
            v302 = *(v307 + (*(v307 + (v302 << 2)) << 2));
            v303 = (v307 + (*(v307 + (v302 << 2)) << 2));
            while (*(v307 + (*(v307 + (v302 << 2)) << 2)) != *(v307 + (v302 << 2))) {
            }
        }
        if (v300 != v301) goto loc_1000010d8;
        v301 = v307->f0;
        if (v307->f0 != 0) {
            v302 = v307;
            v302->f0 = *(v307 + (v301 << 2));
            v300 = *(v307 + (v301 << 2));
            v301 = *(v307 + (*(v307 + (v301 << 2)) << 2));
            v302 = (v307 + (*(v307 + (v301 << 2)) << 2));
            while (*(v307 + (*(v307 + (v301 << 2)) << 2)) != *(v307 + (v301 << 2))) {
            }
        } else {
            v300 = 0;
        }
        v302 = v312->f0;
        if (v312->f0 == 3) {
            v301 = 3;
        } else {
            v303 = v312;
            v303->value = *(v307 + (v302 << 2));
            v301 = *(v307 + (v302 << 2));
            v302 = *(v307 + (*(v307 + (v302 << 2)) << 2));
            v303 = (v307 + (*(v307 + (v302 << 2)) << 2));
            while (*(v307 + (*(v307 + (v302 << 2)) << 2)) != *(v307 + (v302 << 2))) {
            }
        }
        if (v300 == v301) goto loc_1000010e8;
        v301 = v299->f0;
        if (v299->f0 == 2) {
            v300 = 2;
        } else {
            v299->f0 = *(v307 + (v301 << 2));
            v299 = (v307 + (*(v307 + (v301 << 2)) << 2));
            v300 = *(v307 + (v301 << 2));
            v301 = *(v307 + (*(v307 + (v301 << 2)) << 2));
            while (*(v307 + (*(v307 + (v301 << 2)) << 2)) != *(v307 + (v301 << 2))) {
            }
        }
        v301 = v312->f0;
        if (v312->f0 == 3) {
            v299 = 3;
        } else {
            v312->f0 = *(v307 + (v301 << 2));
            v312 = (v307 + (*(v307 + (v301 << 2)) << 2));
            v299 = *(v307 + (v301 << 2));
            v301 = *(v307 + (*(v307 + (v301 << 2)) << 2));
            while (*(v307 + (*(v307 + (v301 << 2)) << 2)) != *(v307 + (v301 << 2))) {
            }
        }
        if (v300 != v299) {
            *(v307 + (((*(argc + (v300 << 2)) < *(argc + (v299 << 2))) ? v300 : v299) << 2)) = ((*(argc + (v300 << 2)) < *(argc + (v299 << 2))) ? v299 : v300);
            v312 = ((*(argc + (v300 << 2)) < *(argc + (v299 << 2))) ? v299 : v300);
            v301 = *(argc + (((*(argc + (v300 << 2)) < *(argc + (v299 << 2))) ? v299 : v300) << 2));
            if (!(*(argc + (((*(argc + (v300 << 2)) < *(argc + (v299 << 2))) ? v299 : v300) << 2)) != *(argc + (((*(argc + (v300 << 2)) < *(argc + (v299 << 2))) ? v300 : v299) << 2)))) {
                *(argc + (v312 << 2)) = (v301 + 1);
            }
        }
        v301 = v307->f0;
        if (v307->f0 != 0) {
            v302 = v307;
            v302->f0 = *(v307 + (v301 << 2));
            v312 = *(v307 + (v301 << 2));
            v301 = *(v307 + (*(v307 + (v301 << 2)) << 2));
            v302 = (v307 + (*(v307 + (v301 << 2)) << 2));
            while (*(v307 + (*(v307 + (v301 << 2)) << 2)) != *(v307 + (v301 << 2))) {
            }
        } else {
            v312 = 0;
        }
        v302 = v311->value;
        if (v311->value == 4) {
            v301 = 4;
        } else {
            v311->value = *(v307 + (v302 << 2));
            v311 = (v307 + (*(v307 + (v302 << 2)) << 2));
            v301 = *(v307 + (v302 << 2));
            v302 = *(v307 + (*(v307 + (v302 << 2)) << 2));
            while (*(v307 + (*(v307 + (v302 << 2)) << 2)) != *(v307 + (v302 << 2))) {
            }
        }
        /* ccmp v300, v299  (chained == compare) */
        if (v312 == v301) goto loc_1000010f4;
        v151 = 9;
        v128 = "S........";
        v175 = 9;
        v199 = 9;
        __assign_with_size(&v48, &v128, &v296, 7);
        v312 = 46;
        if (!((9 & (1<<31)) != 0)) {
            if ((9 & (1<<31)) != 0) goto loc_100000ef8;
            L1:
            if ((9 & (1<<31)) != 0) goto loc_100000f08;
            L2:
            if ((9 & (1<<31)) != 0) goto loc_100000f18;
            L3:
            if ((v199 & (1<<31)) != 0) goto loc_100000f28;
            L4:
            if ((v175 & (1<<31)) != 0) goto loc_100000f38;
            L5:
            v311 = v151;
            if ((v151 & (1<<31)) != 0) goto loc_100000f48;
            L6:
            v299 = (((v312 - v311) >> 3) * 0xaaaaaaaaaaaaaaab);
            if ((((v312 - v311) >> 3) * 0xaaaaaaaaaaaaaaab) < 1) goto loc_100001164;
            v312 = v311->item;
            v299 &= 0x7fffffff;
            v300 = 0;
            v301 = 24;
            v302 = 0;
            v303 = (v311 + v300 * v301);
            v304 = v312;
            while (!((v312 & (1<<7)) == 0)) {
                        v304 = v311->data;
                    }
                    if (v302 < v304) continue;
                }
                v300++;
                if ((v300 + 1) != v299) continue;
            }
            v300 = 1;
            goto L7;
            v300 = (((v300 | v302) == 0) ? 0 : (0 + 1));
            L7:
            v301 = 0;
            v302 = 24;
            v303 = 0;
            v304 = (v311 + v301 * v302);
            v305 = v312;
            while (!((v312 & (1<<7)) == 0)) {
                        v305 = v311->data;
                    }
                    if (v303 < v305) continue;
                }
                v301++;
                if ((v301 + 1) != v299) continue;
            if (v300 != 0) goto loc_100001164;
            goto loc_100001170;
            if ((v300) != 0) goto loc_100001164;
            if (v301 != 6) goto loc_100001170;
            if (v303 != 8) goto loc_100001170;
            t4 = maze_bfs(&v48, 0, 0, 6, 8);
            if (!((t4 == 1) ? 0 : (maze_reachable(&v48, 0, 0, 6, 8)))) goto loc_100001118;
            if (argc != 14) goto loc_10000110c;
            v151 = 5;
            v128 = 0x2e232e53;
            v175 = 5;
            v152 = 0x2e232e2e;
            v199 = 5;
            v176 = 0x2e232e2e;
            __assign_with_size(&v24, &v128, &s_str_3, 3);
            v312 = 0x2e232e2e;
            if ((v199 & (1<<31)) != 0) goto loc_100000f54;
            if ((v175 & (1<<31)) != 0) goto loc_100000f64;
            loc_100000df4:
            v311 = v151;
            if ((v151 & (1<<31)) != 0) goto loc_100000f74;
            loc_100000dfc:
            v299 = (((v312 - v311) >> 3) * 0xaaaaaaaaaaaaaaab);
            if ((((v312 - v311) >> 3) * 0xaaaaaaaaaaaaaaab) < 1) goto loc_100000f80;
            v312 = v311->item;
            v299 &= 0x7fffffff;
            v300 = 24;
            v307 = 0;
            v301 = (v311 + v307 * v300);
            v302 = v312;
            v308 = 0;
            while (!((v312 & (1<<7)) == 0)) {
                        v302 = v311->data;
                    }
                    if (v308 < v302) continue;
                v307++;
                if ((v307 + 1) != v299) continue;
            v307 = -1;
            v308 = -1;
            v300 = 24;
            v309 = 0;
            v301 = (v311 + v309 * v300);
            v302 = v312;
            v310 = 0;
            while (!((v312 & (1<<7)) == 0)) {
                        v302 = v311->data;
                    }
                    if (v310 < v302) continue;
                v309++;
                if ((v309 + 1) != v299) continue;
        } else {
            operator_delete("........G");
            if ((9 & (1<<31)) == 0) goto loc_100000bc4;
            loc_100000ef8:
            operator_delete(v248);
            if ((9 & (1<<31)) == 0) goto loc_100000bcc;
            loc_100000f08:
            operator_delete(s_str_2);
            if ((9 & (1<<31)) == 0) goto loc_100000bd4;
            loc_100000f18:
            operator_delete(s_str_3);
            if ((v199 & (1<<31)) == 0) goto loc_100000bdc;
            loc_100000f28:
            operator_delete(v176);
            if ((v175 & (1<<31)) == 0) goto loc_100000be4;
            loc_100000f38:
            operator_delete(v152);
            v311 = v151;
            if ((v151 & (1<<31)) == 0) goto loc_100000bec;
            loc_100000f48:
            operator_delete(v128);
            goto loc_100000bec;
            loc_100000f54:
            operator_delete(v176);
            if ((v175 & (1<<31)) == 0) goto loc_100000df4;
            loc_100000f64:
            operator_delete(v152);
            v311 = v151;
            if ((v151 & (1<<31)) == 0) goto loc_100000dfc;
            loc_100000f74:
            operator_delete(v128);
            goto loc_100000dfc;
            loc_100000f80:
            v307 = -1;
            v308 = -1;
        v309 = -1;
        v310 = -1;
        if ((maze_reachable(&v24, v307, v308, v309, v310)) != 0) goto loc_100001130;
        if ((maze_bfs(&v24, v307, v308, v309, v310)) != 1) goto loc_100001140;
        printf("maze/union-find ok: comps=%d reach=%d dist=%d\n", 14);
        v307 = v24;
        if (v24 != 0) {
            v308 = v32;
            if (v307 == v32) {
                goto loc_10000102c;
                loc_100001008:
                if (v308 == v307) goto loc_100001028;
            }
            v308 -= 24;
            if ((v308->flag & (1<<31)) == 0) goto loc_100001008;
            operator_delete(v308->value);
            goto loc_100001008;
            loc_100001028:
            loc_10000102c:
            v32 = v307;
            operator_delete();
        }
        v307 = v48;
        if (v48 != 0) {
            v308 = v56;
            if (v307 == v56) {
                goto loc_100001074;
                loc_100001050:
                if (v308 == v307) goto loc_100001070;
            }
            v308 -= 24;
            if ((v308->flag & (1<<31)) == 0) goto loc_100001050;
            operator_delete(v308->value);
            goto loc_100001050;
            loc_100001070:
            loc_100001074:
            v56 = v307;
            operator_delete();
        }
        if (v96 != 0) {
            operator_delete();
        }
        if (obj != 0) {
            operator_delete();
        }
        return 0;
    goto loc_1000010f4;
    loc_1000010d8:
    goto loc_1000010f4;
    loc_1000010e8:
    loc_1000010f4:
    __assert_rtn(&s_main, &s_maze_unionfind_cpp);
    goto loc_100001184;
    loc_10000110c:
    loc_100001118:
    __assert_rtn(&s_main, &s_maze_unionfind_cpp);
    goto loc_100001184;
    loc_100001130:
    goto loc_10000114c;
    loc_100001140:
    loc_10000114c:
    __assert_rtn(&s_main, &s_maze_unionfind_cpp);
    goto loc_100001184;
    loc_100001164:
    loc_100001170:
    __assert_rtn(&s_main, &s_maze_unionfind_cpp);
    loc_100001184:
    __builtin_trap();
    if (!((v199 & (1<<31)) == 0)) {
        operator_delete(v176);
        if ((v175 & (1<<31)) != 0) goto loc_1000011bc;
        loc_1000011a8:
        if ((v151 & (1<<31)) != 0) goto loc_1000011cc;
    } else {
        if ((v175 & (1<<31)) == 0) goto loc_1000011a8;
        loc_1000011bc:
        operator_delete(v152);
        if (!((v151 & (1<<31)) == 0)) {
            loc_1000011cc:
            operator_delete(v128);
        }
    }
    _Unwind_Resume(v307);

struct Pair3 {
    char* data;
    char _pad4[4];
    long value;
};
struct Struct3 {
    long value;
    long data;
    char _pad0[32];
    long item;
    long member;
};
// (anonymous namespace)::maze_reachable((anonymous namespace)::Maze const&, int, int, int, int)
long maze_reachable(struct Pair3* obj, long a, long b, long c, long arr, long arr2, long d, long e) {
    long v29;
    long* v30;
    long v31;
    long v25;
    long v23;
    long v33;
    long v34;
    struct Struct3* v15;
    long v16;
    v29 = *(obj->data + 23);
    v30 = obj->data;
    v31 = (((obj->value - obj->data) >> 3) * 0xaaaaaaaaaaaaaaab);
    if (!((*(obj->data + 23) & (1<<63)) != 0)) {
        v25 = (v29 * v31);
        if ((v29 * v31) == 0) goto L2;
        L1:
        if ((v25 & (1<<31)) != 0) goto loc_1000016f0;
        bzero(v25 << 2);
        bzero(v25 << 2);
        v23 = (operator_new(v25 << 2));
        v24 = (operator_new(v25 << 2));
        if (v25 >= 4) goto L3;
        v33 = 0;
    } else {
        v25 = (v30->f8 * v31);
        if ((v30->f8 * v31) != 0) goto L1;
        L2:
        v23 = 0;
        goto L5;
        L3:
        v34 = g_100002000;
        if (v25 < 16) {
            v33 = 0;
        } else {
            v33 = (v25 & 0x7ffffff0);
            v15 = (v23 + 32);
            v16 = (v25 & 0x7ffffff0);
            v15->value = obj;
            v15->data = arr2;
            v15->item = d;
            v15->member = e;
            v16 -= 16;
            while (v16 != 16) {
            }
            if (v33 == v25) goto L5;
            if (v25 == 0) goto L4;
        }
        v33 = (v25 & 0x7ffffffc);
        v34 = (v33 - (v25 & 0x7ffffffc));
        v15 = (v33 << 2);
        v34 += 4;
        v15 += 16;
        while (v34 != 4) {
        }
        if (v33 == v25) goto L5;
    }
    L4:
    *(v23 + (v33 << 2)) = v33;
    v33++;
    while (v25 != (v33 + 1)) {
    }
    L5:
    if (v31 >= 1) {
        v33 = (v31 & 0x7fffffff);
        v34 = 0;
        v15 = 24;
            v34++;
            v16 = 0;
            goto L7;
            L6:
            v16++;
            L7:
            if (!((v29 & (1<<31)) != 0)) {
                if (v16 < v29) goto loc_1000014a4;
            }
            if (v16 >= v30->f8) continue;
        }
    }
    if (!((v29 & (1<<31)) == 0)) {
        v29 = v30->f8;
    }
    v33 = (v23 + ((b + v29 * a) << 2));
    v25 = (b + v29 * a);
    v26 = (v26 + c * v29);
    v34 = v33->f0;
    while (v33->f0 != v25) {
        v33->f0 = *(v23 + (v34 << 2));
        v33 = (v23 + (*(v23 + (v34 << 2)) << 2));
        v25 = *(v23 + (v34 << 2));
    }
    v33 = (v23 + (v26 << 2));
    v34 = v33->f0;
    while (v33->f0 != v26) {
        v33->f0 = *(v23 + (v34 << 2));
        v33 = (v23 + (*(v23 + (v34 << 2)) << 2));
        v26 = *(v23 + (v34 << 2));
    }
    if (v24 != 0) {
        operator_delete(v24);
    }
    operator_delete(v23);
    return ((v25 != v26) ? 0 : (0 + 1));
    loc_1000016f0:
    operator_delete(v23);
    _Unwind_Resume(__throw_length_error());
    loc_1000014a4:
    if (v16 >= v20) goto loc_100001480;
    v20 = v18;
    if ((v18->flag & (1<<31)) == 0) goto loc_1000014bc;
    v20 = v18->value;
    loc_1000014bc:
    if (*(v20 + v16) == 35) goto loc_100001480;
    if (v34 >= v33) goto loc_100001598;
    v20 = v29;
    if ((v29 & (1<<31)) != 0) goto loc_1000014e4;
    if (v16 < v20) goto loc_1000014f0;
    goto loc_100001598;
    loc_1000014e4:
    if (v16 >= v30->f8) goto loc_100001598;
    loc_1000014f0:
    v20 = v19;
    if ((v19->flag & (1<<31)) == 0) goto loc_100001500;
    v20 = v19->value;
    loc_100001500:
    if (*(v20 + v16) == 35) goto loc_100001598;
    v21 = v29;
    if ((v29 & (1<<31)) == 0) goto loc_100001518;
    v21 = v30->f8;
    loc_100001518:
    v20 = (v16 + v21 * v17);
    v22 = (v23 + ((v16 + v21 * v17) << 2));
    loc_100001520:
    if (v22->f0 == v20) goto loc_10000153c;
    v22->f0 = *(v23 + (obj << 2));
    v20 = *(v23 + (obj << 2));
    v22 = (v23 + (*(v23 + (obj << 2)) << 2));
    goto loc_100001520;
    loc_10000153c:
    v21 = (v16 + v21 * v34);
    v22 = (v23 + ((v16 + v21 * v34) << 2));
    loc_100001544:
    if (v22->f0 == v21) goto loc_100001560;
    v22->f0 = *(v23 + (obj << 2));
    v21 = *(v23 + (obj << 2));
    v22 = (v23 + (*(v23 + (obj << 2)) << 2));
    goto loc_100001544;
    loc_100001560:
    if (v20 == v21) goto loc_100001598;
    *(v23 + (((*(v24 + (v20 << 2)) < *(v24 + (v21 << 2))) ? v20 : v21) << 2)) = ((*(v24 + (v20 << 2)) < *(v24 + (v21 << 2))) ? v21 : v20);
    v20 = *(v24 + (((*(v24 + (v20 << 2)) < *(v24 + (v21 << 2))) ? v21 : v20) << 2));
    v22 = ((*(v24 + (v20 << 2)) < *(v24 + (v21 << 2))) ? v21 : v20);
    if (*(v24 + (((*(v24 + (v20 << 2)) < *(v24 + (v21 << 2))) ? v21 : v20) << 2)) != *(v24 + (((*(v24 + (v20 << 2)) < *(v24 + (v21 << 2))) ? v20 : v21) << 2))) goto loc_100001598;
    *(v24 + (v22 << 2)) = (v20 + 1);
    loc_100001598:
    v21 = v29;
    if ((v29 & (1<<31)) == 0) goto loc_1000015a4;
    v21 = v30->f8;
    loc_1000015a4:
    v20 = (v16 + 1);
    if ((v16 + 1) >= v21) goto loc_100001480;
    v21 = v18;
    if ((v18->flag & (1<<31)) == 0) goto loc_1000015c0;
    v21 = v18->value;
    loc_1000015c0:
    if (*(v21 + v20) == 35) goto loc_100001480;
    v21 = v29;
    if ((v29 & (1<<31)) == 0) goto loc_1000015d8;
    v21 = v30->f8;
    loc_1000015d8:
    v13 = (v23 + (((v21 * v17) + v16) << 2));
    v21 = ((v21 * v17) + v16);
    v22 = (v21 * v17);
    loc_1000015e4:
    if (obj->data == v21) goto loc_100001600;
    obj->data = *(v23 + (arr2 << 2));
    v13 = (v23 + (*(v23 + (arr2 << 2)) << 2));
    v21 = *(v23 + (arr2 << 2));
    goto loc_1000015e4;
    loc_100001600:
    v20 = (v22 + v20);
    v22 = (v23 + ((v22 + v20) << 2));
    loc_100001608:
    if (v22->f0 == v20) goto loc_100001624;
    v22->f0 = *(v23 + (obj << 2));
    v20 = *(v23 + (obj << 2));
    v22 = (v23 + (*(v23 + (obj << 2)) << 2));
    goto loc_100001608;
    loc_100001624:
    if (v21 == v20) goto loc_100001480;
    *(v23 + (((*(v24 + (v21 << 2)) < *(v24 + (v20 << 2))) ? v21 : v20) << 2)) = ((*(v24 + (v21 << 2)) < *(v24 + (v20 << 2))) ? v20 : v21);
    v20 = *(v24 + (((*(v24 + (v21 << 2)) < *(v24 + (v20 << 2))) ? v20 : v21) << 2));
    v22 = ((*(v24 + (v21 << 2)) < *(v24 + (v20 << 2))) ? v20 : v21);
    if (*(v24 + (((*(v24 + (v21 << 2)) < *(v24 + (v20 << 2))) ? v20 : v21) << 2)) != *(v24 + (((*(v24 + (v21 << 2)) < *(v24 + (v20 << 2))) ? v21 : v20) << 2))) goto loc_100001480;
    *(v24 + (v22 << 2)) = (v20 + 1);
    goto loc_100001480;

struct Pair4 {
    char* data;
    long value;
};
struct Pair5 {
    char _pad0[8];
    long value;
    char _pad16[7];
    char flag;
};
// (anonymous namespace)::maze_bfs((anonymous namespace)::Maze const&, int, int, int, int)
long maze_bfs(struct Pair4* obj, long a, long data, long b, long c) {
    struct Pair5* v70;
    long v61;
    struct Pair4* v62;
    struct Pair4* v63;
    long v64;
    long v65;
    struct Pair4* v71;
    long v60;
    long v16;
    long buf;
    long i;
    long v66;
    long v48;
    long ccmp;
    long chained;
    long compare;
    long v69;
    long v67;
    long* v58;
    long v59;
    v70 = (((obj->value - obj->data) >> 3) * 0xaaaaaaaaaaaaaaab);
    v61 = *(obj->data + 23);
    v62 = obj->data;
    v63 = obj;
    v64 = data;
    v65 = a;
    if (!((*(obj->data + 23) & (1<<63)) != 0)) {
        v71 = v61;
    } else {
        v71 = v62->value;
    }
    v70 = ((v70 * v71) << 32);
    if (!(((v70 * v71) << 32) == 0)) {
        if ((v70 & (1<<63)) != 0) goto loc_100001a94;
        memset(255, (v70 >> 30));
        v60 = (operator_new(v70 >> 30));
    } else {
        v60 = 0;
    }
    if (!((v61 & (1<<31)) == 0)) {
        v61 = v62->value;
    }
    *(v60 + ((v64 + v65 * v61) << 2)) = 0;
    __add_back_capacity(&v16);
    *(*(buf + ((((v64 + v65 * v61) + v71) >> 9) << 3)) + ((((v64 + v65 * v61) + v71) & 511) << 3)) = (v65 | v64);
    i++;
    v70 = (i + 1);
    v71 = *(buf + ((((v64 + v65 * v61) + v71) >> 9) << 3));
    if (i >= 1) {
        v61 = -1;
    } else {
        v64 = data;
        v65 = 0xaaaaaaab;
            v70--;
            v71 = buf;
            v66 = *(*(buf + ((v48 >> 9) << 3)) + ((v48 & 511) << 3));
            if (!((v48 + 1) < 1024)) {
                operator_delete(v71->data);
                buf += 8;
                v48 -= 512;
                v70 = (buf + 8);
                v71 = (v48 - 512);
            }
            /* ccmp v70, (v66 >> 32)  (chained == compare) */
            v69 = (v66 >> 32);
            if (v71 == v66) goto L3;
            v61 = 0;
            L1:
            *(*(v70 + ((v71 >> 9) << 3)) + ((v71 & 511) << 3)) = (v62 | v67);
            i++;
            v71 &= 511;
            L2:
            v61 += 4;
            if ((v61 + 4) == 16) continue;
        }
        v62 = (*(v64 + v61) + v66);
        if (*(v64 + v61) < v66) goto L2;
        v67 = (*(data2 + v61) + v69);
        if (((*(data2 + v61) + v69) & (1<<31)) != 0) goto L2;
        v70 = v63->data;
        v71 = (((v63->value - v63->data) >> 3) * v65);
        if (v62 >= (((v63->value - v63->data) >> 3) * v65)) goto L2;
        v71 = v70->flag;
        if (!((v70->flag & (1<<63)) != 0)) {
            if (v67 >= v71) goto L2;
        } else {
            if (v67 >= v70->value) goto L2;
        }
        v58 = (v70 + v62 * 24);
        v59 = *((v70 + v62 * 24) + 23);
        if (!((*((v70 + v62 * 24) + 23) & (1<<31)) == 0)) {
            v58 = v58->f0;
        }
        if (*(v58 + v67) == 35) goto L2;
        if (!((v71 & (1<<31)) != 0)) {
            v70 = *(v60 + ((v67 + v62 * v71) << 2));
            v58 = (v67 + v62 * v71);
            if (*(v60 + ((v67 + v62 * v71) << 2)) != 1) goto L2;
        } else {
            v70 = (v67 + v62 * v70->value);
            v71 = v70->value;
            if (*(v60 + ((v67 + v62 * v70->value) << 2)) != 1) goto L2;
            v58 = v70;
        }
        *(v60 + (v58 << 2)) = (*(v60 + ((v69 + v71 * v66) << 32)) + 1);
        v71 = (v59 + (*(v60 + ((v69 + v71 * v66) << 32)) + 1));
        v58 = ((v59 == v70) ? 0 : (((v59 - v70) << 6) - 1));
        if (((v59 == v70) ? 0 : (((v59 - v70) << 6) - 1)) != (v59 + (*(v60 + ((v69 + v71 * v66) << 32)) + 1))) goto L1;
        __add_back_capacity(&v16);
        v70 = buf;
        v71 = (v58 + v71);
        goto L1;
        L3:
        v70 = *(v63->data + 23);
        v71 = v63->data;
        if (!((*(v63->data + 23) & (1<<63)) == 0)) {
            v70 = v71->value;
        }
        v61 = *(v60 + ((v69 + v70 * v66) << 32));
    }
    i = 0;
    v70 = ((v63 - v62) >> 3);
    if (!(((v63 - v62) >> 3) < 3)) {
        operator_delete(v62->data);
        buf = (v70 + 8);
        v70 = ((v63 - (v70 + 8)) >> 3);
        v62 = (v70 + 8);
        while (((v63 - (v70 + 8)) >> 3) >= 2) {
        }
    }
    if (v70 != 1) {
        if (v70 != 2) goto loc_100001a24;
        v70 = 512;
    } else {
        v70 = 256;
    }
    v48 = v70;
    loc_100001a24:
    if (v62 != v63) {
        operator_delete(v62->value);
        while (v62 != v63) {
        }
        v71 -= v70;
        if (v71 != v70) {
            v70 + ((v71 + 7) & -8);
        }
    }
    if (v16 != 0) {
        operator_delete();
    }
    if (v60 != 0) {
        operator_delete(v60);
    }
    return v61;
    loc_100001a94:
    __throw_length_error();
    operator_delete(v60);
    _Unwind_Resume(obj);

struct Struct4 {
    long value;
};
// (anonymous namespace)::Maze::Maze_dtor()
long Maze_dtor(struct Struct4* obj) {
    struct Pair4* ret;
    long v1;
    struct Pair2* v2;
    ret = obj;
    v1 = obj->value;
    if (obj->value != 0) {
        v2 = ret->value;
        if (v1 == ret->value) {
            goto L3;
            L1:
            if (v2 == v1) goto L2;
        }
        v2 -= 24;
        if ((v2->flag & (1<<31)) == 0) goto L1;
        operator_delete(v2->value);
        goto L1;
        L2:
        L3:
        ret->value = v1;
        operator_delete();
    }
    return ret;
}

struct Struct5 {
    char _pad0[24];
    long value;
};
struct Struct6 {
    long value;
    long data;
    char _pad16[16];
    long item;
};
// (anonymous namespace)::DisjointSet::DisjointSet_dtor()
long DisjointSet_dtor(struct Struct5* obj) {
    struct Struct6* ret;
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

