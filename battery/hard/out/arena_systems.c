long main.cold.1(struct Struct0* obj);
long main.cold.2();
long main.cold.3();
long main.cold.4();
long main.cold.5();

static long g_100000088[8192];   // unresolved data global — real bytes pending (lifter)
static long g_100000089[8192];   // unresolved data global — real bytes pending (lifter)

const char* s_main = "main";
const char* s_arena_systems_cpp = "arena_systems.cpp";

struct Struct0 {
    long value;
    long data;
    char _pad16[16];
    int item;
};
struct Struct1 {
    long value;
    char data;
    char _pad2[6];
    long item;
    char _pad16[16];
    int member;
};
struct Struct2 {
    int value;
    int data;
    int item;
    char _pad4[36];
    int member;
};
int main(int argc, char** argv) {
    struct Struct0* v4104;
    long v47;
    long v4105;
    long v0;
    long v4099;
    struct Struct0* v4100;
    struct Struct1* v4101;
    struct Struct2* v4102;
    long v4103;
    (*__chkstk_darwin)();
    v4104 = (&v47 & -8);
    v4105 = &v0;
    v4099 = (((&v47 & -8) - &v0) + 40);
    if (!((((&v47 & -8) - &v0) + 40) >= 4096)) {
        v4104->value = *g_100000088;
        v4104->data = argc;
        v4104->item = 9;
        v4105 = ((((v4099 + v4105) & -8) - (v4099 + v4105)) + v4099);
        v4100 = ((v4099 + v4105) & -8);
        v4101 = (((((v4099 + v4105) & -8) - (v4099 + v4105)) + v4099) + 2560);
        if (!((((((v4099 + v4105) & -8) - (v4099 + v4105)) + v4099) + 2560) >= 4096)) {
            if (v4100 != 0) {
                v4101 = 0;
                *((v4100 + v4101) + 32) = 0;
                *(v4100 + v4101) = argv;
                *((v4100 + v4101) + 8) = argv;
                v4101 += 40;
                while ((v4101 + 40) != 2560) {
                }
                v4101 = 0;
                v4102 = (v4100 + 112);
                v4102->value = v4101;
                v4102->data = (v4101 + 1);
                v4102->item = (v4101 + 2);
                v4102->member = (v4101 + 3);
                v4101 += 4;
                v4102 += 160;
                while ((v4101 + 4) != 64) {
                }
                v4102 = (v4105 + 40);
                if ((v4105 + 40) >= 4096) goto L2;
                v4100->value = *g_100000089;
                v4100->data = argc;
                v4100->item = 11;
                v4099 = &v0;
                v4100 = (&v0 | 7);
                v4101 = ((v4102 + (&v0 | 7)) & -8);
                v4103 = ((v4105 + (((v4102 + (&v0 | 7)) & -8) - (v4102 + &v0))) + 80);
                if (!(((v4105 + (((v4102 + (&v0 | 7)) & -8) - (v4102 + &v0))) + 80) >= 4096)) {
                    v4105 = 0;
                    v4102 = v4103;
                    while (v4101 != 0) {
                        v4101->value = 0;
                        v4101->item = 0;
                        v4101->member = 0;
                        v4105--;
                        v4101 = ((v4102 + v4100) & -8);
                        v4103 = ((((v4102 + v4100) & -8) - v4099) + 40);
                        if (((((v4102 + v4100) & -8) - v4099) + 40) < 4096) continue;
                    }
                    if (v4105 != 0) {
                        L1:
                        v4105 = (((((v4102 + &v0) + 7) & -8) - &v0) + 40);
                        if (!((((((v4102 + &v0) + 7) & -8) - &v0) + 40) >= 4096)) {
                            if (argc != 0) goto L3;
                        }
                        return argc;
                    }
                }
                main.cold.2();
                goto L4;
            }
        }
        main.cold.4();
    } else {
        main.cold.5();
        L2:
        main.cold.3();
        L3:
        main.cold.1();
    }
    L4:
    __builtin_trap();
    _Unwind_Resume();
    goto L1;
}

long main.cold.1(struct Struct0* obj) {
    obj->item = 0;
    obj->value = obj;
    obj->data = obj;
    __assert_rtn(&s_main, &s_arena_systems_cpp, 90, "arena.make<Particle>() == nullptr");
}

long main.cold.2() {
    __assert_rtn(&s_main, &s_arena_systems_cpp, 89, "count > 0");
}

long main.cold.3() {
    __assert_rtn(&s_main, &s_arena_systems_cpp, 83, "c == reinterpret_cast<Particle*>(scratch)");
}

long main.cold.4() {
    __assert_rtn(&s_main, &s_arena_systems_cpp, 75, "scratch != nullptr");
}

long main.cold.5() {
    __assert_rtn(&s_main, &s_arena_systems_cpp, 68, "a && b");
}

