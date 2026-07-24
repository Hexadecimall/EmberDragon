#include <cstring>
#include <iostream>

using namespace std;

int letter();


static const double coeffs_init[7] = {
    91.5, 78, 64.25, 88, 99, 55.5, 72 
};
static const char str[] = "\n";

struct Struct0 {
    char _pad0[8];
    long value;
    long data;
    long item;
    char _pad32[24];
    long member;
    long value64;
    char _pad72[32];
    long data104;
    long item112;
};
int main(int argc, char** argv) {
    struct Struct0* obj;
    int ret;
    double coeffs[7];
    long item2;
    long t9;
    long v200;
    long* buf;
    long v216;
    long v224;
    long v232;
    long v240;
    long v144;
    long v136;
    long v112;
    long v104;
    long v96;
    long v88;
    long v80;
    long i2;
    int v68;
    char v223;
    int i;
    long v56;
    long v48;
    long v40;
    long v32;
    int v28;
    long v16;
    obj = &obj;
    ret = 0;
    memcpy(coeffs, coeffs_init, 56);
    obj->member = coeffs;
    obj->value64 = 7;
    vector(&item2, obj->member, obj->value64);
    // neon:  mvni   v0.16b, #0x0
    // neon:  str    d0, [v250, #0x30]
    operator_index(&item2, 0);
    // neon:  ldr    d0, [v251, #0x0]
    // neon:  str    d0, [v250, #0x28]
    operator_index(&item2, 0);
    // neon:  ldr    d0, [v249, #0x0]
    // neon:  str    d0, [v250, #0x20]
    obj->item = &item2;
    obj->data = begin(obj->item);
    obj->value = end(obj->item);
    while (&item2 != coeffs) {
        operator_mul&&item2;
        // neon:  ldr    d0, [v249, #0x0]
        // neon:  str    d0, [v250, #0x0]
        // neon:  ldr    d1, [v250, #0x0]
        // neon:  ldr    d0, [v250, #0x30]
        // neon:  neon.0x1e612800
        // neon:  str    d0, [v250, #0x30]
        // neon:  ldr    d0, [v250, #0x0]
        // neon:  ldr    d1, [v250, #0x28]
        // neon:  neon.0x1e612000
        if ( < ) {
            // neon:  ldr    d0, [v250, #0x0]
            // neon:  str    d0, [v250, #0x28]
        }
        // neon:  ldr    d0, [v250, #0x0]
        // neon:  ldr    d1, [v250, #0x20]
        // neon:  neon.0x1e612000
        if ( > ) {
            // neon:  ldr    d0, [v250, #0x0]
            // neon:  str    d0, [v250, #0x20]
        }
        operator_inc&&item2;
    }
    // neon:  ldr    d0, [v250, #0x30]
    // neon:  str    d0, [sp, #0x98]
    t9 = size(&item2);
    // neon:  ldr    d0, [sp, #0x98]
    // neon:  neon.0x9e630001
    // neon:  neon.0x1e611800
    v200 = t9;
    // neon:  mvni   v0.16b, #0x0
    buf = t9;
    v216 = &item2;
    v224 = begin(v216);
    v232 = end(v216);
    while (v224 != v232) {
        // neon:  ldr    d0, [v249, #0x0]
        v240 = operator_mul&v224;
        // neon:  neon.0x1e613800
        // neon:  str    d0, [sp, #0x118]
        // neon:  ldr    d0, [sp, #0x118]
        // neon:  ldr    d1, [sp, #0x118]
        // neon:  neon.0x1f410800
        buf = v240;
        operator_inc&v224;
    }
    size(&item2);
    // neon:  neon.0x9e630001
    // neon:  neon.0x1e611800
    buf = buf;
    // neon:  neon.0x1e61c000
    // neon:  str    d0, [sp, #0x110]
    v144 = cout << "n=";
    v136 = v144 << size(&item2);
    v136 << str;
    cout << "mean=";
    item2 = operator_lshv200;
    v112 = item2 << " min=";
    // neon:  ldr    d0, [v250, #0x28]
    v104 = operator_lshv112;
    v96 = v104 << " max=";
    // neon:  ldr    d0, [v250, #0x20]
    v88 = operator_lshv96;
    v88 << str;
    v80 = cout << "stddev=";
    // neon:  ldr    d0, [sp, #0x110]
    i2 = operator_lshv80;
    i2 << str;
    obj->data104 = 0;
    obj->item112 = 0;
    i2 = 0;
    v240 = begin(&item2);
    v232 = end(&item2);
    while (v240 != v232) {
        operator_mul&v240;
        // neon:  ldr    d0, [v249, #0x0]
        // neon:  str    d0, [sp, #0xe0]
        // neon:  ldr    d0, [sp, #0xe0]
        v68 = letter();
        v223 = v68;
        (&v88)[v223 - 65 << 2] = (&v88)[v223 - 65 << 2] + (v223 == 70 ? 0 : 1);
        if (v223 == 70) {
            i2++;
        }
        operator_inc&v240;
    }
    buf = "ABCDF";
    i = 0;
    while (i < 5) {
        v56 = cout << buf[i];
        v48 = v56 << ": ";
        v40 = v48 << (&v88)[i << 2];
        v40 << str;
        i++;
    }
    v32 = cout << "class grade: ";
    v28 = letter(v200);
    v16 = v32 << v28;
    v16 << str;
    ret = 0;
    return ret;
}

int letter(double p0) {
    char result;
    // neon:  str    d0, [sp, #0x0]
    // neon:  ldr    d0, [sp, #0x0]
    // neon:  fmov   d1, v16
    // neon:  neon.0x1e612000
    return result;
}

