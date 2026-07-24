/* Packed RGBA-ish bitfield struct + bit manipulation. */
#include <stdio.h>
#include <stdint.h>

struct Pixel {
    unsigned r : 5;
    unsigned g : 6;
    unsigned b : 5;
    unsigned a : 4;
    unsigned tag : 12;   /* fills out to 32 bits */
};

static unsigned luminance(struct Pixel p) {
    /* weighted, kept integer; bitfields widen to unsigned in the expr */
    return (p.r * 2u + p.g * 5u + p.b * 1u) / 8u;
}

int main(void) {
    struct Pixel px;
    px.r = 31; px.g = 63; px.b = 16; px.a = 9; px.tag = 0xABC;
    px.g -= 20;          /* mutate one field */
    px.r ^= 0x15;        /* xor inside a 5-bit field -> wraps */
    printf("r=%u g=%u b=%u a=%u tag=0x%X lum=%u\n",
           px.r, px.g, px.b, px.a, px.tag, luminance(px));
    printf("sizeof Pixel = %zu\n", sizeof(struct Pixel));
    /* prove the field truncation: assign past width */
    px.a = 20;           /* 20 & 0xF == 4 */
    printf("a after overflow assign = %u\n", px.a);
    return 0;
}
