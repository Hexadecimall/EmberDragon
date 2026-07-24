/*
 * modexp.c — Modular arithmetic primitives for number theory and
 * cryptographic-style computations.
 *
 * Implements overflow-safe modular addition and multiplication,
 * fast modular exponentiation by squaring, a Fermat-based primality
 * heuristic, and modular inverse via Fermat's little theorem. All
 * arithmetic is unsigned 64-bit with explicit overflow handling.
 */

#include <stdint.h>

/*
 * Reduce a value into the canonical range [0, modulus).
 *   value:   the number to reduce.
 *   modulus: the modulus (must be > 0).
 * Returns value mod modulus. Returns 0 if modulus is 0 to stay defined.
 */
uint64_t mod_normalize(uint64_t value, uint64_t modulus) {
    if (modulus == 0) return 0;  /* undefined mathematically; pick 0 */
    return value % modulus;
}

/*
 * Modular addition that cannot overflow even when both operands are near
 * 2^64.
 *   a, b:    operands, each assumed already < modulus.
 *   modulus: the modulus (> 0).
 * Returns (a + b) mod modulus.
 */
uint64_t mod_add(uint64_t a, uint64_t b, uint64_t modulus) {
    a %= modulus;
    b %= modulus;
    /* If a + b would wrap, subtract the modulus from b first so the sum
     * stays representable. */
    if (a >= modulus - b) {
        return a - (modulus - b);
    }
    return a + b;
}

/*
 * Modular multiplication using the Russian-peasant (double-and-add)
 * method, which avoids needing a 128-bit intermediate product.
 *   a, b:    operands.
 *   modulus: the modulus (> 0).
 * Returns (a * b) mod modulus. O(log b) modular additions.
 */
uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t modulus) {
    uint64_t result = 0;
    a %= modulus;
    while (b > 0) {
        /* If the low bit of b is set, fold the current `a` into result. */
        if (b & 1) {
            result = mod_add(result, a, modulus);
        }
        a = mod_add(a, a, modulus);  /* double a, staying within the modulus */
        b >>= 1;
    }
    return result;
}

/*
 * Fast modular exponentiation: base^exponent mod modulus, computed by
 * repeated squaring.
 *   base:     the base.
 *   exponent: the exponent.
 *   modulus:  the modulus (> 1 for meaningful results).
 * Returns the modular power. Defined as 1 for exponent 0. O(log exponent)
 * modular multiplications.
 */
uint64_t mod_pow(uint64_t base, uint64_t exponent, uint64_t modulus) {
    if (modulus == 1) return 0;  /* everything is congruent to 0 mod 1 */
    uint64_t result = 1;
    base %= modulus;
    while (exponent > 0) {
        /* Multiply result by the current base power when this bit is set. */
        if (exponent & 1) {
            result = mod_mul(result, base, modulus);
        }
        base = mod_mul(base, base, modulus);  /* base now holds base^(2^k) */
        exponent >>= 1;
    }
    return result;
}

/*
 * Fermat primality test for a single witness. By Fermat's little theorem,
 * if p is prime then witness^(p-1) == 1 (mod p) for any witness coprime
 * to p.
 *   candidate: the number to test (should be > 2).
 *   witness:   the base to test with (1 < witness < candidate).
 * Returns 1 if `candidate` passes (probably prime for this witness), 0 if
 * it definitely fails (composite). Note: composite Carmichael numbers can
 * pass for many witnesses, so this is a heuristic, not a proof.
 */
int fermat_probably_prime(uint64_t candidate, uint64_t witness) {
    if (candidate < 2) return 0;
    if (candidate == 2) return 1;
    if ((candidate & 1) == 0) return 0;  /* even numbers above 2 are composite */
    return mod_pow(witness, candidate - 1, candidate) == 1 ? 1 : 0;
}

/*
 * Modular multiplicative inverse via Fermat's little theorem, valid only
 * when `prime` is actually prime. Computes value^(prime-2) mod prime.
 *   value: the element to invert (must not be a multiple of prime).
 *   prime: a prime modulus.
 * Returns the inverse in [1, prime), or 0 if value is 0 mod prime (no
 * inverse exists).
 */
uint64_t mod_inverse(uint64_t value, uint64_t prime) {
    if (prime <= 1) return 0;
    if (value % prime == 0) return 0;  /* zero has no inverse */
    return mod_pow(value, prime - 2, prime);
}
