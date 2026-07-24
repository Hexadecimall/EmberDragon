#include <iostream>
#include <string>

using namespace std;

void convert(long a);
long c_to_f(long a);
long c_to_k(long a);
long mi_to_km(long a);
long f_to_c(long a);


static const char str[] = "\n";
static const char str2[] = "(";

struct Pair {
    char _pad0[8];
    long value;
    long data;
};
int main(int argc, char** argv) {
    int ret;
    struct Pair* obj;
    long dtor;
    long v128;
    long value2;
    long value3;
    long v104;
    long str2;
    long data2;
    long v80;
    long v168;
    char v272;
    long v160;
    long v72;
    long v64;
    long v56;
    long v48;
    long v40;
    long v32;
    long v24;
    ret = 0;
    obj = argv;
    if (argc == 3) {
        std::string v224 = obj->value;
        std::string v200 = obj->data;
        stod(&v200, 0);
        // neon:  str    d0, [sp, #0x90]
        // neon:  ldr    d0, [sp, #0x90]
        convert(&v224);
        // neon:  str    d0, [sp, #0x88]
        // neon:  ldr    d0, [sp, #0x88]
        dtor = basic_string_dtor(&v224);
        if (isnan(dtor)) {
            v128 = cerr << "unknown conversion: ";
            value2 = v128 << obj->value;
            value2 << str;
            ret = 1;
            goto L1;
        }
        value3 = cout << obj->value;
        v104 = value3 << str2;
        data2 = v104 << obj->data;
        data2 << ") = ";
        v80 = operator_lshdtor;
        v80 << str;
        ret = 0;
    } else {
        // neon:  ldr    value0, [v298, #0x0]  // = 0x00000000000000000000000000804240
        // neon:  ldr    d0, [v298, #0x10]  // = 0x0000000000005940
        v168 = &v272;
        v160 = &v272 + 24;
        while (v168 != v160) {
            // neon:  ldr    d0, [v297, #0x0]
            // neon:  str    d0, [sp, #0x98]
            // neon:  ldr    d0, [sp, #0x98]
            v72 = operator_lshcout;
            v64 = v72 << "C = ";
            // neon:  ldr    d0, [sp, #0x98]
            c_to_f();
            v56 = operator_lshv64;
            v48 = v56 << "F = ";
            // neon:  ldr    d0, [sp, #0x98]
            c_to_k();
            v40 = operator_lshv48;
            v40 << "K\n";
            v168 += 8;
        }
        v32 = cout << "26.2 mi = ";
        // neon:  fmov   d0, v297
        mi_to_km();
        v24 = operator_lshv32;
        v24 << " km\n";
        ret = 0;
    }
    L1:
    return ret;
}

void convert(std::string const& a, double p1) {
    long v16;
    long result;
    long t38;
    v16 = a;
    // neon:  str    d0, [sp, #0x8]
    if (v16 == "c2f") {
        // neon:  ldr    d0, [sp, #0x8]
        result = c_to_f();
        return;
    } else {
        if (v16 == "f2c") {
            // neon:  ldr    d0, [sp, #0x8]
            result = f_to_c();
            return;
        } else {
            if (v16 == "c2k") {
                // neon:  ldr    d0, [sp, #0x8]
                result = c_to_k();
                return;
            } else {
                t38 = v16 == "mi2km";
                a = t38;
                if (t38) {
                    // neon:  ldr    d0, [sp, #0x8]
                    result = mi_to_km();
                    return;
                } else {
                    // neon:  fmov   d0, v25
                    result = a;
                    return;
                }
            }
        }
    }
    return;
}

long c_to_f(double a) {
    // neon:  str    d0, [sp, #0x8]
    // neon:  ldr    d0, [sp, #0x8]
    // neon:  neon.0x1e645001
    // neon:  neon.0x1e610800
    // neon:  neon.0x1e629001
    // neon:  neon.0x1e611800
    // neon:  fmov   d1, v0
    // neon:  neon.0x1e612800
    return a;
}

long c_to_k(double a) {
    // neon:  str    d0, [sp, #0x8]
    // neon:  ldr    d0, [sp, #0x8]
    // neon:  fmov   d1, v0
    // neon:  neon.0x1e612800
    return a;
}

long mi_to_km(double a) {
    // neon:  str    d0, [sp, #0x8]
    // neon:  ldr    d0, [sp, #0x8]
    // neon:  fmov   d1, v0
    // neon:  neon.0x1e610800
    return a;
}

long f_to_c(double a) {
    // neon:  str    d0, [sp, #0x8]
    // neon:  ldr    d0, [sp, #0x8]
    // neon:  fmov   d1, v0
    // neon:  neon.0x1e613800
    // neon:  neon.0x1e629001
    // neon:  neon.0x1e610800
    // neon:  neon.0x1e645001
    // neon:  neon.0x1e611800
    return a;
}

