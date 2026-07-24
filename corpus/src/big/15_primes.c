/*
 * primes.c — Prime number generation and primality testing.
 *
 * A small primes toolkit: trial-division primality, a Sieve of
 * Eratosthenes that produces a packed list of primes up to a limit, prime
 * counting, and next-prime lookup. The sieve allocates a byte mark array
 * and the caller owns the returned prime list.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * Test whether n is prime by trial division.
 *
 * Checks divisibility by 2, 3, then all numbers of the form 6k +/- 1 up to
 * sqrt(n). The square-root bound is enforced via the product test
 * `divisor*divisor <= n`, avoiding any floating-point math.
 *
 * n:      candidate value.
 * return: 1 if n is prime, 0 otherwise. O(sqrt(n)) time.
 */
int is_prime(uint64_t n) {
    if (n < 2) {
        return 0;            /* 0 and 1 are not prime by definition */
    }
    if (n < 4) {
        return 1;            /* 2 and 3 are prime */
    }
    if ((n % 2) == 0 || (n % 3) == 0) {
        return 0;            /* quick reject of the two smallest factors */
    }
    /* Every prime > 3 is congruent to 1 or 5 modulo 6, so step by 6 and
     * test the two neighbours of each multiple of 6. */
    for (uint64_t divisor = 5; divisor * divisor <= n; divisor += 6) {
        if ((n % divisor) == 0 || (n % (divisor + 2)) == 0) {
            return 0;
        }
    }
    return 1;
}

/*
 * Sieve of Eratosthenes: collect all primes <= limit.
 *
 * Allocates a temporary mark array, strikes out composites starting each
 * prime's multiples at p*p, then compacts the survivors into a freshly
 * allocated result array.
 *
 * limit:     inclusive upper bound to sieve.
 * out_count: receives the number of primes found.
 * return:    a malloc'd array of primes (caller must free), or NULL on
 *            allocation failure. O(n log log n) time, O(n) space.
 */
uint64_t *sieve_primes(uint64_t limit, uint64_t *out_count) {
    *out_count = 0;
    if (limit < 2) {
        return NULL;         /* no primes exist below 2 */
    }
    /* is_composite[i] == 1 once i is known to be composite. */
    uint8_t *is_composite = (uint8_t *)calloc((size_t)limit + 1, 1);
    if (is_composite == NULL) {
        return NULL;
    }
    for (uint64_t p = 2; p * p <= limit; p++) {
        if (is_composite[p]) {
            continue;        /* already struck out, so its multiples are too */
        }
        /* Start at p*p; smaller multiples carry a smaller prime factor and
         * were eliminated in earlier passes. */
        for (uint64_t multiple = p * p; multiple <= limit; multiple += p) {
            is_composite[multiple] = 1;
        }
    }
    /* First pass: count survivors so we can size the result exactly. */
    uint64_t count = 0;
    for (uint64_t i = 2; i <= limit; i++) {
        if (!is_composite[i]) {
            count++;
        }
    }
    uint64_t *primes = (uint64_t *)malloc(count * sizeof(uint64_t));
    if (primes == NULL) {
        free(is_composite);
        return NULL;
    }
    /* Second pass: compact the unmarked indices into the output. */
    uint64_t index = 0;
    for (uint64_t i = 2; i <= limit; i++) {
        if (!is_composite[i]) {
            primes[index++] = i;
        }
    }
    free(is_composite);
    *out_count = count;
    return primes;
}

/*
 * Count primes <= limit without retaining the list.
 *
 * Convenience wrapper that runs the sieve and frees the array immediately,
 * trading memory for a single scalar answer.
 *
 * limit:  inclusive upper bound.
 * return: the count of primes in [2, limit]. O(n log log n) time.
 */
uint64_t prime_count(uint64_t limit) {
    uint64_t count = 0;
    uint64_t *primes = sieve_primes(limit, &count);
    free(primes);            /* free(NULL) is safe when limit < 2 */
    return count;
}

/*
 * Return the smallest prime strictly greater than n.
 *
 * Scans upward using the trial-division test. There is always a next prime
 * (primes are infinite), so this terminates for any valid input.
 *
 * n:      lower bound (exclusive).
 * return: the next prime after n. O((gap) * sqrt(n)) time, where gap is the
 *         distance to that prime.
 */
uint64_t next_prime(uint64_t n) {
    uint64_t candidate = n + 1;
    while (!is_prime(candidate)) {
        candidate++;
    }
    return candidate;
}
