/* Converts an unsigned int to binary, octal, and hex string forms by hand. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Writes value in the given base (2..16) into buf as a null-terminated string. */
static void to_base(uint32_t value, int base, char *buf) {
    static const char digits[] = "0123456789abcdef";
    char tmp[64];
    int pos = 0;
    if (value == 0) {
        tmp[pos++] = '0';
    }
    while (value > 0) {
        tmp[pos++] = digits[value % (uint32_t)base];
        value /= (uint32_t)base;
    }
    int out = 0;
    while (pos > 0) {
        buf[out++] = tmp[--pos];
    }
    buf[out] = '\0';
}

int main(void) {
    uint32_t samples[] = {0, 5, 42, 255, 1024, 0xDEADu};
    int n = (int)(sizeof(samples) / sizeof(samples[0]));
    char bin[64], oct[64], hex[64];
    for (int i = 0; i < n; i++) {
        to_base(samples[i], 2, bin);
        to_base(samples[i], 8, oct);
        to_base(samples[i], 16, hex);
        printf("%-6u bin=%-16s oct=%-8s hex=%s\n",
               samples[i], bin, oct, hex);
    }
    return 0;
}
