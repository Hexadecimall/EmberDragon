/*
 * int_format.c -- Format integers with thousands separators and sign control.
 *
 * Converts a signed 64-bit integer into a human-readable string with grouped
 * digits (e.g. -1234567 -> "-1,234,567") using a configurable group size and
 * separator character. All work is done with integer arithmetic and a small
 * stack buffer; no division-by-floating-point and no printf are involved.
 */

#include <stddef.h>
#include <stdint.h>

/* Knobs that control how a number is grouped and signed. */
typedef struct {
    char separator;     /* character inserted between digit groups, e.g. ',' */
    int  groupSize;     /* digits per group (3 for Western style); >=1        */
    int  forceSign;     /* if nonzero, prefix non-negative values with '+'    */
} GroupStyle;

/*
 * reverseInPlace -- Reverse the first `len` bytes of a buffer.
 *
 * @buf: buffer to reverse.
 * @len: number of bytes to reverse.
 *
 * Used because digits are produced least-significant first and must be flipped
 * into print order. O(len) with a two-pointer swap.
 */
static void reverseInPlace(char *buf, size_t len) {
    size_t i = 0, j = (len == 0 ? 0 : len - 1);
    while (i < j) {
        char tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
        i++;
        j--;
    }
}

/*
 * absToDigits -- Emit the decimal digits of |value|, least significant first.
 *
 * @value:  the (possibly negative) input value.
 * @digits: out buffer receiving ASCII digits in reverse order.
 * Returns the number of digits written (always >= 1, since 0 yields "0").
 *
 * INT64_MIN is handled safely by negating in the unsigned domain rather than
 * computing -value (which would overflow). O(number of digits).
 */
static size_t absToDigits(int64_t value, char *digits) {
    /* Cast to unsigned so the magnitude of INT64_MIN is representable. */
    uint64_t magnitude;
    if (value < 0) {
        magnitude = (uint64_t)(-(value + 1)) + 1ULL;
    } else {
        magnitude = (uint64_t)value;
    }

    size_t n = 0;
    do {
        digits[n++] = (char)('0' + (int)(magnitude % 10));
        magnitude /= 10;
    } while (magnitude > 0);
    return n;
}

/*
 * formatGrouped -- Format `value` with grouped digits into `out`.
 *
 * @out:   destination buffer for the NUL-terminated result.
 * @cap:   capacity of `out` in bytes.
 * @value: the signed integer to format.
 * @style: grouping and sign options; groupSize is clamped to at least 1.
 *
 * Returns the number of characters written (excluding the terminator), or 0 if
 * the buffer is too small to hold the full result. Separators are inserted
 * every `groupSize` digits counting from the least significant end. The sign
 * is '-' for negatives, optionally '+' for non-negatives. O(number of digits).
 */
size_t formatGrouped(char *out, size_t cap, int64_t value,
                     const GroupStyle *style) {
    if (cap == 0) {
        return 0;
    }

    int groupSize = style->groupSize < 1 ? 1 : style->groupSize;

    /* 20 digits is enough for any 64-bit value. */
    char digits[24];
    size_t digitCount = absToDigits(value, digits);

    /* Build the grouped digit string (still reversed) into a scratch buffer.
     * Worst case: every digit plus a separator before it -> 2x digits. */
    char grouped[48];
    size_t g = 0;
    for (size_t i = 0; i < digitCount; i++) {
        /* Insert a separator before starting each new full group (but never
         * before the very first digit). */
        if (i > 0 && (i % (size_t)groupSize) == 0) {
            grouped[g++] = style->separator;
        }
        grouped[g++] = digits[i];
    }

    /* Flip from least-significant-first into human reading order. */
    reverseInPlace(grouped, g);

    /* Determine the sign prefix. */
    char sign = 0;
    if (value < 0) {
        sign = '-';
    } else if (style->forceSign) {
        sign = '+';
    }

    /* Compute total length and bail out if it cannot fit (with the NUL). */
    size_t total = g + (sign ? 1 : 0);
    if (total + 1 > cap) {
        out[0] = '\0';
        return 0;
    }

    size_t idx = 0;
    if (sign) {
        out[idx++] = sign;
    }
    for (size_t i = 0; i < g; i++) {
        out[idx++] = grouped[i];
    }
    out[idx] = '\0';
    return idx;
}
