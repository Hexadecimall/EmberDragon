/*
 * matrix.c — Fixed-size 3x3 integer matrix algebra.
 *
 * Provides construction, multiplication, transpose, determinant, and
 * trace operations on 3x3 matrices of 64-bit signed integers. All math
 * is exact integer arithmetic; no floating point is involved, so the
 * determinant and products are computed without rounding error.
 */

#include <stdint.h>

/* A dense 3x3 matrix stored in row-major order. */
typedef struct {
    int64_t m[3][3];
} Matrix3;

/*
 * Initialize a matrix to the multiplicative identity (1 on the diagonal,
 * 0 elsewhere).
 *   out: destination matrix, overwritten in place.
 * Returns nothing; the caller owns `out`.
 */
void matrix_identity(Matrix3 *out) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            /* Diagonal entries are 1, off-diagonal entries are 0. */
            out->m[row][col] = (row == col) ? 1 : 0;
        }
    }
}

/*
 * Multiply two matrices: result = lhs * rhs (standard row-by-column).
 *   lhs, rhs: input operands (left unchanged).
 *   result:   destination; must not alias lhs or rhs.
 * Runs in O(27) scalar multiplies. Caller owns `result`.
 */
void matrix_multiply(const Matrix3 *lhs, const Matrix3 *rhs, Matrix3 *result) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int64_t sum = 0;
            /* Dot product of lhs row `row` with rhs column `col`. */
            for (int k = 0; k < 3; k++) {
                sum += lhs->m[row][k] * rhs->m[k][col];
            }
            result->m[row][col] = sum;
        }
    }
}

/*
 * Transpose a matrix: result[i][j] = src[j][i].
 *   src:    input matrix (unchanged).
 *   result: destination; must not alias src.
 */
void matrix_transpose(const Matrix3 *src, Matrix3 *result) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            result->m[row][col] = src->m[col][row];
        }
    }
}

/*
 * Compute the determinant via cofactor expansion along the first row.
 *   src: input matrix (unchanged).
 * Returns the exact signed determinant. A zero result means the matrix
 * is singular (non-invertible).
 */
int64_t matrix_determinant(const Matrix3 *src) {
    const int64_t (*a)[3] = src->m;
    /* Three 2x2 minors, each scaled by its first-row entry and sign. */
    int64_t minorA = a[1][1] * a[2][2] - a[1][2] * a[2][1];
    int64_t minorB = a[1][0] * a[2][2] - a[1][2] * a[2][0];
    int64_t minorC = a[1][0] * a[2][1] - a[1][1] * a[2][0];
    return a[0][0] * minorA - a[0][1] * minorB + a[0][2] * minorC;
}

/*
 * Compute the trace (sum of the diagonal entries).
 *   src: input matrix (unchanged).
 * Returns the sum a[0][0] + a[1][1] + a[2][2].
 */
int64_t matrix_trace(const Matrix3 *src) {
    int64_t total = 0;
    for (int i = 0; i < 3; i++) {
        total += src->m[i][i];
    }
    return total;
}

/*
 * Test whether a matrix equals the identity.
 *   src: input matrix (unchanged).
 * Returns 1 if every entry matches the identity, 0 otherwise.
 */
int matrix_is_identity(const Matrix3 *src) {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int64_t expected = (row == col) ? 1 : 0;
            if (src->m[row][col] != expected) {
                return 0;  /* First mismatch: definitely not identity. */
            }
        }
    }
    return 1;
}
