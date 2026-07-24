/* Classic recursion zoo: factorial, gcd, fibonacci, ackermann, hanoi-move-count. */
#include <stdio.h>
#include <stdint.h>

static uint64_t fact(uint64_t n) { return n < 2 ? 1 : n * fact(n - 1); }

static uint64_t gcd(uint64_t a, uint64_t b) { return b == 0 ? a : gcd(b, a % b); }

static uint64_t fib(uint64_t n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }

static uint64_t ackermann(uint64_t m, uint64_t n) {
    if (m == 0) return n + 1;
    if (n == 0) return ackermann(m - 1, 1);
    return ackermann(m - 1, ackermann(m, n - 1));
}

static uint64_t hanoi_moves(uint64_t disks) {
    if (disks == 0) return 0;
    return 2 * hanoi_moves(disks - 1) + 1;
}

int main(void) {
    printf("fact(10)      = %llu\n", (unsigned long long)fact(10));
    printf("gcd(1071,462) = %llu\n", (unsigned long long)gcd(1071, 462));
    printf("fib(20)       = %llu\n", (unsigned long long)fib(20));
    printf("ack(3,4)      = %llu\n", (unsigned long long)ackermann(3, 4));
    printf("hanoi(16)     = %llu\n", (unsigned long long)hanoi_moves(16));
    return 0;
}
