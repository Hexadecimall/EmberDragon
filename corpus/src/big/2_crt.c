/*
 * crt.c — Linear congruence solving and the Chinese Remainder Theorem.
 *
 * Solves systems of simultaneous congruences x ≡ r_i (mod m_i) by folding
 * them pairwise into a single congruence. The implementation handles
 * non-coprime moduli (general CRT), reporting failure when the constraints
 * are inconsistent. Useful for clock/calendar math and modular reconstruction.
 */

#include <stdint.h>

/*
 * Extended Euclidean algorithm.
 * Parameters: a, b — inputs; x, y — receive Bezout coefficients such that
 *             a*x + b*y == gcd(a, b).
 * Returns the gcd. Iterative form to keep the stack flat. O(log min(a,b)).
 */
static int64_t ext_gcd(int64_t a, int64_t b, int64_t *x, int64_t *y) {
    int64_t old_r = a, r = b;
    int64_t old_x = 1, cx = 0;
    int64_t old_y = 0, cy = 1;
    while (r != 0) {
        int64_t q = old_r / r;
        int64_t t;
        t = old_r - q * r; old_r = r; r = t;
        t = old_x - q * cx; old_x = cx; cx = t;
        t = old_y - q * cy; old_y = cy; cy = t;
    }
    *x = old_x;
    *y = old_y;
    return old_r;
}

/*
 * Reduce v into the canonical residue range [0, m).
 * Parameters: v — any integer; m — positive modulus.
 * Returns v mod m as a non-negative value. Handles negative v correctly,
 * which the C `%` operator does not by itself.
 */
static int64_t mod_floor(int64_t v, int64_t m) {
    int64_t r = v % m;
    if (r < 0)
        r += m;
    return r;
}

/*
 * Combine two congruences into one.
 * Parameters: r1, m1 — first congruence x ≡ r1 (mod m1);
 *             r2, m2 — second congruence x ≡ r2 (mod m2);
 *             out_r, out_m — receive the merged residue and modulus on success.
 * Returns 1 on success and 0 if the two congruences are incompatible (no x
 * satisfies both). Works even when m1 and m2 are not coprime: a solution
 * exists iff (r2 - r1) is divisible by gcd(m1, m2), and the merged modulus
 * is lcm(m1, m2). Complexity O(log min(m1,m2)).
 */
int crt_combine(int64_t r1, int64_t m1, int64_t r2, int64_t m2,
                int64_t *out_r, int64_t *out_m) {
    int64_t p, q;
    int64_t g = ext_gcd(m1, m2, &p, &q);
    int64_t diff = r2 - r1;
    if (diff % g != 0)
        return 0;                 /* constraints conflict: no solution */

    /* lcm without overflowing more than necessary: (m1/g) * m2. */
    int64_t lcm = (m1 / g) * m2;

    /* Solve r1 + m1*t ≡ r2 (mod m2). The step factor is diff/g scaled by the
     * inverse-like coefficient p, reduced to the smaller modulus m2/g. */
    int64_t m2g = m2 / g;
    int64_t t = mod_floor((diff / g) * mod_floor(p, m2g), m2g);

    int64_t result = r1 + m1 * t;
    *out_r = mod_floor(result, lcm);
    *out_m = lcm;
    return 1;
}

/*
 * Solve a whole system of congruences via repeated pairwise combination.
 * Parameters: residues — array of r_i; moduli — array of m_i (each > 0);
 *             count — number of congruences;
 *             out_r, out_m — receive the unique solution residue and the
 *             combined modulus on success.
 * Returns 1 if the system is consistent (and writes the solution), or 0 if
 * any pair of congruences conflicts. On success x ≡ out_r (mod out_m) is the
 * complete solution set. Complexity O(count * log M).
 */
int crt_solve(const int64_t *residues, const int64_t *moduli, int count,
              int64_t *out_r, int64_t *out_m) {
    if (count <= 0)
        return 0;
    /* Seed the fold with the first congruence, normalized into range. */
    int64_t cur_r = mod_floor(residues[0], moduli[0]);
    int64_t cur_m = moduli[0];
    for (int i = 1; i < count; i++) {
        int64_t nr, nm;
        if (!crt_combine(cur_r, cur_m,
                         mod_floor(residues[i], moduli[i]), moduli[i],
                         &nr, &nm))
            return 0;             /* short-circuit on the first conflict */
        cur_r = nr;
        cur_m = nm;
    }
    *out_r = cur_r;
    *out_m = cur_m;
    return 1;
}
