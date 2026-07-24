/*
 * matrix.c — Fixed-size square integer matrix algebra.
 *
 * Provides addition, multiplication, identity construction, and fast
 * exponentiation for small square matrices of 64-bit signed integers.
 * Matrix exponentiation is the workhorse here: it lets callers evaluate
 * linear recurrences (Fibonacci, path counts in graphs, etc.) in
 * O(n^3 log p) time instead of O(p) by repeated squaring.
 */

#include <stdint.h>
#include <string.h>

/* Maximum supported dimension. Kept small so matrices live on the stack
 * and the O(n^3) multiply stays cheap. */
#define MATRIX_MAX_DIM 8

/*
 * A dense square matrix stored row-major in a fixed buffer. Only the top
 * `dim x dim` sub-block is meaningful; the rest of `data` is ignored.
 */
typedef struct {
    int      dim;                                   /* active dimension, 1..MATRIX_MAX_DIM */
    int64_t  data[MATRIX_MAX_DIM][MATRIX_MAX_DIM];  /* row-major elements */
} Matrix;

/*
 * Reset `m` to the additive identity (all zeros) of dimension `dim`.
 * Parameters: m — matrix to initialize; dim — desired dimension.
 * Returns nothing. No-op behavior is well defined for the whole buffer
 * because we clear every slot, not just the active block.
 */
void matrix_zero(Matrix *m, int dim) {
    m->dim = dim;
    memset(m->data, 0, sizeof(m->data));
}

/*
 * Set `m` to the multiplicative identity (1s on the diagonal) of size `dim`.
 * Parameters: m — destination; dim — dimension.
 * Returns nothing. Used as the seed accumulator for matrix_power().
 */
void matrix_identity(Matrix *m, int dim) {
    matrix_zero(m, dim);
    for (int i = 0; i < dim; i++)
        m->data[i][i] = 1;
}

/*
 * Compute out = a + b element-wise.
 * Parameters: out — result (may alias a or b); a, b — operands.
 * Returns nothing. Assumes a->dim == b->dim; result inherits that dim.
 * Complexity O(n^2).
 */
void matrix_add(Matrix *out, const Matrix *a, const Matrix *b) {
    int n = a->dim;
    out->dim = n;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            out->data[i][j] = a->data[i][j] + b->data[i][j];
}

/*
 * Compute out = a * b (standard matrix product).
 * Parameters: out — result; a, b — operands of equal dimension.
 * Returns nothing. Because the product is accumulated into a temporary,
 * `out` is allowed to alias `a` or `b` safely. Complexity O(n^3).
 */
void matrix_multiply(Matrix *out, const Matrix *a, const Matrix *b) {
    int n = a->dim;
    Matrix tmp;
    matrix_zero(&tmp, n);
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++) {
            /* Hoist a[i][k] out of the inner loop; if it is zero the whole
             * row contribution is zero and we can skip the work. */
            int64_t aik = a->data[i][k];
            if (aik == 0)
                continue;
            for (int j = 0; j < n; j++)
                tmp.data[i][j] += aik * b->data[k][j];
        }
    *out = tmp;
}

/*
 * Raise matrix `base` to the non-negative integer power `exp`.
 * Parameters: out — result; base — square matrix; exp — exponent >= 0.
 * Returns nothing. exp == 0 yields the identity. Uses binary
 * exponentiation (repeated squaring), so complexity is O(n^3 log exp).
 * Note: results are NOT reduced modulo anything — callers needing modular
 * recurrences should reduce externally or extend matrix_multiply().
 */
void matrix_power(Matrix *out, const Matrix *base, uint64_t exp) {
    int n = base->dim;
    Matrix result, b = *base;
    matrix_identity(&result, n);
    while (exp > 0) {
        /* Multiply the accumulator only on set bits of the exponent. */
        if (exp & 1u)
            matrix_multiply(&result, &result, &b);
        exp >>= 1;
        if (exp > 0)            /* avoid one wasted square on the last bit */
            matrix_multiply(&b, &b, &b);
    }
    *out = result;
}

/*
 * Return the trace (sum of diagonal entries) of `m`.
 * Parameters: m — matrix to inspect.
 * Returns the int64_t sum a[0][0] + ... + a[n-1][n-1]. O(n).
 */
int64_t matrix_trace(const Matrix *m) {
    int64_t sum = 0;
    for (int i = 0; i < m->dim; i++)
        sum += m->data[i][i];
    return sum;
}
