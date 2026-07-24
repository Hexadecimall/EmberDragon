#include <stdint.h>
#include <string.h>

#define GRID_WIDTH  16
#define GRID_HEIGHT 16

typedef struct {
    uint8_t cells[GRID_HEIGHT][GRID_WIDTH];
    int32_t generation;
} LifeBoard;

void board_init(LifeBoard *board) {
    memset(board->cells, 0, sizeof(board->cells));
    board->generation = 0;
}

void board_set_cell(LifeBoard *board, int row, int col, uint8_t alive) {
    if (row < 0 || row >= GRID_HEIGHT) return;
    if (col < 0 || col >= GRID_WIDTH) return;
    board->cells[row][col] = alive ? 1 : 0;
}

int count_live_neighbors(const LifeBoard *board, int row, int col) {
    int total = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            if (nr < 0 || nr >= GRID_HEIGHT) continue;
            if (nc < 0 || nc >= GRID_WIDTH) continue;
            total += board->cells[nr][nc];
        }
    }
    return total;
}

void board_step(LifeBoard *board) {
    uint8_t next[GRID_HEIGHT][GRID_WIDTH];
    for (int row = 0; row < GRID_HEIGHT; row++) {
        for (int col = 0; col < GRID_WIDTH; col++) {
            int neighbors = count_live_neighbors(board, row, col);
            uint8_t current = board->cells[row][col];
            uint8_t result = 0;
            if (current) {
                if (neighbors == 2 || neighbors == 3) result = 1;
            } else {
                if (neighbors == 3) result = 1;
            }
            next[row][col] = result;
        }
    }
    memcpy(board->cells, next, sizeof(next));
    board->generation++;
}

int board_population(const LifeBoard *board) {
    int sum = 0;
    for (int row = 0; row < GRID_HEIGHT; row++) {
        for (int col = 0; col < GRID_WIDTH; col++) {
            sum += board->cells[row][col];
        }
    }
    return sum;
}

int board_run(LifeBoard *board, int steps) {
    for (int i = 0; i < steps; i++) {
        board_step(board);
        if (board_population(board) == 0) return i + 1;
    }
    return steps;
}
