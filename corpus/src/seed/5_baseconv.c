#include <stdint.h>
#include <string.h>

#define MAX_DIGITS 64

typedef struct NumberString {
    char digits[MAX_DIGITS];
    int length;
    int base;
} NumberString;

static int digit_to_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    return -1;
}

static char value_to_digit(int value) {
    if (value < 10) {
        return (char)('0' + value);
    }
    return (char)('A' + value - 10);
}

uint64_t parse_in_base(const char *text, int base, int *error) {
    uint64_t result;
    int i, value;
    result = 0;
    *error = 0;
    for (i = 0; text[i] != '\0'; i++) {
        value = digit_to_value(text[i]);
        if (value < 0 || value >= base) {
            *error = 1;
            return 0;
        }
        result = result * (uint64_t)base + (uint64_t)value;
    }
    return result;
}

int format_in_base(uint64_t number, int base, NumberString *out) {
    char temp[MAX_DIGITS];
    int count, i, remainder;
    if (base < 2 || base > 36) {
        return -1;
    }
    count = 0;
    if (number == 0) {
        temp[count++] = '0';
    }
    while (number > 0) {
        remainder = (int)(number % (uint64_t)base);
        temp[count++] = value_to_digit(remainder);
        number = number / (uint64_t)base;
    }
    for (i = 0; i < count; i++) {
        out->digits[i] = temp[count - 1 - i];
    }
    out->digits[count] = '\0';
    out->length = count;
    out->base = base;
    return count;
}

int convert_base(const char *text, int from_base, int to_base, NumberString *out) {
    uint64_t value;
    int error;
    value = parse_in_base(text, from_base, &error);
    if (error) {
        return -1;
    }
    return format_in_base(value, to_base, out);
}

int count_significant_bits(uint64_t number) {
    int bits;
    bits = 0;
    while (number > 0) {
        bits++;
        number >>= 1;
    }
    return bits;
}
