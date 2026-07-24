// Nested loops: triple-nested matrix multiply plus a sieve via doubly-nested loops.
#include <iostream>
#include <vector>

int main() {
    const int N = 4;
    std::vector<std::vector<int>> a(N, std::vector<int>(N));
    std::vector<std::vector<int>> b(N, std::vector<int>(N));
    std::vector<std::vector<int>> c(N, std::vector<int>(N, 0));

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            a[i][j] = i * N + j + 1;
            b[i][j] = (i == j) ? 2 : 0; // 2*identity
        }

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            for (int k = 0; k < N; ++k)
                c[i][j] += a[i][k] * b[k][j];

    long trace = 0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (i == j) trace += c[i][j];

    // Sieve of Eratosthenes (nested loop with inner step-by-p loop).
    const int M = 30;
    std::vector<bool> sieve(M + 1, true);
    for (int p = 2; p * p <= M; ++p)
        if (sieve[p])
            for (int q = p * p; q <= M; q += p)
                sieve[q] = false;

    std::cout << "trace(2A) = " << trace << "\nprimes<=30:";
    for (int p = 2; p <= M; ++p)
        if (sieve[p]) std::cout << ' ' << p;
    std::cout << '\n';
    return 0;
}
