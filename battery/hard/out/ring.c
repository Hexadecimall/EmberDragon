#include <cstdint>

long OUTLINED_FUNCTION_0();
long main.cold.1();
long main.cold.2();
long main.cold.3();
long main.cold.4();
long main.cold.5();
long main.cold.6();
long main.cold.7();
long main.cold.8();

const char* s_main = "main";
const char* s_rb_empty = "rb.empty()";

int main(int argc, char** argv) {
    long v82;
    long v72;
    long v74;
    long v64;
    long* v83;
    long v0;
    long v75;
    long v76;
    long v78;
    long v80;
    long v81;
    long v79;
    v82 = __atomic_exchange_n(&v72, 0);
    v74 = __atomic_exchange_n(&v64, 0);
    if (!(__atomic_exchange_n(&v64, 0) != __atomic_exchange_n(&v72, 0))) {
        v82 = v64;
        v83 = ((v64 + 1) - __atomic_exchange_n(v83, 0));
        v74 = (v64 + 1);
        if (((v64 + 1) - __atomic_exchange_n(v83, 0)) >= 9) goto loc_100000a48;
        *(&v0 + ((v82 & 7) << 3)) = 0;
        v64 = v74;
        v83 = &v64;
        v75 = (v64 + 1);
        v76 = v64;
        if (((v64 + 1) - __atomic_exchange_n(&v72, 0)) >= 8) goto loc_100000a48;
        *(v82 + ((v76 & 7) << 3)) = 1;
        v83->f0 = v75;
        v82 = v64;
        v83 = ((v64 + 1) - __atomic_exchange_n(v74, 0));
        v75 = (v64 + 1);
        if (((v64 + 1) - __atomic_exchange_n(v74, 0)) >= 8) goto loc_100000a48;
        *(&v0 + ((v82 & 7) << 3)) = 2;
        v64 = v75;
        v83 = &v64;
        v74 = &v72;
        v75 = (v64 + 1);
        v76 = v64;
        if (((v64 + 1) - __atomic_exchange_n(&v72, 0)) >= 8) goto loc_100000a48;
        *(v82 + ((v76 & 7) << 3)) = 3;
        v83->f0 = v75;
        v82 = v64;
        v83 = ((v64 + 1) - __atomic_exchange_n(v74, 0));
        v75 = (v64 + 1);
        if (((v64 + 1) - __atomic_exchange_n(v74, 0)) >= 8) goto loc_100000a48;
        *(&v0 + ((v82 & 7) << 3)) = 4;
        v64 = v75;
        v83 = &v64;
        v74 = &v72;
        v75 = (v64 + 1);
        v76 = v64;
        if (((v64 + 1) - __atomic_exchange_n(&v72, 0)) >= 8) goto loc_100000a48;
        *(v82 + ((v76 & 7) << 3)) = 5;
        v83->f0 = v75;
        v82 = v64;
        v83 = ((v64 + 1) - __atomic_exchange_n(v74, 0));
        v75 = (v64 + 1);
        if (((v64 + 1) - __atomic_exchange_n(v74, 0)) >= 8) goto loc_100000a48;
        *(&v0 + ((v82 & 7) << 3)) = 6;
        v64 = v75;
        v83 = &v64;
        v74 = &v72;
        v75 = (v64 + 1);
        v76 = v64;
        if (((v64 + 1) - __atomic_exchange_n(&v72, 0)) >= 8) goto loc_100000a48;
        *(v82 + ((v76 & 7) << 3)) = 7;
        v83->f0 = v75;
        v82 = (__atomic_exchange_n(v83, 0) - __atomic_exchange_n(v74, 0));
        v83 = __atomic_exchange_n(v74, 0);
        v76 &= 7;
        if ((__atomic_exchange_n(v83, 0) - __atomic_exchange_n(v74, 0)) != 8) goto loc_100000a38;
        v83 = (v64 + 1);
        v74 = v64;
        if (((v64 + 1) - __atomic_exchange_n(&v72, 0)) < 9) goto loc_100000a50;
        v82 = __atomic_exchange_n((v82 + 64), 0);
        v83 = v72;
        if (v72 == __atomic_exchange_n((v82 + 64), 0)) goto loc_100000a4c;
        v72 = (v83 + 1);
        v82 = *(&v0 + ((v83 & 7) << 3));
        v83++;
        if (!(*(&v0 + ((v83 & 7) << 3)) != 0)) {
            v82 = v72;
            v83 = __atomic_exchange_n((v74 + 64), 0);
            if (v72 == __atomic_exchange_n((v74 + 64), 0)) goto loc_100000a4c;
            v72 = (v82 + 1);
            v82++;
            v74 = *(&v0 + ((v82 & 7) << 3));
            if (!(*(&v0 + ((v82 & 7) << 3)) != 1)) {
                v82 = v72;
                v83 = __atomic_exchange_n((v83 + 64), 0);
                if (v72 == __atomic_exchange_n((v83 + 64), 0)) goto loc_100000a4c;
                v72 = (v82 + 1);
                v82++;
                v74 = *(&v0 + ((v82 & 7) << 3));
                if (!(*(&v0 + ((v82 & 7) << 3)) != 2)) {
                    v82 = v72;
                    v83 = __atomic_exchange_n((v83 + 64), 0);
                    if (v72 == __atomic_exchange_n((v83 + 64), 0)) goto loc_100000a4c;
                    v72 = (v82 + 1);
                    v82++;
                    v74 = *(&v0 + ((v82 & 7) << 3));
                    if (!(*(&v0 + ((v82 & 7) << 3)) != 3)) {
                        v82 = v72;
                        v83 = __atomic_exchange_n((v83 + 64), 0);
                        if (v72 == __atomic_exchange_n((v83 + 64), 0)) goto loc_100000a4c;
                        v72 = (v82 + 1);
                        v82++;
                        v74 = *(&v0 + ((v82 & 7) << 3));
                        if (!(*(&v0 + ((v82 & 7) << 3)) != 4)) {
                            v82 = v72;
                            v83 = __atomic_exchange_n((v83 + 64), 0);
                            if (v72 == __atomic_exchange_n((v83 + 64), 0)) goto loc_100000a4c;
                            v72 = (v82 + 1);
                            v82++;
                            v74 = *(&v0 + ((v82 & 7) << 3));
                            if (!(*(&v0 + ((v82 & 7) << 3)) != 5)) {
                                v82 = v72;
                                v83 = __atomic_exchange_n((v83 + 64), 0);
                                if (v72 == __atomic_exchange_n((v83 + 64), 0)) goto loc_100000a4c;
                                v72 = (v82 + 1);
                                v82++;
                                v74 = *(&v0 + ((v82 & 7) << 3));
                                if (!(*(&v0 + ((v82 & 7) << 3)) != 6)) {
                                    v82 = v72;
                                    v83 = __atomic_exchange_n((v83 + 64), 0);
                                    if (v72 == __atomic_exchange_n((v83 + 64), 0)) goto loc_100000a4c;
                                    v72 = (v82 + 1);
                                    v74 = *(&v0 + ((v82 & 7) << 3));
                                    if (!(*(&v0 + ((v82 & 7) << 3)) != 7)) {
                                        v82 = __atomic_exchange_n(v82, 0);
                                        v83 = __atomic_exchange_n((v83 + 64), 0);
                                        if (__atomic_exchange_n((v83 + 64), 0) != __atomic_exchange_n(v82, 0)) goto loc_100000a3c;
                                        v82 = 0;
                                        v83 = 0;
                                        v74 = 0;
                                        v75 = 1000;
                                        goto L2;
                                            L1:
                                            v83 = (argc + (v74 + (v78 + v83)));
                                            v74 = ((v80 == v81) ? v79 : (v79 + 1));
                                            v75--;
                                            if (v75 == 1) goto L3;
                                            L2:
                                            v78 = (v64 + 1);
                                            v79 = v64;
                                            if (!(((v64 + 1) - __atomic_exchange_n(7, 0)) >= 9)) {
                                                *(v76 + ((v79 & 7) << 3)) = v82;
                                                *(v76 + 64) = v78;
                                                v82++;
                                            }
                                            v78 = (v76 + 72);
                                            v79 = (v64 + 1);
                                            v80 = v64;
                                            if (!(((v64 + 1) - __atomic_exchange_n((v76 + 72), 0)) >= 8)) {
                                                *(v76 + ((v80 & 7) << 3)) = v82;
                                                *(v76 + 64) = v79;
                                                v82++;
                                            }
                                            v79 = (v64 + 1);
                                            v80 = v64;
                                            if (!(((v64 + 1) - __atomic_exchange_n(v78, 0)) >= 8)) {
                                                *(v76 + ((v80 & 7) << 3)) = v82;
                                                *(v76 + 64) = v79;
                                                v82++;
                                            }
                                            v78 = (v76 + 72);
                                            v79 = (v64 + 1);
                                            v80 = v64;
                                            if (!(((v64 + 1) - __atomic_exchange_n((v76 + 72), 0)) >= 8)) {
                                                *(v76 + ((v80 & 7) << 3)) = v82;
                                                *(v76 + 64) = v79;
                                                v82++;
                                            }
                                            v79 = (v64 + 1);
                                            v80 = v64;
                                            if (!(((v64 + 1) - __atomic_exchange_n(v78, 0)) >= 8)) {
                                                *(v76 + ((v80 & 7) << 3)) = v82;
                                                *(v76 + 64) = v79;
                                                v82++;
                                            }
                                            v79 = (v76 + 64);
                                            v80 = v72;
                                            v81 = __atomic_exchange_n((v76 + 64), 0);
                                            if (v72 != __atomic_exchange_n((v76 + 64), 0)) {
                                                *(v76 + 72) = (v80 + 1);
                                                v78 = *(v76 + ((v80 & 7) << 3));
                                            } else {
                                                v78 = 0;
                                            }
                                            v79 = __atomic_exchange_n(v79, 0);
                                            v80 = ((v80 == v81) ? v74 : (v74 + 1));
                                            v81 = v72;
                                            if (v72 != __atomic_exchange_n(v79, 0)) {
                                                *(v76 + 72) = (v81 + 1);
                                                v74 = *(v76 + ((v81 & 7) << 3));
                                            } else {
                                                v74 = 0;
                                            }
                                            v79 = ((v81 == v79) ? v80 : (v80 + 1));
                                            v80 = v72;
                                            v81 = __atomic_exchange_n((v76 + 64), 0);
                                        } while (v72 != __atomic_exchange_n((v76 + 64), 0));
                                        goto L1;
                                        L3:
                                        v76 = v72;
                                        while (v76 != __atomic_exchange_n((v75 + 64), 0)) {
                                            *(v75 + 72) = (v76 + 1);
                                            v83 = (*(v75 + ((v76 & 7) << 3)) + v83);
                                            v74++;
                                            v76 = v72;
                                        }
                                        if (v82 != v74) goto loc_100000a40;
                                        v82 = ((v82 - 1) * v82);
                                        v74 = (v82 - 1);
                                        if (v83 != ((v82 - 1) * v82)) goto loc_100000a44;
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        main.cold.4();
    }
    main.cold.1();
    loc_100000a38:
    main.cold.3();
    loc_100000a3c:
    main.cold.5();
    loc_100000a40:
    main.cold.6();
    loc_100000a44:
    main.cold.7();
    loc_100000a48:
    main.cold.2();
    loc_100000a4c:
    main.cold.8();
    loc_100000a50:
    *(v82 + ((v74 & 7) << 3)) = 999;
    *(v82 + 64) = v83;
    __assert_rtn(&s_main, "ring.cpp", 57, "!rb.push(999)");

long OUTLINED_FUNCTION_0() {
    return &s_main;
}

long main.cold.1() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(51, &s_rb_empty);
}

long main.cold.2() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(55, "rb.push(i)");
}

long main.cold.3() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(56, "rb.size() == 8");
}

long main.cold.4() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(65, "v == static_cast<std::uint64_t>(i)");
}

long main.cold.5() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(68, &s_rb_empty);
}

long main.cold.6() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(87, "produced == consumed");
}

long main.cold.7() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(90, "acc == expect");
}

long main.cold.8() {
    long v0;
    OUTLINED_FUNCTION_0(v0);
    __assert_rtn(64, "ok");
}

