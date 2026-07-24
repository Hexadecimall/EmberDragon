/* Sieve of Eratosthenes: marks composites up to LIMIT, then prints the primes. */
#include <stdio.h>
#include <string.h>

#define LIMIT 100

int main(void) {
    unsigned char is_composite[LIMIT + 1];
    memset(is_composite, 0, sizeof(is_composite));

    for (int p = 2; (long)p * p <= LIMIT; p++) {
        if (!is_composite[p]) {
            for (int multiple = p * p; multiple <= LIMIT; multiple += p) {
                is_composite[multiple] = 1;
            }
        }
    }

    int count = 0;
    for (int n = 2; n <= LIMIT; n++) {
        if (!is_composite[n]) {
            printf("%d ", n);
            count++;
        }
    }
    printf("\n%d primes up to %d\n", count, LIMIT);
    return 0;
}
