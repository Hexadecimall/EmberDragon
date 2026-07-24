#include <cstdio>

// Fixed-size square matrix multiply: C = A * B (3x3), classic triple loop.
static const int N = 3;

static void matmul(const int A[N][N], const int B[N][N], int C[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++)
                sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
}

int main() {
    int A[N][N] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    int B[N][N] = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
    int C[N][N];
    matmul(A, B, C);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            std::printf("%4d", C[i][j]);
        std::printf("\n");
    }
    return 0;
}
