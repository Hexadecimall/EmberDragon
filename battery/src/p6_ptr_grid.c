/* Pointer arithmetic with casts + a 2D array passed to a function.
   Walks a byte buffer two ways: as int32 via cast, and row/col via 2D param. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ROWS 3
#define COLS 4

/* genuine 2D array parameter: COLS must be known to the callee */
static int trace(int g[ROWS][COLS]) {
    int sum = 0;
    for (int r = 0; r < ROWS; ++r)
        sum += g[r][r % COLS];          /* diagonal-ish */
    return sum;
}

/* reinterpret a flat byte block as int32s via pointer cast + arithmetic */
static uint32_t fold_bytes(const uint8_t *base, size_t n) {
    const uint32_t *w = (const uint32_t *)base;
    uint32_t acc = 0;
    for (size_t i = 0; i < n / 4; ++i)
        acc ^= *(w + i) + (uint32_t)i;  /* w+i, not &w[i] */
    return acc;
}

int main(void) {
    int grid[ROWS][COLS] = {
        {  1,  2,  3,  4 },
        { 10, 20, 30, 40 },
        {100,200,300,400 }
    };
    printf("trace = %d\n", trace(grid));

    uint8_t buf[16];
    for (int i = 0; i < 16; ++i) buf[i] = (uint8_t)(i * 7 + 1);
    printf("fold = 0x%08X\n", fold_bytes(buf, sizeof buf));

    /* cast a row pointer and step by element size */
    int *p = &grid[1][0];
    printf("p[0]=%d *(p+2)=%d\n", p[0], *(p + 2));
    return 0;
}
