#include <cstdio>

long luminance(int a);


int main(int argc, char** argv) {
    int result;
    int v72;
    int v48;
    int v52;
    int v56;
    int v60;
    int v64;
    result = 0;
    v72 = v72 & 0xffffffe0 | 31;
    v72 = v72 & 0xfffff81f | 2016;
    v72 = v72 & 0xffff07ff | 32768;
    v72 = v72 & 0xfff0ffff | 589824;
    v72 = v72 & 0xfffff | 0xabc00000;
    v72 = v72 & 0xfffff81f | (((unsigned)(v72) >> 5 & 63) - 20 & 63) << 5;
    v72 = v72 & 0xffffffe0 | (v72 & 31 ^ 21) & 31;
    v48 = v72 & 31;
    v52 = (unsigned)(v72) >> 5 & 63;
    v56 = (unsigned)(v72) >> 11 & 31;
    v60 = (unsigned)(v72) >> 16 & 15;
    v64 = (unsigned)(v72) >> 20;
    printf("r=%u g=%u b=%u a=%u tag=0x%X lum=%u\n", v48, v52, v56, v60, v64, luminance(v72));
    printf("sizeof Pixel = %zu\n", 4);
    v72 = v72 & 0xfff0ffff | 262144;
    printf("a after overflow assign = %u\n", (unsigned)(v72) >> 16 & 15);
    return result;
}

long luminance(int a) {
    return ((((unsigned)(a) >> 5 & 63) * 5 + ((a & 31) << 1) + ((unsigned)(a) >> 11 & 31)) / 8);
}

