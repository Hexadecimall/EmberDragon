// Q16.16 fixed-point: mul/div with rounding, a fixed-point sqrt, and a sum.
#include <cstdio>
#include <cstdint>

typedef int32_t fx;                    // Q16.16
static const int SH = 16;

static fx fx_from_int(int v)      { return (fx)(v << SH); }
static int fx_to_int(fx a)        { return (int)(a >> SH); }
static fx fx_mul(fx a, fx b)      { return (fx)(((int64_t)a * b) >> SH); }
static fx fx_div(fx a, fx b)      { return (fx)(((int64_t)a << SH) / b); }

static fx fx_sqrt(fx a) {           // Newton iteration in fixed point
    if (a <= 0) return 0;
    fx x = a > fx_from_int(1) ? a : fx_from_int(1);
    for (int i = 0; i < 24; i++)
        x = (x + fx_div(a, x)) >> 1;    // x = (x + a/x)/2
    return x;
}

int main() {
    fx area = 0;
    for (int i = 1; i <= 50; i++) {
        fx r = fx_div(fx_from_int(i), fx_from_int(7));   // i/7
        fx pi = (fx)(205887);                            // ~3.14159 in Q16.16
        fx a  = fx_mul(pi, fx_mul(r, r));                // pi * r^2
        area += a;
    }
    fx side = fx_sqrt(area);
    // print only integer parts to avoid float I/O
    printf("area_int=%d side_int=%d frac16=%d\n",
           fx_to_int(area), fx_to_int(side), (int)(side & 0xFFFF));
    return fx_to_int(side) & 0x7F;
}
