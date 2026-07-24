#include <cstdio>
#include <cstring>

void rect_translate(long a);
void point_distance(long a, long b);
void rect_area(long a);


static const double coeffs_init[2] = {
    1, 1 
};
static const double coeffs2_init[2] = {
    5, 2 
};

int main(int argc, char** argv) {
    int result;
    double coeffs[2];
    double coeffs2[2];
    char v80;
    char __addr0[64];
    result = 0;
    // neon:  ldr    value0, [v125, #0x0]  // = 0x00000000000008400000000000001040
    // neon:  ldr    value0, [v125, #0x0]  // = 0x000000000000f03f000000000000f03f
    memcpy(coeffs, coeffs_init, 16);  // data assembled on the stack via NEON
    // neon:  ldr    value0, [v125, #0x10]  // = 0x00000000000014400000000000000040
    memcpy(coeffs2, coeffs2_init, 16);  // data assembled on the stack via NEON
    // neon:  neon.0x1e601000
    // neon:  neon.0x1e7c1001
    rect_translate(coeffs);
    point_distance(__addr0, &v80);
    // neon:  str    d0, [v125, #0x0]
    printf("distance = %.2f\n");
    rect_area(coeffs);
    // neon:  str    d0, [v125, #0x0]
    printf("area = %.2f\n");
    // neon:  ldr    d1, [sp, #0x30]
    // neon:  ldr    d0, [sp, #0x38]
    // neon:  str    d1, [v125, #0x0]
    // neon:  str    d0, [v125, #0x8]
    printf("origin = (%.2f, %.2f)\n");
    return result;
}

void rect_translate(long a) {
    // neon:  str    d0, [sp, #0x10]
    // neon:  str    d1, [sp, #0x8]
    // neon:  ldr    d1, [sp, #0x10]
    // neon:  ldr    d0, [v25, #0x0]
    // neon:  neon.0x1e612800
    // neon:  str    d0, [v25, #0x0]
    // neon:  ldr    d1, [sp, #0x8]
    // neon:  ldr    d0, [v25, #0x8]
    // neon:  neon.0x1e612800
    // neon:  str    d0, [v25, #0x8]
    return;
}

void point_distance(long a, long b) {
    // neon:  ldr    d0, [v25, #0x0]
    // neon:  ldr    d1, [v25, #0x0]
    // neon:  neon.0x1e613800
    // neon:  str    d0, [sp, #0x8]
    // neon:  ldr    d0, [v25, #0x8]
    // neon:  ldr    d1, [v25, #0x8]
    // neon:  neon.0x1e613800
    // neon:  str    d0, [sp, #0x0]
    // neon:  ldr    d0, [sp, #0x8]
    // neon:  ldr    d1, [sp, #0x8]
    // neon:  ldr    d2, [sp, #0x0]
    // neon:  ldr    d3, [sp, #0x0]
    // neon:  neon.0x1e630842
    // neon:  neon.0x1f410800
    // neon:  neon.0x1e61c000
    return;
}

void rect_area(long a) {
    // neon:  ldr    d0, [v9, #0x10]
    // neon:  ldr    d1, [v9, #0x18]
    // neon:  neon.0x1e610800
    return;
}

