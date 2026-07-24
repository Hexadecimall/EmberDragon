/*
 * gcd.c — Greatest common divisor, least common multiple, and the
 * extended Euclidean algorithm over signed 64-bit integers.
 *
 * Includes both the binary (Stein's) GCD and the classic Euclidean GCD,
 * plus LCM, coprimality testing, and the extended algorithm that also
 * recovers the Bezout coefficients used for modular inverses.
 */

#include <stdint.h>

/*
 * Absolute value of a 64-bit integer.
 *   v: the input value.
 * Returns |v|. Kept local so this module needs no standard library.
 */
static int64_t abs64(int64_t v) {
    return (v < 0) ? -v : v;
}

/*
 * Greatest common divisor via the classic Euclidean algorithm.
 *   a, b: the two integers (signs are ignored).
 * Returns the non-negative gcd; gcd(0, 0) is defined here as 0.
 * O(log(min(a, b))) iterations.
 */
int64_t gcd_euclid(int64_t a, int64_t b) {
    a = abs64(a);
    b = abs64(b);
    /* Repeatedly replace (a, b) with (b, a mod b) until b reaches 0. */
    while (b != 0) {
        int64_t remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

/*
 * Greatest common divisor via Stein's binary algorithm, which uses only
 * subtraction and bit shifts (no division).
 *   a, b: the two integers (signs are ignored).
 * Returns the non-negative gcd. Equivalent result to gcd_euclid but
 * avoids the modulo operation.
 */
int64_t gcd_binary(int64_t a, int64_t b) {
    a = abs64(a);
    b = abs64(b);
    if (a == 0) return b;   /* gcd(0, b) = b */
    if (b == 0) return a;   /* gcd(a, 0) = a */

    /* Factor out the common powers of two into `shift`. */
    int shift = 0;
    while (((a | b) & 1) == 0) {
        a >>= 1;
        b >>= 1;
        shift++;
    }

    /* Remove remaining factors of two from a; a is now odd. */
    while ((a & 1) == 0) a >>= 1;

    do {
        /* Make b odd, then ensure a <= b before subtracting. */
        while ((b & 1) == 0) b >>= 1;
        if (a > b) {
            int64_t t = a; a = b; b = t;  /* swap so the difference stays positive */
        }
        b = b - a;
    } while (b != 0);

    /* Restore the common factors of two stripped out earlier. */
    return a << shift;
}

/*
 * Least common multiple of two integers.
 *   a, b: the two integers.
 * Returns the non-negative lcm, or 0 if either input is 0 (the lcm with
 * zero is conventionally 0). Divides before multiplying to reduce the
 * chance of 64-bit overflow.
 */
int64_t lcm(int64_t a, int64_t b) {
    if (a == 0 || b == 0) return 0;
    int64_t g = gcd_euclid(a, b);
    /* (a / g) * b is exact because g divides a, and keeps the product
     * smaller than the naive a*b form. */
    return abs64(a / g * b);
}

/*
 * Test whether two integers are coprime (share no factor but 1).
 *   a, b: the two integers.
 * Returns 1 if gcd(a, b) == 1, else 0.
 */
int are_coprime(int64_t a, int64_t b) {
    return gcd_euclid(a, b) == 1 ? 1 : 0;
}

/*
 * Extended Euclidean algorithm. Finds integers x, y satisfying
 * a*x + b*y = gcd(a, b) (Bezout's identity).
 *   a, b:   the two integers.
 * 	 out_x:  receives the coefficient of a (may be NULL to discard).
 *   out_y:  receives the coefficient of b (may be NULL to discard).
 * Returns gcd(a, b). The coefficients are the primary use case for
 * computing modular inverses.
 */
int64_t gcd_extended(int64_t a, int64_t b, int64_t *out_x, int64_t *out_y) {
    /* Iteratively maintain (old_r, r) alongside their coefficients. */
    int64_t old_r = a, r = b;
    int64_t old_s = 1, s = 0;   /* coefficients of a */
    int64_t old_t = 0, t = 1;   /* coefficients of b */

    while (r != 0) {
        int64_t quotient = old_r / r;

        int64_t tmp = old_r - quotient * r;
        old_r = r; r = tmp;

        tmp = old_s - quotient * s;
        old_s = s; s = tmp;

        tmp = old_t - quotient * t;
        old_t = t; t = tmp;
    }

    if (out_x) *out_x = old_s;
    if (out_y) *out_y = old_t;
    return old_r;  /* old_r is now gcd(a, b) */
}
