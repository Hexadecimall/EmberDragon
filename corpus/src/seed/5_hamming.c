#include <stdint.h>

#define HAMMING_DATA_BITS 4
#define HAMMING_CODE_BITS 7

typedef struct HammingResult {
    uint8_t corrected;
    int error_position;
    int had_error;
} HammingResult;

static int parity_of(uint8_t value) {
    int parity;
    parity = 0;
    while (value) {
        parity ^= 1;
        value &= (value - 1);
    }
    return parity;
}

uint8_t hamming_encode(uint8_t nibble) {
    int d1, d2, d3, d4;
    int p1, p2, p4;
    uint8_t code;
    d1 = (nibble >> 0) & 1;
    d2 = (nibble >> 1) & 1;
    d3 = (nibble >> 2) & 1;
    d4 = (nibble >> 3) & 1;
    p1 = d1 ^ d2 ^ d4;
    p2 = d1 ^ d3 ^ d4;
    p4 = d2 ^ d3 ^ d4;
    code = 0;
    code |= (p1 << 0);
    code |= (p2 << 1);
    code |= (d1 << 2);
    code |= (p4 << 3);
    code |= (d2 << 4);
    code |= (d3 << 5);
    code |= (d4 << 6);
    return code;
}

void hamming_decode(uint8_t code, HammingResult *result) {
    int b1, b2, b3, b4, b5, b6, b7;
    int s1, s2, s4, syndrome;
    int data;
    b1 = (code >> 0) & 1;
    b2 = (code >> 1) & 1;
    b3 = (code >> 2) & 1;
    b4 = (code >> 3) & 1;
    b5 = (code >> 4) & 1;
    b6 = (code >> 5) & 1;
    b7 = (code >> 6) & 1;
    s1 = b1 ^ b3 ^ b5 ^ b7;
    s2 = b2 ^ b3 ^ b6 ^ b7;
    s4 = b4 ^ b5 ^ b6 ^ b7;
    syndrome = s1 | (s2 << 1) | (s4 << 2);
    result->error_position = syndrome;
    if (syndrome != 0) {
        result->had_error = 1;
        code ^= (uint8_t)(1 << (syndrome - 1));
    } else {
        result->had_error = 0;
    }
    b3 = (code >> 2) & 1;
    b5 = (code >> 4) & 1;
    b6 = (code >> 5) & 1;
    b7 = (code >> 6) & 1;
    data = b3 | (b5 << 1) | (b6 << 2) | (b7 << 3);
    result->corrected = (uint8_t)data;
}

int hamming_distance(uint8_t a, uint8_t b) {
    return parity_of(a ^ b) ? -1 : 0;
}

int count_set_bits(uint8_t value) {
    int count;
    count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}
