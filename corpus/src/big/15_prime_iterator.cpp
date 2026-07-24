/*
 * prime_iterator.cpp — A lazy prime generator object.
 *
 * Wraps incremental primality testing in an iterator-style class so callers
 * can pull primes one at a time without committing to a fixed upper bound
 * up front. Caches discovered primes and tests new candidates only against
 * the primes found so far, which is faster than naive trial division.
 */

#include <cstdint>
#include <cstdlib>

/*
 * PrimeIterator yields the primes 2, 3, 5, 7, ... in order.
 *
 * The growable `cache_` array stores every prime emitted so far; `next_` is
 * the candidate the generator will examine next. The cache lets primality
 * checks divide only by known primes up to the square root.
 */
class PrimeIterator {
public:
    /*
     * Construct an iterator positioned before the first prime.
     *
     * Allocates a small initial cache. The first call to next() returns 2.
     */
    PrimeIterator() {
        capacity_ = 16;
        count_ = 0;
        cache_ = static_cast<uint64_t *>(malloc(capacity_ * sizeof(uint64_t)));
        next_ = 2; /* the first candidate to consider */
    }

    /*
     * Release the cache. The iterator owns its storage exclusively.
     */
    ~PrimeIterator() {
        free(cache_);
    }

    /*
     * Produce the next prime in sequence.
     *
     * Scans candidates upward, testing each against cached primes, and
     * appends the winner to the cache before returning it.
     *
     * return: the next prime. Amortized cost grows with the prime gap and
     *         the number of cached primes below sqrt(candidate).
     */
    uint64_t next() {
        while (!is_prime_by_cache(next_)) {
            next_++; /* skip composites until a prime appears */
        }
        uint64_t prime = next_;
        append(prime);
        next_++;     /* never re-test the prime we just emitted */
        return prime;
    }

    /*
     * Advance until a prime strictly greater than `floor` is produced.
     *
     * Useful for jumping ahead. Repeatedly pulls from next() until the value
     * clears the floor; all skipped primes are still cached.
     *
     * floor:  exclusive lower bound.
     * return: the first emitted prime greater than floor.
     */
    uint64_t advance_past(uint64_t floor) {
        uint64_t p = next();
        while (p <= floor) {
            p = next();
        }
        return p;
    }

    /*
     * Number of primes emitted (and cached) so far.
     *
     * return: the running count of generated primes.
     */
    uint64_t count() const {
        return count_;
    }

private:
    uint64_t *cache_;   /* primes discovered so far, in ascending order */
    uint64_t count_;    /* how many entries of cache_ are valid */
    uint64_t capacity_; /* allocated slots in cache_ */
    uint64_t next_;     /* next candidate to test */

    /*
     * Test n for primality using only the cached primes.
     *
     * Relies on the invariant that the cache contains every prime below n;
     * since candidates are visited in order, all primes up to sqrt(n) are
     * already present by the time n is examined.
     *
     * n:      candidate, assumed >= 2.
     * return: true if n is prime, false otherwise.
     */
    bool is_prime_by_cache(uint64_t n) {
        for (uint64_t i = 0; i < count_; i++) {
            uint64_t p = cache_[i];
            if (p * p > n) {
                break;          /* no prime factor can exceed sqrt(n) */
            }
            if ((n % p) == 0) {
                return false;   /* found a divisor: composite */
            }
        }
        return true;
    }

    /*
     * Append a prime to the cache, growing storage when full.
     *
     * Doubles capacity on overflow to keep appends amortized O(1).
     *
     * prime:  the value to store.
     */
    void append(uint64_t prime) {
        if (count_ == capacity_) {
            capacity_ *= 2;
            cache_ = static_cast<uint64_t *>(
                realloc(cache_, capacity_ * sizeof(uint64_t)));
        }
        cache_[count_++] = prime;
    }
};

/*
 * Fill a buffer with the first `count` primes using a PrimeIterator.
 *
 * Free function demonstrating the iterator in use; keeps the class focused
 * on generation while this handles bulk extraction.
 *
 * out:    destination array, at least `count` elements.
 * count:  how many primes to generate.
 * return: the number of primes written (equal to count).
 */
uint32_t first_n_primes(uint64_t *out, uint32_t count) {
    PrimeIterator it;
    uint32_t written = 0;
    while (written < count) {
        out[written++] = it.next();
    }
    return written;
}
