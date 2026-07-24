/* Bit-packing/unpacking: pack RGBA + flags into a u32, then round-trip. */
#include <stdio.h>
#include <stdint.h>

typedef struct { uint8_t r, g, b, a; } Pixel;

static uint32_t pack(Pixel p, uint8_t flags3) {
    return ((uint32_t)p.r << 24) | ((uint32_t)p.g << 16) |
           ((uint32_t)p.b << 8)  | ((uint32_t)(p.a & 0x1F) << 3) |
           (uint32_t)(flags3 & 0x07);
}

static Pixel unpack(uint32_t v, uint8_t *flags3_out) {
    Pixel p;
    p.r = (uint8_t)((v >> 24) & 0xFF);
    p.g = (uint8_t)((v >> 16) & 0xFF);
    p.b = (uint8_t)((v >> 8)  & 0xFF);
    p.a = (uint8_t)((v >> 3)  & 0x1F);
    *flags3_out = (uint8_t)(v & 0x07);
    return p;
}

int main(void) {
    uint32_t mismatches = 0, total = 0;
    for (int i = 0; i < 64; i++) {
        Pixel in = { (uint8_t)(i * 4), (uint8_t)(255 - i), (uint8_t)(i ^ 0x2A),
                     (uint8_t)(i & 0x1F) };
        uint8_t fin = (uint8_t)(i & 0x07);
        uint32_t packed = pack(in, fin);
        uint8_t fout; Pixel out = unpack(packed, &fout);
        total++;
        if (out.r != in.r || out.g != in.g || out.b != in.b ||
            out.a != in.a || fout != fin) mismatches++;
    }
    printf("checked=%u mismatches=%u\n", total, mismatches);
    return (int)mismatches;
}
