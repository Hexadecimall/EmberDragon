#include <cstdio>
#include <cstring>

int rle_encode(long s, long data);


static const long data_init[2] = {
    4503599627372268, 4503599627372279 
};
static const long data2_init[2] = {
    4503599627372292, 4503599627372297 
};

int main(int argc, char** argv) {
long data[2];
long data2[2];
int i;
int encode;
char buf[64];
    // neon:  ldr    value0, [v233, #0x0]  // = 0xec06000000001000f706000000001000
    memcpy(data, data_init, 16);  // data assembled on the stack via NEON
    // neon:  ldr    value0, [v233, #0x10]  // = 0x04070000000010000907000000001000
    memcpy(data2, data2_init, 16);  // data assembled on the stack via NEON
    i = 0;
    while (i < 4) {
        encode = rle_encode(data[i], buf);
        printf("%-14s -> %s (len %d)\n", data[i], buf, encode);
        i++;
    }
    return 0;
}

int rle_encode(long s, long data) {
    int i;
    int sum;
    char v7;
    int v0;
    i = 0;
    sum = 0;
    while (!(s[sum] == 0)) {
        v7 = s[sum];
        v0 = 1;
        while (!(s[sum + v0] != v7)) {
            v0++;
        }
        i++;
        data[i] = v7;
        if (v0 >= 10) {
            i++;
            data[i] = v0 / 10 + 48;
        }
        i++;
        data[i] = v0 - v0 / 10 * 10 + 48;
        sum += v0;
    }
    data[i] = 0;
    return i;
}

