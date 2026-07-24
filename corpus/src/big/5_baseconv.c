/*
 * baseconv.c — Convert unsigned integers to and from arbitrary text bases.
 *
 * Supports any radix from 2 through 36 using the digit alphabet 0-9 then
 * A-Z. Provides encoding of a 64-bit value into a caller-supplied buffer and
 * parsing of a string back into a value with explicit error reporting.
 */

#include <stddef.h>
#include <stdint.h>

/* Digit alphabet: index i is the symbol for digit value i. */
static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
 * Map a single character to its digit value in the given base. Accepts both
 * upper- and lowercase letters. Returns the digit value [0, base), or -1 if
 * the character is not a valid digit in this base.
 */
static int digit_value(char c, int base) {
    int v;
    if (c >= '0' && c <= '9')
        v = c - '0';
    else if (c >= 'A' && c <= 'Z')
        v = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z')
        v = c - 'a' + 10; /* fold lowercase to the same value as uppercase */
    else
        return -1;
    /* A digit is only valid if it is strictly less than the radix. */
    return (v < base) ? v : -1;
}

/*
 * Encode `value` into `out` as a base-`base` string. Writes a NUL-terminated
 * result and returns the number of characters written (excluding the NUL),
 * or 0 on invalid input (base out of [2,36] or out_size too small).
 *
 * Because remainders are produced least-significant digit first, the digits
 * are emitted into a temporary buffer and then reversed into `out`.
 */
size_t base_encode(uint64_t value, int base, char *out, size_t out_size) {
    if (base < 2 || base > 36 || out_size == 0)
        return 0;

    char tmp[65]; /* enough for 64 binary digits plus slack */
    size_t n = 0;

    /* Generate digits low-to-high; a value of 0 must still emit one '0'. */
    do {
        tmp[n++] = DIGITS[value % (uint64_t)base];
        value /= (uint64_t)base;
    } while (value != 0);

    /* Need room for n digits plus the terminating NUL. */
    if (n + 1 > out_size)
        return 0;

    /* Reverse the temporary digits into the output buffer. */
    for (size_t i = 0; i < n; i++)
        out[i] = tmp[n - 1 - i];
    out[n] = '\0';
    return n;
}

/*
 * Parse the base-`base` string `text` into `*result`. Returns 1 on success.
 * Returns 0 (leaving *result untouched) if the base is out of range, the
 * string is empty, or it contains a character not valid for the base.
 *
 * Note: overflow of the 64-bit accumulator is not detected; inputs are
 * assumed to fit. Complexity is O(length of text).
 */
int base_parse(const char *text, int base, uint64_t *result) {
    if (base < 2 || base > 36 || text == NULL || *text == '\0')
        return 0;

    uint64_t acc = 0;
    for (const char *p = text; *p != '\0'; p++) {
        int d = digit_value(*p, base);
        if (d < 0)
            return 0; /* reject the whole string on the first bad digit */
        acc = acc * (uint64_t)base + (uint64_t)d;
    }
    *result = acc;
    return 1;
}

/*
 * Convert a number's textual representation directly from one base to
 * another. Parses `text` in `from_base`, then re-encodes into `out` using
 * `to_base`. Returns the length written, or 0 if parsing or encoding fails.
 */
size_t base_reencode(const char *text, int from_base, int to_base,
                     char *out, size_t out_size) {
    uint64_t value;
    if (!base_parse(text, from_base, &value))
        return 0;
    return base_encode(value, to_base, out, out_size);
}
