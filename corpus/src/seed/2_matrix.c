#include <stdio.h>
#include <stdint.h>

#define MAT_DIM 4

typedef struct Matrix {
    int32_t rows;
    int32_t cols;
    int64_t cells[MAT_DIM][MAT_DIM];
} Matrix;

void matrix_identity(Matrix *matrix, int32_t size) {
    matrix->rows = size;
    matrix->cols = size;
    for (int32_t r = 0; r < size; r++) {
        for (int32_t c = 0; c < size; c++) {
            matrix->cells[r][c] = (r == c) ? 1 : 0;
        }
    }
}

void matrix_multiply(const Matrix *left, const Matrix *right, Matrix *product) {
    product->rows = left->rows;
    product->cols = right->cols;
    for (int32_t r = 0; r < left->rows; r++) {
        for (int32_t c = 0; c < right->cols; c++) {
            int64_t accumulator = 0;
            for (int32_t k = 0; k < left->cols; k++) {
                accumulator += left->cells[r][k] * right->cells[k][c];
            }
            product->cells[r][c] = accumulator;
        }
    }
}

void matrix_transpose(const Matrix *source, Matrix *result) {
    result->rows = source->cols;
    result->cols = source->rows;
    for (int32_t r = 0; r < source->rows; r++) {
        for (int32_t c = 0; c < source->cols; c++) {
            result->cells[c][r] = source->cells[r][c];
        }
    }
}

int64_t matrix_trace(const Matrix *matrix) {
    int64_t sum = 0;
    int32_t limit = matrix->rows < matrix->cols ? matrix->rows : matrix->cols;
    for (int32_t i = 0; i < limit; i++) {
        sum += matrix->cells[i][i];
    }
    return sum;
}

int matrix_is_symmetric(const Matrix *matrix) {
    if (matrix->rows != matrix->cols) {
        return 0;
    }
    for (int32_t r = 0; r < matrix->rows; r++) {
        for (int32_t c = r + 1; c < matrix->cols; c++) {
            if (matrix->cells[r][c] != matrix->cells[c][r]) {
                return 0;
            }
        }
    }
    return 1;
}

void matrix_scale(Matrix *matrix, int64_t factor) {
    for (int32_t r = 0; r < matrix->rows; r++) {
        for (int32_t c = 0; c < matrix->cols; c++) {
            matrix->cells[r][c] *= factor;
        }
    }
}
