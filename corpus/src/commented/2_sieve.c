/*
 * sieve.c — Prime generation via the Sieve of Eratosthenes.
 *
 * Builds a boolean primality table over [0, limit] and exposes helpers
 * to test primality, count primes, list them into a caller buffer, and
 * find the next prime above a value. The sieve buffer is heap-allocated
 * and owned by a PrimeSieve handle that the caller must free.
 */

#include <stdint.h>
#include <stdlib.h>

/* Holds the computed primality table for the inclusive range [0, limit]. */
typedef struct {
    uint8_t *is_prime;  /* is_prime[i] == 1 iff i is prime; length limit+1 */
    int32_t  limit;     /* largest number covered by the table             */
} PrimeSieve;

/*
 * Build a sieve covering [0, limit].
 *   limit: the largest integer to classify (must be >= 0).
 * Returns a heap-allocated PrimeSieve, or NULL on allocation failure.
 * The caller owns the result and must release it with sieve_free.
 * Runs in O(n log log n) time and O(n) space.
 */
PrimeSieve *sieve_create(int32_t limit) {
    if (limit < 0) return NULL;

    PrimeSieve *sieve = (PrimeSieve *)malloc(sizeof(PrimeSieve));
    if (sieve == NULL) return NULL;

    sieve->limit = limit;
    sieve->is_prime = (uint8_t *)malloc((size_t)limit + 1);
    if (sieve->is_prime == NULL) {
        free(sieve);             /* avoid leaking the handle on failure */
        return NULL;
    }

    /* Optimistically mark everything prime, then strike out non-primes. */
    for (int32_t i = 0; i <= limit; i++) {
        sieve->is_prime[i] = 1;
    }
    if (limit >= 0) sieve->is_prime[0] = 0;  /* 0 is not prime */
    if (limit >= 1) sieve->is_prime[1] = 0;  /* 1 is not prime */

    /* For each prime p, strike out its multiples starting at p*p, since
     * all smaller multiples were already struck by smaller primes. */
    for (int32_t p = 2; (int64_t)p * p <= limit; p++) {
        if (sieve->is_prime[p]) {
            for (int64_t multiple = (int64_t)p * p; multiple <= limit; multiple += p) {
                sieve->is_prime[multiple] = 0;
            }
        }
    }
    return sieve;
}

/*
 * Release a sieve created by sieve_create.
 *   sieve: handle to free; passing NULL is safe and does nothing.
 */
void sieve_free(PrimeSieve *sieve) {
    if (sieve == NULL) return;
    free(sieve->is_prime);
    free(sieve);
}

/*
 * Test whether a number is prime according to the table.
 *   sieve: a built sieve.
 *   n:     the number to test.
 * Returns 1 if prime, 0 if composite, and 0 for out-of-range values.
 * O(1).
 */
int sieve_is_prime(const PrimeSieve *sieve, int32_t n) {
    if (n < 0 || n > sieve->limit) return 0;  /* outside coverage */
    return sieve->is_prime[n] ? 1 : 0;
}

/*
 * Count the primes in [0, limit].
 *   sieve: a built sieve.
 * Returns the total number of primes. O(limit).
 */
int32_t sieve_count(const PrimeSieve *sieve) {
    int32_t total = 0;
    for (int32_t i = 2; i <= sieve->limit; i++) {
        if (sieve->is_prime[i]) total++;
    }
    return total;
}

/*
 * Copy the primes, in ascending order, into a caller-supplied buffer.
 *   sieve:    a built sieve.
 *   out:      destination array.
 *   capacity: maximum number of primes to write.
 * Returns the number of primes actually written (<= capacity). Stops
 * early once the buffer is full. O(limit).
 */
int32_t sieve_collect(const PrimeSieve *sieve, int32_t *out, int32_t capacity) {
    int32_t written = 0;
    for (int32_t i = 2; i <= sieve->limit && written < capacity; i++) {
        if (sieve->is_prime[i]) {
            out[written++] = i;
        }
    }
    return written;
}

/*
 * Find the smallest prime strictly greater than n.
 *   sieve: a built sieve.
 *   n:     the exclusive lower bound.
 * Returns the next prime, or -1 if none exists within the sieve's range.
 * O(distance to the next prime).
 */
int32_t sieve_next_prime(const PrimeSieve *sieve, int32_t n) {
    for (int32_t candidate = n + 1; candidate <= sieve->limit; candidate++) {
        if (sieve->is_prime[candidate]) return candidate;
    }
    return -1;  /* ran off the end of the table */
}
