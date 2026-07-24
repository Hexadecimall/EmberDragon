// Heavy const-folding: constexpr factorial / fib / gcd that the compiler
// at -O0 still evaluates at compile time because the results seed constants.
// The decompiler should see baked-in literals with no runtime loop for these.
#include <cstdio>
#include <cstdint>

constexpr uint64_t fact(int n) { return n <= 1 ? 1 : n * fact(n - 1); }
constexpr uint64_t fib(int n)  { return n < 2 ? (uint64_t)n : fib(n - 1) + fib(n - 2); }
constexpr int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

// folds an entire arithmetic tree into one constant
constexpr int blob() {
    int s = 0;
    for (int i = 1; i <= 10; ++i) s += i * i;      // 385
    return (s * 3 + 7) ^ 0x55;
}

int main() {
    constexpr uint64_t F = fact(12);
    constexpr uint64_t B = fib(20);
    constexpr int G = gcd(1071, 462);
    constexpr int X = blob();
    static_assert(F == 479001600ULL, "fact");
    static_assert(G == 21, "gcd");
    std::printf("fact(12)=%llu fib(20)=%llu gcd=%d blob=%d\n",
                (unsigned long long)F, (unsigned long long)B, G, X);
    return 0;
}
