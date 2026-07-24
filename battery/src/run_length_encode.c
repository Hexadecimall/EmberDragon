#include <stdio.h>

// Run-length encode a string into "<char><count>" pairs in an output buffer.
static int rle_encode(const char *src, char *out) {
    int oi = 0, i = 0;
    while (src[i] != '\0') {
        char c = src[i];
        int count = 1;
        while (src[i + count] == c) count++;
        out[oi++] = c;
        // emit decimal count digits (counts here stay < 100)
        if (count >= 10) out[oi++] = (char)('0' + count / 10);
        out[oi++] = (char)('0' + count % 10);
        i += count;
    }
    out[oi] = '\0';
    return oi;
}

int main(void) {
    const char *inputs[] = {"aaabbbcccd", "wwwwwwwwwwww", "abcd", "mmmmnnp"};
    char out[128];
    for (int k = 0; k < 4; k++) {
        int len = rle_encode(inputs[k], out);
        printf("%-14s -> %s (len %d)\n", inputs[k], out, len);
    }
    return 0;
}
