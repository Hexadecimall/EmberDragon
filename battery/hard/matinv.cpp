// 4x4 matrix inverse via Gauss-Jordan elimination with partial pivoting.
// Self-contained, deterministic, std-lib only. Builds a fixed non-singular
// matrix, augments with the identity, reduces to RREF, then verifies that
// A * A^-1 == I to within tolerance and returns a checksum from main().
#include <array>
#include <cstdint>
#include <cmath>

namespace {

constexpr int N = 4;
using Row = std::array<double, 2 * N>;   // [ A | I ]
using Aug = std::array<Row, N>;
using Mat = std::array<std::array<double, N>, N>;

// Build the augmented matrix [A | I].
Aug make_augmented(const Mat& a) {
    Aug m{};
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) m[i][j] = a[i][j];
        for (int j = 0; j < N; ++j) m[i][N + j] = (i == j) ? 1.0 : 0.0;
    }
    return m;
}

// Gauss-Jordan with partial pivoting. Returns false if singular.
bool gauss_jordan(Aug& m) {
    for (int col = 0; col < N; ++col) {
        // Find pivot: largest magnitude in this column at/below the diagonal.
        int pivot = col;
        double best = std::fabs(m[col][col]);
        for (int r = col + 1; r < N; ++r) {
            double v = std::fabs(m[r][col]);
            if (v > best) { best = v; pivot = r; }
        }
        if (best < 1e-12) return false;          // singular
        if (pivot != col) std::swap(m[pivot], m[col]);

        // Normalize the pivot row.
        double inv = 1.0 / m[col][col];
        for (int j = 0; j < 2 * N; ++j) m[col][j] *= inv;

        // Eliminate this column from every other row.
        for (int r = 0; r < N; ++r) {
            if (r == col) continue;
            double f = m[r][col];
            if (f == 0.0) continue;
            for (int j = 0; j < 2 * N; ++j) m[r][j] -= f * m[col][j];
        }
    }
    return true;
}

// Extract the right half (the inverse) into a plain matrix.
Mat extract_inverse(const Aug& m) {
    Mat inv{};
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            inv[i][j] = m[i][N + j];
    return inv;
}

// Standard matrix multiply.
Mat multiply(const Mat& a, const Mat& b) {
    Mat c{};
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            double s = 0.0;
            for (int k = 0; k < N; ++k) s += a[i][k] * b[k][j];
            c[i][j] = s;
        }
    return c;
}

} // namespace

int main() {
    // Fixed, well-conditioned, non-singular matrix.
    Mat A = {{
        {{ 4.0,  3.0,  2.0,  1.0}},
        {{ 1.0,  5.0,  1.0,  2.0}},
        {{ 2.0,  1.0,  6.0,  3.0}},
        {{ 1.0,  2.0,  3.0,  7.0}}
    }};

    Aug m = make_augmented(A);
    if (!gauss_jordan(m)) return 255;            // should not happen here

    Mat inv = extract_inverse(m);
    Mat prod = multiply(A, inv);

    // Verify A * A^-1 is the identity; accumulate residual error.
    double residual = 0.0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            double expected = (i == j) ? 1.0 : 0.0;
            residual += std::fabs(prod[i][j] - expected);
        }

    // Checksum of the inverse entries, quantized for stability.
    std::int64_t sum = 0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            sum += static_cast<std::int64_t>(std::llround(inv[i][j] * 10000.0));

    // residual should be tiny; bad result -> nonzero high bits.
    int err_flag = (residual > 1e-6) ? 128 : 0;
    return static_cast<int>((sum & 0x7F) | err_flag);
}
