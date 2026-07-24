/*
 * gcd.c — Greatest common divisor, least common multiple, and the
 * extended Euclidean algorithm with modular-inverse support.
 *
 * All routines operate on 64-bit signed integers and avoid overflow where
 * practical. The extended variant returns Bezout coefficients, which the
 * modular-inverse helper uses to solve a*x ≡ 1 (mod m).
 */

#include <stdint.h>

/*
 * Compute the greatest common divisor of a and b.
 * Parameters: a, b — any integers (sign is ignored).
 * Returns the non-negative gcd; gcd(0, 0) is defined as 0. Uses the
 * iterative Euclidean algorithm, O(log min(|a|,|b|)) divisions.
 */
int64_t gcd(int64_t a, int64_t b) {
    /* Work with magnitudes so the result is always non-negative. */
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int64_t t = a % b;        /* remainder shrinks each step */
        a = b;
        b = t;
    }
    return a;
}

/*
 * Compute the least common multiple of a and b.
 * Parameters: a, b — integers.
 * Returns the non-negative lcm, or 0 if either input is 0. Divides before
 * multiplying (a/g * b) to reduce the chance of intermediate overflow.
 */
int64_t lcm(int64_t a, int64_t b) {
    if (a == 0 || b == 0)
        return 0;
    int64_t g = gcd(a, b);
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    /* a/g is exact since g divides a; this keeps the product smaller. */
    return (a / g) * b;
}

/*
 * Extended Euclidean algorithm: find g = gcd(a, b) along with x, y such
 * that a*x + b*y = g.
 * Parameters: a, b — inputs; out_x, out_y — receive the Bezout coefficients
 *             (either may be NULL if the caller does not need it).
 * Returns the gcd. Implemented iteratively to avoid deep recursion.
 */
int64_t extended_gcd(int64_t a, int64_t b, int64_t *out_x, int64_t *out_y) {
    /* Maintain coefficient pairs for the current remainders. The loop
     * invariant is: old_r = a*old_s + b*old_t at all times. */
    int64_t old_r = a, r = b;
    int64_t old_s = 1, s = 0;
    int64_t old_t = 0, t = 1;
    while (r != 0) {
        int64_t q = old_r / r;
        int64_t tmp;
        tmp = old_r - q * r; old_r = r; r = tmp;
        tmp = old_s - q * s; old_s = s; s = tmp;
        tmp = old_t - q * t; old_t = t; t = tmp;
    }
    if (out_x) *out_x = old_s;
    if (out_y) *out_y = old_t;
    return old_r;
}

/*
 * Compute the modular multiplicative inverse of a modulo m.
 * Parameters: a — value to invert; m — modulus > 1.
 * Returns x in [0, m) with a*x ≡ 1 (mod m), or -1 when the inverse does not
 * exist (i.e. gcd(a, m) != 1). Builds on extended_gcd().
 */
int64_t mod_inverse(int64_t a, int64_t m) {
    int64_t x, y;
    int64_t g = extended_gcd(a, m, &x, &y);
    if (g != 1)
        return -1;                /* a and m share a factor: no inverse */
    /* x may be negative; normalize it into the canonical range [0, m). */
    int64_t inv = x % m;
    if (inv < 0)
        inv += m;
    return inv;
}

/*
 * Test whether two integers are coprime (share no common factor > 1).
 * Parameters: a, b — integers.
 * Returns 1 if gcd(a, b) == 1, else 0.
 */
int are_coprime(int64_t a, int64_t b) {
    return gcd(a, b) == 1;
}
