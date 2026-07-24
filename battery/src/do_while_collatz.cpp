// do/while loops: Collatz step-counter and a digit-reversal, both post-test loops.
#include <iostream>
#include <cstdint>

static int collatz_steps(uint64_t n) {
    int steps = 0;
    do {
        if (n & 1ULL) n = 3 * n + 1;
        else          n >>= 1;
        ++steps;
    } while (n != 1);
    return steps;
}

static uint64_t reverse_digits(uint64_t n) {
    uint64_t r = 0;
    do {
        r = r * 10 + (n % 10);
        n /= 10;
    } while (n != 0);
    return r;
}

int main() {
    uint64_t seeds[] = { 6, 7, 27, 97 };
    for (uint64_t s : seeds)
        std::cout << "collatz(" << s << ") = "
                  << collatz_steps(s) << " steps\n";

    uint64_t v = 1234500;
    std::cout << "reverse(" << v << ") = " << reverse_digits(v) << '\n';
    return 0;
}
