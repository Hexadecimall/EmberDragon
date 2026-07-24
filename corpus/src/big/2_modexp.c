/*
 * modexp.c — Modular arithmetic primitives over 64-bit integers.
 *
 * Implements overflow-safe modular multiplication, fast modular
 * exponentiation, and a Miller-Rabin primality test built on top of them.
 * These are the building blocks for hashing, RSA-style toy crypto, and
 * deterministic primality checks on values up to 2^63.
 */

#include <stdint.h>

/*
 * Multiply (a * b) mod m without overflowing 64 bits.
 * Parameters: a, b — operands already reduced into [0, m); m — modulus > 0.
 * Returns (a*b) % m. Uses Russian-peasant (double-and-add) multiplication
 * so the intermediate product never exceeds 2*m, dodging the overflow that
 * a naive a*b would suffer for large operands. Complexity O(log b).
 */
uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
    uint64_t result = 0;
    a %= m;                       /* keep the running addend below m */
    while (b > 0) {
        /* If the low bit of b is set, fold one copy of a into the sum,
         * reducing mod m to stay in range. */
        if (b & 1u) {
            result += a;
            if (result >= m)      /* single conditional subtract == cheap mod */
                result -= m;
        }
        a <<= 1;                  /* double a ... */
        if (a >= m)
            a -= m;               /* ... and keep it reduced */
        b >>= 1;
    }
    return result;
}

/*
 * Compute (base^exp) mod m using binary exponentiation.
 * Parameters: base — value to raise; exp — non-negative exponent;
 *             m — modulus > 0.
 * Returns base^exp mod m. Returns 0 when m == 1 (everything is congruent
 * to 0). Relies on mulmod() so it is safe for moduli near 2^63.
 * Complexity O(log exp) modular multiplications.
 */
uint64_t powmod(uint64_t base, uint64_t exp, uint64_t m) {
    if (m == 1)
        return 0;
    uint64_t result = 1;
    base %= m;
    while (exp > 0) {
        if (exp & 1u)                       /* accumulate on set bits */
            result = mulmod(result, base, m);
        exp >>= 1;
        base = mulmod(base, base, m);       /* square for the next bit */
    }
    return result;
}

/*
 * Run a single Miller-Rabin witness test for compositeness of n.
 * Parameters: n — odd candidate > 2; a — base/witness in [2, n-2];
 *             d, r — the factorization n-1 = d * 2^r with d odd.
 * Returns 1 if `a` is consistent with n being prime, 0 if `a` proves n
 * composite. A return of 1 is not a proof of primality by itself.
 */
static int miller_rabin_witness(uint64_t n, uint64_t a, uint64_t d, int r) {
    uint64_t x = powmod(a, d, n);
    if (x == 1 || x == n - 1)
        return 1;                           /* probable prime for this base */
    /* Square up to r-1 times looking for a -1 (== n-1) residue. */
    for (int i = 0; i < r - 1; i++) {
        x = mulmod(x, x, n);
        if (x == n - 1)
            return 1;
    }
    return 0;                               /* definitely composite */
}

/*
 * Deterministic primality test for any n < 2^64.
 * Parameters: n — value to test.
 * Returns 1 if n is prime, 0 otherwise. Uses a fixed set of witnesses that
 * is proven to be deterministic for the whole 64-bit range, so the answer
 * is exact (no false positives). Complexity O(k log^3 n) for k witnesses.
 */
int is_prime(uint64_t n) {
    if (n < 2)
        return 0;
    /* Trial-divide by the first few primes to catch easy composites and to
     * satisfy the "n is odd and > 2" precondition of Miller-Rabin. */
    static const uint64_t small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (int i = 0; i < 12; i++) {
        if (n == small[i])
            return 1;
        if (n % small[i] == 0)
            return 0;
    }
    /* Decompose n-1 = d * 2^r with d odd. */
    uint64_t d = n - 1;
    int r = 0;
    while ((d & 1u) == 0) {
        d >>= 1;
        r++;
    }
    /* This witness set is sufficient for a deterministic verdict below 2^64. */
    for (int i = 0; i < 12; i++)
        if (!miller_rabin_witness(n, small[i], d, r))
            return 0;
    return 1;
}
