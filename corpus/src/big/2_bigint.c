/*
 * bigint.c — Arbitrary-precision unsigned integers stored as decimal digits.
 *
 * A compact big-integer type that keeps numbers as an array of base-10
 * digits in little-endian order (least significant digit first). It supports
 * addition, multiplication by a small scalar, comparison, and full
 * multiplication, which together suffice for things like exact factorials.
 */

#include <stdint.h>
#include <string.h>

/* Maximum number of decimal digits a BigInt can hold. Sized to handle, for
 * example, factorials well past 100!. */
#define BIG_MAX_DIGITS 512

/*
 * An unsigned big integer. `digits[0]` is the ones place; `len` counts the
 * significant digits (always >= 1; the value zero has len 1 and digit 0).
 */
typedef struct {
    int     len;                       /* number of significant digits */
    uint8_t digits[BIG_MAX_DIGITS];    /* little-endian base-10 digits */
} BigInt;

/*
 * Initialize a BigInt from a small unsigned value.
 * Parameters: b — destination; value — initial magnitude.
 * Returns nothing. Always leaves `b` in canonical form (no leading zeros,
 * len >= 1). O(number of digits in value).
 */
void big_from_uint(BigInt *b, uint64_t value) {
    memset(b->digits, 0, sizeof(b->digits));
    b->len = 0;
    /* Peel off decimal digits least-significant first. */
    do {
        b->digits[b->len++] = (uint8_t)(value % 10);
        value /= 10;
    } while (value > 0 && b->len < BIG_MAX_DIGITS);
}

/*
 * Strip leading (most-significant) zero digits so the value is canonical.
 * Parameters: b — number to normalize.
 * Returns nothing. Guarantees len >= 1 even for the value zero.
 */
static void big_trim(BigInt *b) {
    while (b->len > 1 && b->digits[b->len - 1] == 0)
        b->len--;
}

/*
 * Compare two big integers.
 * Parameters: a, b — operands.
 * Returns -1 if a < b, 0 if equal, +1 if a > b. Compares lengths first, then
 * scans from the most significant digit down. O(len).
 */
int big_compare(const BigInt *a, const BigInt *b) {
    if (a->len != b->len)
        return a->len < b->len ? -1 : 1;
    for (int i = a->len - 1; i >= 0; i--) {
        if (a->digits[i] != b->digits[i])
            return a->digits[i] < b->digits[i] ? -1 : 1;
    }
    return 0;
}

/*
 * Add two big integers: out = a + b.
 * Parameters: out — result (may alias a or b); a, b — operands.
 * Returns nothing. Performs schoolbook digit addition with carry; the result
 * is truncated if it would exceed BIG_MAX_DIGITS. O(max(len)).
 */
void big_add(BigInt *out, const BigInt *a, const BigInt *b) {
    BigInt tmp;
    int n = a->len > b->len ? a->len : b->len;
    int carry = 0;
    tmp.len = 0;
    for (int i = 0; i < n || carry; i++) {
        if (i >= BIG_MAX_DIGITS)
            break;                          /* overflow guard */
        int da = i < a->len ? a->digits[i] : 0;
        int db = i < b->len ? b->digits[i] : 0;
        int sum = da + db + carry;
        tmp.digits[tmp.len++] = (uint8_t)(sum % 10);
        carry = sum / 10;                   /* propagate the tens place */
    }
    if (tmp.len == 0)
        tmp.digits[tmp.len++] = 0;          /* keep canonical form for 0 */
    *out = tmp;
    big_trim(out);
}

/*
 * Multiply a big integer by a small scalar: out = a * k.
 * Parameters: out — result (may alias a); a — operand; k — small multiplier.
 * Returns nothing. Each digit is multiplied with running carry that can be
 * larger than 9, so the carry division handles multi-digit overflow. O(len).
 */
void big_mul_small(BigInt *out, const BigInt *a, uint32_t k) {
    BigInt tmp;
    int carry = 0;
    tmp.len = 0;
    for (int i = 0; i < a->len || carry; i++) {
        if (i >= BIG_MAX_DIGITS)
            break;
        int prod = (i < a->len ? a->digits[i] : 0) * (int)k + carry;
        tmp.digits[tmp.len++] = (uint8_t)(prod % 10);
        carry = prod / 10;
    }
    if (tmp.len == 0)
        tmp.digits[tmp.len++] = 0;
    *out = tmp;
    big_trim(out);
}

/*
 * Full multiplication: out = a * b.
 * Parameters: out — result (must NOT alias a or b); a, b — operands.
 * Returns nothing. Uses the classic O(len_a * len_b) convolution with a
 * 32-bit accumulator per column to absorb carries. Overflow past
 * BIG_MAX_DIGITS is silently truncated.
 */
void big_mul(BigInt *out, const BigInt *a, const BigInt *b) {
    /* Accumulate partial products into a wide scratch array, then carry. */
    uint32_t acc[BIG_MAX_DIGITS];
    memset(acc, 0, sizeof(acc));
    for (int i = 0; i < a->len; i++)
        for (int j = 0; j < b->len; j++) {
            int pos = i + j;
            if (pos < BIG_MAX_DIGITS)
                acc[pos] += (uint32_t)a->digits[i] * b->digits[j];
        }
    /* Normalize the accumulator into single decimal digits. */
    int carry = 0;
    out->len = 0;
    for (int i = 0; i < BIG_MAX_DIGITS; i++) {
        uint32_t cur = acc[i] + carry;
        out->digits[out->len++] = (uint8_t)(cur % 10);
        carry = (int)(cur / 10);
        /* Stop once both inputs are exhausted and no carry remains. */
        if (i >= a->len + b->len && carry == 0)
            break;
    }
    if (out->len == 0)
        out->digits[out->len++] = 0;
    big_trim(out);
}

/*
 * Compute n! as a big integer.
 * Parameters: out — destination for the factorial; n — non-negative input.
 * Returns nothing. Builds the product 1*2*...*n via repeated small
 * multiplication. Complexity O(n * digits).
 */
void big_factorial(BigInt *out, uint32_t n) {
    big_from_uint(out, 1);
    for (uint32_t i = 2; i <= n; i++)
        big_mul_small(out, out, i);
}
