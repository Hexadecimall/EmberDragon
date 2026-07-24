/*
 * fixedpoint.c — Q16.16 fixed-point arithmetic in pure integer math.
 *
 * Represents fractional numbers as 32-bit signed integers scaled by 2^16,
 * giving 16 bits of fraction. This avoids floating point entirely, which is
 * useful on hardware without an FPU or where deterministic, bit-exact
 * results matter (audio, embedded control, game physics).
 */

#include <stdint.h>

/* Number of fractional bits and the corresponding scaling factor. */
#define FP_SHIFT  16
#define FP_ONE    (1 << FP_SHIFT)          /* the fixed-point value 1.0 */
#define FP_HALF   (1 << (FP_SHIFT - 1))    /* the fixed-point value 0.5 */

/* A Q16.16 fixed-point number. The integer part occupies the high 16 bits,
 * the fraction the low 16 bits. */
typedef int32_t fixed_t;

/*
 * Convert a whole integer to fixed-point.
 * Parameters: value — an integer in roughly [-32768, 32767] to stay in range.
 * Returns the Q16.16 representation. O(1).
 */
fixed_t fixed_from_int(int32_t value) {
    return value << FP_SHIFT;
}

/*
 * Convert a fixed-point number back to an integer, truncating toward zero.
 * Parameters: x — fixed-point value.
 * Returns the integer part. The arithmetic shift discards the fraction.
 */
int32_t fixed_to_int(fixed_t x) {
    return x >> FP_SHIFT;
}

/*
 * Add two fixed-point numbers.
 * Parameters: a, b — operands.
 * Returns a + b. Since both share the same scale, plain integer addition is
 * correct; the caller is responsible for avoiding 32-bit overflow.
 */
fixed_t fixed_add(fixed_t a, fixed_t b) {
    return a + b;
}

/*
 * Subtract two fixed-point numbers.
 * Parameters: a, b — operands.
 * Returns a - b at the shared scale.
 */
fixed_t fixed_sub(fixed_t a, fixed_t b) {
    return a - b;
}

/*
 * Multiply two fixed-point numbers.
 * Parameters: a, b — operands.
 * Returns a * b. The product of two Q16.16 values is Q32.32, so we widen to
 * 64 bits, then shift right by FP_SHIFT to renormalize back to Q16.16.
 * Widening is what prevents the intermediate result from overflowing.
 */
fixed_t fixed_mul(fixed_t a, fixed_t b) {
    int64_t wide = (int64_t)a * (int64_t)b;
    return (fixed_t)(wide >> FP_SHIFT);
}

/*
 * Divide two fixed-point numbers.
 * Parameters: a — numerator; b — denominator (must be non-zero).
 * Returns a / b in Q16.16, rounded toward zero. We pre-scale the numerator
 * by 2^16 (in 64-bit space) so the quotient lands back at the right scale.
 * Returns 0 if b is 0, to fail safe instead of trapping.
 */
fixed_t fixed_div(fixed_t a, fixed_t b) {
    if (b == 0)
        return 0;
    int64_t wide = ((int64_t)a << FP_SHIFT);
    return (fixed_t)(wide / b);
}

/*
 * Round a fixed-point number to the nearest integer (ties away from zero
 * for positives, toward zero behavior handled by sign).
 * Parameters: x — fixed-point value.
 * Returns the nearest integer as a plain int32_t. Adds/subtracts 0.5 before
 * truncating so the result rounds rather than floors.
 */
int32_t fixed_round(fixed_t x) {
    if (x >= 0)
        return (x + FP_HALF) >> FP_SHIFT;
    /* For negatives, nudge the other direction so rounding is symmetric. */
    return -(((-x) + FP_HALF) >> FP_SHIFT);
}

/*
 * Compute an integer square root of a fixed-point number, returning a
 * fixed-point result.
 * Parameters: x — non-negative fixed-point value.
 * Returns sqrt(x) in Q16.16, or 0 for negative input. Uses a bit-by-bit
 * digit-recurrence algorithm on the 64-bit pre-scaled operand — no floating
 * point and no <math.h>. Complexity O(1) (fixed 32 iterations).
 */
fixed_t fixed_sqrt(fixed_t x) {
    if (x <= 0)
        return 0;
    /* Pre-scale by 2^16 so that sqrt of the scaled value comes out already
     * in Q16.16: sqrt(x * 2^16) = sqrt(x) * 2^8, but we shift x left by
     * FP_SHIFT first so the final root is correctly scaled. */
    uint64_t n = ((uint64_t)x) << FP_SHIFT;
    uint64_t result = 0;
    /* `bit` walks down from the highest power-of-four <= n. */
    uint64_t bit = (uint64_t)1 << 62;
    while (bit > n)
        bit >>= 2;
    while (bit != 0) {
        if (n >= result + bit) {
            n -= result + bit;          /* this bit belongs in the root */
            result = (result >> 1) + bit;
        } else {
            result >>= 1;               /* this bit does not fit */
        }
        bit >>= 2;
    }
    return (fixed_t)result;
}
