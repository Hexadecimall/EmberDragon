#include <cstdio>

class LCG {
    long total;
    int next() {
        this->total = this->total * 0x5851f42d4c957f2d + 0x14057b7ef767814f;
        return ((unsigned long)(this->total) >> 32);
    }
};

int main(int argc, char** argv) {
    long v120;
    long total;
    long sum;
    int j;
    int v92;
    int v88;
    char v144;
    int v84;
    int ret;
    int i;
    long mean;
    long sd_x100;
    v120 = 0x1234567 + argc;
    // neon:  mvni   v0.16b, #0x0
    total = 0;
    sum = 0;
    j = 0;
    while (j < 4000) {
        v92 = v120.next();
        v88 = (unsigned)(v92) >> 29;
        (&v144)[v88 << 2] = (&v144)[v88 << 2] + 1;
        v84 = v92 - v92 / 100 * 100;
        total += v84;
        sum = sum + (long)(v84) * (long)(v84);
        j++;
    }
    // neon:  neon.0x7e61d800
    // neon:  fmov   d1, v187
    // neon:  neon.0x1e611800
    // neon:  str    d0, [sp, #0x48]
    // neon:  neon.0x7e61d800
    // neon:  neon.0x1e611802
    // neon:  ldr    d0, [sp, #0x48]
    // neon:  ldr    d1, [sp, #0x48]
    // neon:  neon.0x1f418800
    // neon:  str    d0, [sp, #0x40]
    // neon:  ldr    d0, [sp, #0x40]
    // neon:  neon.0x1e61c000
    // neon:  str    d0, [sp, #0x38]
    ret = 0;
    i = 1;
    while (i < 8) {
        if (!((&v144)[i << 2] < (&v144)[ret << 2])) {
            ret = i;
        }
        i++;
    }
    // neon:  ldr    d0, [sp, #0x48]
    // neon:  neon.0x1e6c1002
    // neon:  neon.0x1e622800
    // neon:  neon.0x1e78000d
    // neon:  ldr    d0, [sp, #0x38]
    // neon:  fmov   d1, v187
    // neon:  neon.0x1f410800
    // neon:  neon.0x1e78000c
    printf("mean=%d sd_x100=%d maxbin=%d count=%u\n", mean, sd_x100, ret, (&v144)[ret << 2]);
    return ret;
}

