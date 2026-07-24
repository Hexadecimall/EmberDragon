#include <stdint.h>
#include <string.h>

#define SAND_COLS 24
#define SAND_ROWS 32

enum CellKind {
    CELL_EMPTY = 0,
    CELL_SAND  = 1,
    CELL_WALL  = 2
};

typedef struct {
    uint8_t cell[SAND_ROWS][SAND_COLS];
    int32_t grains_settled;
} SandWorld;

void sand_clear(SandWorld *world) {
    memset(world->cell, CELL_EMPTY, sizeof(world->cell));
    world->grains_settled = 0;
}

void sand_spawn(SandWorld *world, int col) {
    if (col < 0 || col >= SAND_COLS) return;
    if (world->cell[0][col] == CELL_EMPTY)
        world->cell[0][col] = CELL_SAND;
}

void sand_place_wall(SandWorld *world, int row, int col) {
    if (row < 0 || row >= SAND_ROWS) return;
    if (col < 0 || col >= SAND_COLS) return;
    world->cell[row][col] = CELL_WALL;
}

static int is_open(const SandWorld *world, int row, int col) {
    if (row < 0 || row >= SAND_ROWS) return 0;
    if (col < 0 || col >= SAND_COLS) return 0;
    return world->cell[row][col] == CELL_EMPTY;
}

static int try_move_grain(SandWorld *world, int row, int col) {
    if (is_open(world, row + 1, col)) {
        world->cell[row + 1][col] = CELL_SAND;
        world->cell[row][col] = CELL_EMPTY;
        return 1;
    }
    if (is_open(world, row + 1, col - 1)) {
        world->cell[row + 1][col - 1] = CELL_SAND;
        world->cell[row][col] = CELL_EMPTY;
        return 1;
    }
    if (is_open(world, row + 1, col + 1)) {
        world->cell[row + 1][col + 1] = CELL_SAND;
        world->cell[row][col] = CELL_EMPTY;
        return 1;
    }
    return 0;
}

int sand_step(SandWorld *world) {
    int moved = 0;
    for (int row = SAND_ROWS - 2; row >= 0; row--) {
        for (int col = 0; col < SAND_COLS; col++) {
            if (world->cell[row][col] != CELL_SAND) continue;
            if (try_move_grain(world, row, col))
                moved++;
        }
    }
    if (moved == 0) {
        for (int row = 0; row < SAND_ROWS; row++)
            for (int col = 0; col < SAND_COLS; col++)
                if (world->cell[row][col] == CELL_SAND)
                    world->grains_settled++;
    }
    return moved;
}

int sand_simulate(SandWorld *world, int max_steps) {
    int step = 0;
    while (step < max_steps) {
        if (sand_step(world) == 0) break;
        step++;
    }
    return step;
}
