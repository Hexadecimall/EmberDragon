#include <stdint.h>
#include <string.h>

#define SIEVE_CAPACITY 256

typedef struct PrimeSieve {
    int32_t limit;
    uint8_t composite[SIEVE_CAPACITY];
    int32_t primes[SIEVE_CAPACITY];
    int32_t primeCount;
} PrimeSieve;

void sieve_build(PrimeSieve *sieve, int32_t limit) {
    if (limit > SIEVE_CAPACITY) {
        limit = SIEVE_CAPACITY;
    }
    sieve->limit = limit;
    sieve->primeCount = 0;
    memset(sieve->composite, 0, sizeof(sieve->composite));
    for (int32_t candidate = 2; candidate < limit; candidate++) {
        if (!sieve->composite[candidate]) {
            sieve->primes[sieve->primeCount] = candidate;
            sieve->primeCount++;
            for (int32_t multiple = candidate * 2; multiple < limit; multiple += candidate) {
                sieve->composite[multiple] = 1;
            }
        }
    }
}

int sieve_is_prime(const PrimeSieve *sieve, int32_t value) {
    if (value < 2 || value >= sieve->limit) {
        return 0;
    }
    return sieve->composite[value] == 0;
}

int32_t sieve_nth_prime(const PrimeSieve *sieve, int32_t index) {
    if (index < 0 || index >= sieve->primeCount) {
        return -1;
    }
    return sieve->primes[index];
}

int32_t count_primes_below(const PrimeSieve *sieve, int32_t bound) {
    int32_t count = 0;
    for (int32_t i = 0; i < sieve->primeCount; i++) {
        if (sieve->primes[i] < bound) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

int32_t next_prime_after(const PrimeSieve *sieve, int32_t value) {
    for (int32_t i = 0; i < sieve->primeCount; i++) {
        if (sieve->primes[i] > value) {
            return sieve->primes[i];
        }
    }
    return -1;
}

int32_t factorize(const PrimeSieve *sieve, int32_t number, int32_t *factorsOut) {
    int32_t written = 0;
    int32_t remaining = number;
    for (int32_t i = 0; i < sieve->primeCount && remaining > 1; i++) {
        int32_t prime = sieve->primes[i];
        while (remaining % prime == 0) {
            factorsOut[written] = prime;
            written++;
            remaining /= prime;
        }
    }
    if (remaining > 1) {
        factorsOut[written] = remaining;
        written++;
    }
    return written;
}
