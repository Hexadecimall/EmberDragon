/*
 * basen.c - Convert unsigned integers to and from arbitrary numeric bases
 * (2 through 36) using the digit alphabet 0-9 then A-Z.
 *
 * Encoding writes into a caller-supplied buffer; decoding parses a string and
 * reports success or the position of the first invalid character.
 */

#include <stdint.h>
#include <stddef.h>

/* Smallest and largest radices this module accepts. */
#define MIN_BASE 2
#define MAX_BASE 36

/*
 * Map a single digit value (0..35) to its character. Values 0-9 become '0'-'9'
 * and 10-35 become 'A'-'Z'. Returns '?' for out-of-range values so a bug is
 * visible rather than silent.
 */
char digit_to_char(int value) {
    if (value >= 0 && value <= 9)
        return (char)('0' + value);
    if (value >= 10 && value <= 35)
        return (char)('A' + (value - 10));
    return '?';
}

/*
 * Map a digit character to its numeric value, accepting both upper- and
 * lowercase letters. Returns the value 0..35, or -1 if the character is not a
 * valid base-36 digit.
 */
int char_to_digit(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    return -1;
}

/*
 * Encode 'value' in the given 'base' into 'out', writing a NUL terminator.
 * 'outSize' is the buffer capacity in bytes. Returns the string length on
 * success, or -1 if the base is out of range or the buffer is too small.
 * The value zero encodes as the single digit "0".
 */
int encode_base(uint64_t value, int base, char *out, size_t outSize) {
    if (base < MIN_BASE || base > MAX_BASE)
        return -1;
    if (outSize == 0)
        return -1;

    /* Generate digits least-significant first, then reverse them. A 64-bit
     * value in base 2 needs at most 64 digits plus the terminator. */
    char scratch[64];
    int len = 0;
    if (value == 0) {
        scratch[len++] = '0';
    } else {
        while (value > 0) {
            int rem = (int)(value % (uint64_t)base);
            scratch[len++] = digit_to_char(rem);
            value /= (uint64_t)base;
        }
    }

    /* Bail out if the result plus terminator would not fit. */
    if ((size_t)len + 1 > outSize)
        return -1;

    /* Reverse the digit run into the output buffer. */
    for (int i = 0; i < len; i++)
        out[i] = scratch[len - 1 - i];
    out[len] = '\0';
    return len;
}

/*
 * Decode the NUL-terminated string 'text' interpreted in 'base' into '*result'.
 * Returns 0 on success. On error returns -1 and, if 'errorPos' is non-NULL,
 * stores the index of the offending character (or 0 for an empty string or a
 * bad base). Overflow of the 64-bit accumulator is reported as an error at the
 * digit that triggered it.
 */
int decode_base(const char *text, int base, uint64_t *result, size_t *errorPos) {
    if (base < MIN_BASE || base > MAX_BASE) {
        if (errorPos) *errorPos = 0;
        return -1;
    }

    uint64_t acc = 0;
    size_t i = 0;
    int sawDigit = 0;
    for (; text[i] != '\0'; i++) {
        int d = char_to_digit(text[i]);
        /* Reject characters that are not digits of this particular base. */
        if (d < 0 || d >= base) {
            if (errorPos) *errorPos = i;
            return -1;
        }
        /* Detect overflow before it happens by checking the headroom. */
        if (acc > (UINT64_MAX - (uint64_t)d) / (uint64_t)base) {
            if (errorPos) *errorPos = i;
            return -1;
        }
        acc = acc * (uint64_t)base + (uint64_t)d;
        sawDigit = 1;
    }

    /* An empty string has no value to report. */
    if (!sawDigit) {
        if (errorPos) *errorPos = 0;
        return -1;
    }
    *result = acc;
    return 0;
}
