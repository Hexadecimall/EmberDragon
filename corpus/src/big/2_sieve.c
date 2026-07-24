/*
 * sieve.c — Prime sieves and basic number-theory helpers.
 *
 * Builds a Sieve of Eratosthenes over a contiguous range and exposes
 * helpers to test primality, count primes, and factor a number using the
 * sieve's smallest-prime-factor table. The smallest-prime-factor variant
 * lets a single O(n log log n) preprocess support O(log v) factorization
 * for any value up to the sieve limit.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * A precomputed sieve covering integers [0, limit].
 * `is_composite[i]` is non-zero when i is composite; `spf[i]` holds the
 * smallest prime factor of i (0 for i < 2). Both arrays are heap-owned.
 */
typedef struct {
    int       limit;          /* largest value the sieve covers */
    uint8_t  *is_composite;   /* length limit+1; 1 == composite */
    int      *spf;            /* length limit+1; smallest prime factor */
} Sieve;

/*
 * Allocate and populate a sieve for [0, limit].
 * Parameters: limit — inclusive upper bound, must be >= 1.
 * Returns a heap-allocated Sieve, or NULL on allocation failure. The caller
 * owns the result and must release it with sieve_free(). Runs in
 * O(limit log log limit) time and O(limit) space.
 */
Sieve *sieve_create(int limit) {
    Sieve *s = malloc(sizeof(Sieve));
    if (!s)
        return NULL;
    s->limit = limit;
    s->is_composite = calloc((size_t)limit + 1, sizeof(uint8_t));
    s->spf = calloc((size_t)limit + 1, sizeof(int));
    if (!s->is_composite || !s->spf) {
        /* Partial allocation must not leak; free whatever succeeded. */
        free(s->is_composite);
        free(s->spf);
        free(s);
        return NULL;
    }
    /* 0 and 1 are neither prime nor composite; mark them so primality
     * queries below never treat them as prime. */
    if (limit >= 0) s->is_composite[0] = 1;
    if (limit >= 1) s->is_composite[1] = 1;
    for (int i = 2; i <= limit; i++) {
        if (!s->is_composite[i]) {
            s->spf[i] = i;                 /* i is prime: its own smallest factor */
            /* Start crossing out at i*i; smaller multiples already carry a
             * smaller prime factor recorded by an earlier pass. */
            for (int64_t j = (int64_t)i * i; j <= limit; j += i) {
                if (!s->is_composite[j]) {
                    s->is_composite[j] = 1;
                    s->spf[j] = i;         /* first prime to hit j is its spf */
                }
            }
        }
    }
    return s;
}

/*
 * Release a sieve created by sieve_create().
 * Parameters: s — sieve to free (NULL is tolerated).
 * Returns nothing.
 */
void sieve_free(Sieve *s) {
    if (!s)
        return;
    free(s->is_composite);
    free(s->spf);
    free(s);
}

/*
 * Report whether `n` is prime according to the sieve.
 * Parameters: s — populated sieve; n — value in [0, s->limit].
 * Returns 1 if prime, 0 otherwise. Returns 0 for out-of-range n rather than
 * reading past the arrays. O(1).
 */
int sieve_is_prime(const Sieve *s, int n) {
    if (n < 2 || n > s->limit)
        return 0;
    return !s->is_composite[n];
}

/*
 * Count how many primes lie in [0, n].
 * Parameters: s — sieve; n — inclusive upper bound, clamped to the limit.
 * Returns the count of primes. O(n) over the requested prefix.
 */
int sieve_count_primes(const Sieve *s, int n) {
    if (n > s->limit)
        n = s->limit;
    int count = 0;
    for (int i = 2; i <= n; i++)
        if (!s->is_composite[i])
            count++;
    return count;
}

/*
 * Factor `n` into its prime factors using the smallest-prime-factor table.
 * Parameters: s — sieve covering n; n — value in [2, s->limit];
 *             out_factors — caller buffer receiving the (possibly repeated)
 *             prime factors in ascending order; cap — capacity of out_factors.
 * Returns the number of factors written, or -1 if n is out of range. Writes
 * at most `cap` factors. Complexity O(log n) because each step strips one
 * prime via the spf table.
 */
int sieve_factorize(const Sieve *s, int n, int *out_factors, int cap) {
    if (n < 2 || n > s->limit)
        return -1;
    int count = 0;
    while (n > 1 && count < cap) {
        int p = s->spf[n];        /* O(1) lookup of the next prime factor */
        out_factors[count++] = p;
        n /= p;                   /* peel it off and continue */
    }
    return count;
}
