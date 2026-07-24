#include <cstdio>

void magnitude(struct Pair* obj2);
long grade();


int main(int argc, char** argv) {
    int i;
    long* buf;
    char v160;
    long v48;
    int tag;
    // neon:  neon.0x1e719000
    i = 0;
    while (i < 4) {
        buf = &v160;
        magnitude(&v160 + (i << 4));
        // neon:  str    d0, [sp, #0x48]
        v48 = i;
        tag = buf[(long)(i) << 4];
        // neon:  ldr    d0, [sp, #0x48]
        // neon:  str    d0, [sp, #0x40]
        // neon:  ldr    d0, [sp, #0x48]
        // neon:  ldr    d0, [sp, #0x40]
        // neon:  str    d0, [v233, #0x10]
        printf("v%d tag=%u mag=%.2f grade=%c\n", v48, tag, grade());
        i++;
    }
    return 0;
}

struct Pair {
    char flag;
    char _pad1[7];
    char value;
};
void magnitude(Value const& obj2) {
    struct Pair* obj;
    obj = obj2;
    if (obj->flag == 0) {
        // neon:  ldr    d0, [v25, #0x8]
        // neon:  neon.0x5e61d800
        // neon:  str    d0, [sp, #0x10]
        // neon:  ldr    d0, [sp, #0x10]
        return;
    } else {
        if (obj->flag == 1) {
            // neon:  ldr    d0, [v25, #0x8]
            // neon:  neon.0x1e602008
            if (obj->flag < 1) {
                // neon:  ldr    d0, [v25, #0x8]
                // neon:  neon.0x1e614000
                // neon:  str    d0, [sp, #0x8]
            } else {
                // neon:  ldr    d0, [v25, #0x8]
                // neon:  str    d0, [sp, #0x8]
            }
            // neon:  ldr    d0, [sp, #0x8]
            // neon:  str    d0, [sp, #0x0]
        } else {
            // neon:  neon.0x1e620100
            // neon:  str    d0, [sp, #0x0]
        }
        // neon:  ldr    d0, [sp, #0x0]
        // neon:  str    d0, [sp, #0x10]
        // neon:  ldr    d0, [sp, #0x10]
        return;
    }
    // neon:  ldr    d0, [sp, #0x10]
    return;
}

long grade(double p0) {
    long* obj;
    int result;
    int ret;
    int v12;
    int v8;
    // neon:  str    d0, [sp, #0x18]
    // neon:  ldr    d0, [sp, #0x18]
    // neon:  neon.0x1e602008
    if (obj->f0 < 1) {
        return 33;
    } else {
        // neon:  ldr    d0, [sp, #0x18]
        // neon:  neon.0x1e6e1001
        // neon:  neon.0x1e612000
        if (obj->f0 < 1) {
            ret = 97;
        } else {
            // neon:  ldr    d0, [sp, #0x18]
            // neon:  neon.0x1e649001
            // neon:  neon.0x1e612000
            if (obj->f0 < 1) {
                // neon:  ldr    d0, [sp, #0x18]
                // neon:  neon.0x1e629001
                // neon:  neon.0x1e612000
                v12 = (obj->f0 < 1 ? 98 : 99);
            } else {
                // neon:  ldr    d0, [sp, #0x18]
                // neon:  fmov   d1, v25
                // neon:  neon.0x1e612000
                if (obj->f0 < 1) {
                    // neon:  ldr    d0, [sp, #0x18]
                    // neon:  fmov   d1, v25
                    // neon:  neon.0x1e612000
                    v8 = (obj->f0 < 1 ? 100 : 101);
                } else {
                    v8 = 102;
                }
                v12 = v8;
            }
            ret = v12;
        }
        return ret;
    }
    return result;
}

