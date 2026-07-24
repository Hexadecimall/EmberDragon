/*
 * combinatorics.cpp — Binomial coefficients and related counting functions.
 *
 * Offers two complementary ways to compute "n choose k": an exact 64-bit
 * direct product for small inputs, and a modular variant backed by a
 * precomputed factorial table for repeated queries under a prime modulus.
 * Also provides permutation counts and a Pascal-triangle row builder.
 */

#include <cstdint>
#include <vector>

/*
 * Compute C(n, k) exactly in 64-bit integer arithmetic.
 * Parameters: n — set size; k — subset size.
 * Returns the binomial coefficient, or 0 when k < 0 or k > n. Multiplies
 * and divides incrementally so each partial result stays an exact integer,
 * which keeps the running value as small as possible to delay overflow.
 * Complexity O(k). Caller must ensure the true result fits in int64_t.
 */
int64_t choose(int64_t n, int64_t k) {
    if (k < 0 || k > n)
        return 0;
    /* C(n,k) == C(n,n-k); pick the smaller k to minimize iterations and
     * the size of the intermediate product. */
    if (k > n - k)
        k = n - k;
    int64_t result = 1;
    for (int64_t i = 0; i < k; i++) {
        /* result *= (n - i) / (i + 1). The division is always exact here
         * because result holds C(n, i) which (n-i) divides evenly. */
        result = result * (n - i) / (i + 1);
    }
    return result;
}

/*
 * Compute the number of ordered arrangements P(n, k) = n! / (n-k)!.
 * Parameters: n — set size; k — number of positions to fill.
 * Returns the permutation count, or 0 when k < 0 or k > n. O(k).
 */
int64_t permute(int64_t n, int64_t k) {
    if (k < 0 || k > n)
        return 0;
    int64_t result = 1;
    for (int64_t i = 0; i < k; i++)
        result *= (n - i);        /* n * (n-1) * ... * (n-k+1) */
    return result;
}

/*
 * A reusable table of factorials and inverse factorials modulo a fixed
 * prime, enabling O(1) binomial queries after O(n) setup.
 */
class ModBinomial {
public:
    /*
     * Build factorial tables for arguments up to `maxN` under prime `mod`.
     * Parameters: maxN — largest n any later query will use; mod — a prime.
     * Notable: precomputes inverse factorials via Fermat's little theorem,
     * so each subsequent choose() is constant time. Setup is O(maxN log mod).
     */
    ModBinomial(int maxN, int64_t mod) : mod_(mod) {
        fact_.resize(maxN + 1);
        inv_fact_.resize(maxN + 1);
        fact_[0] = 1;
        for (int i = 1; i <= maxN; i++)
            fact_[i] = fact_[i - 1] * i % mod_;
        /* Inverse of the top factorial via a^(p-2) mod p, then walk down:
         * inv_fact_[i-1] = inv_fact_[i] * i, which avoids a power per entry. */
        inv_fact_[maxN] = powmod(fact_[maxN], mod_ - 2);
        for (int i = maxN; i >= 1; i--)
            inv_fact_[i - 1] = inv_fact_[i] * i % mod_;
    }

    /*
     * Return C(n, k) modulo the configured prime.
     * Parameters: n, k — arguments within the table's range.
     * Returns the modular binomial coefficient, or 0 when k is out of [0, n].
     * Complexity O(1).
     */
    int64_t choose(int n, int k) const {
        if (k < 0 || k > n)
            return 0;
        /* C(n,k) = n! * (k!)^-1 * ((n-k)!)^-1, all taken mod p. */
        return fact_[n] * inv_fact_[k] % mod_ * inv_fact_[n - k] % mod_;
    }

private:
    /*
     * Fast modular exponentiation used only during table construction.
     * Parameters: base — value to raise; exp — non-negative exponent.
     * Returns base^exp mod mod_. O(log exp).
     */
    int64_t powmod(int64_t base, int64_t exp) const {
        int64_t r = 1;
        base %= mod_;
        while (exp > 0) {
            if (exp & 1)
                r = r * base % mod_;
            base = base * base % mod_;
            exp >>= 1;
        }
        return r;
    }

    int64_t mod_;                  /* the prime modulus */
    std::vector<int64_t> fact_;    /* fact_[i] = i! mod p */
    std::vector<int64_t> inv_fact_;/* inv_fact_[i] = (i!)^-1 mod p */
};

/*
 * Generate row `n` of Pascal's triangle (the coefficients C(n, 0..n)).
 * Parameters: n — non-negative row index.
 * Returns a vector of length n+1. Each entry is derived from its neighbor
 * in the previous conceptual row using the multiplicative recurrence
 * C(n,k) = C(n,k-1) * (n-k+1) / k, so no triangle storage is needed. O(n).
 */
std::vector<int64_t> pascal_row(int n) {
    std::vector<int64_t> row(n + 1);
    row[0] = 1;
    for (int k = 1; k <= n; k++)
        row[k] = row[k - 1] * (n - k + 1) / k;
    return row;
}
