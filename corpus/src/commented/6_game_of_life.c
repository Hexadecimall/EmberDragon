/*
 * Conway's Game of Life on a fixed-size toroidal (wrap-around) grid.
 *
 * The world is a rectangular grid of cells, each either alive (1) or dead (0).
 * Every generation, all cells are updated simultaneously according to Conway's
 * four rules, using a separate scratch buffer so that updates do not interfere
 * with one another. Edges wrap, so the top row is a neighbor of the bottom row
 * and the left column is a neighbor of the right column.
 */

#include <stdint.h>
#include <string.h>

/* Maximum supported world dimensions. The grid is stored row-major. */
#define LIFE_MAX_WIDTH  64
#define LIFE_MAX_HEIGHT 64

/*
 * A Life world: a width x height grid of cells plus a scratch buffer used to
 * hold the next generation while the current one is still being read.
 */
typedef struct {
    int     width;
    int     height;
    uint8_t cells[LIFE_MAX_HEIGHT][LIFE_MAX_WIDTH];  /* current generation */
    uint8_t next[LIFE_MAX_HEIGHT][LIFE_MAX_WIDTH];   /* scratch for next gen */
} LifeWorld;

/*
 * Initialize a world to the given dimensions with every cell dead.
 * Dimensions are clamped to the compile-time maximums so the fixed-size
 * backing arrays can never be overrun.
 * Returns 1 on success, or 0 if either requested dimension was non-positive.
 */
int life_init(LifeWorld *world, int width, int height) {
    if (width <= 0 || height <= 0) {
        return 0;  /* reject degenerate worlds rather than guessing a size */
    }
    if (width > LIFE_MAX_WIDTH)   width = LIFE_MAX_WIDTH;
    if (height > LIFE_MAX_HEIGHT) height = LIFE_MAX_HEIGHT;
    world->width = width;
    world->height = height;
    memset(world->cells, 0, sizeof(world->cells));
    return 1;
}

/*
 * Set the alive/dead state of a single cell.
 * Out-of-bounds coordinates are ignored so callers can be sloppy near edges.
 * Any non-zero 'alive' is normalized to exactly 1.
 */
void life_set(LifeWorld *world, int x, int y, int alive) {
    if (x < 0 || x >= world->width || y < 0 || y >= world->height) {
        return;
    }
    world->cells[y][x] = alive ? 1 : 0;
}

/*
 * Return the state (0 or 1) of the cell at (x, y), wrapping coordinates
 * around the grid edges so the world behaves as a torus. The modulo dance
 * with += dimension handles negative inputs, which C's % would otherwise
 * leave negative.
 */
int life_get_wrapped(const LifeWorld *world, int x, int y) {
    x %= world->width;
    if (x < 0) x += world->width;
    y %= world->height;
    if (y < 0) y += world->height;
    return world->cells[y][x];
}

/*
 * Count the live cells among the eight neighbors of (x, y), using wrap-around
 * lookups. The cell itself is excluded. Runs in constant time (a fixed 3x3
 * scan minus the center).
 */
int life_count_neighbors(const LifeWorld *world, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;  /* skip the center cell; only neighbors count */
            }
            count += life_get_wrapped(world, x + dx, y + dy);
        }
    }
    return count;
}

/*
 * Advance the world by exactly one generation.
 *
 * Conway's rules: a live cell with 2 or 3 live neighbors survives; a dead
 * cell with exactly 3 live neighbors becomes alive; everything else dies or
 * stays dead. The next generation is computed into the scratch buffer first
 * and then copied back, so every cell sees the old neighborhood.
 * Runs in O(width * height) time.
 */
void life_step(LifeWorld *world) {
    for (int y = 0; y < world->height; ++y) {
        for (int x = 0; x < world->width; ++x) {
            int neighbors = life_count_neighbors(world, x, y);
            int alive = world->cells[y][x];
            /* Survival: a live cell needs 2 or 3 neighbors.
               Birth: a dead cell springs to life on exactly 3. */
            if (alive) {
                world->next[y][x] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                world->next[y][x] = (neighbors == 3) ? 1 : 0;
            }
        }
    }
    /* Commit the freshly computed generation as the current one. */
    memcpy(world->cells, world->next, sizeof(world->cells));
}

/*
 * Return the total number of live cells currently in the world.
 * Useful for detecting extinction or stable populations. O(width * height).
 */
int life_population(const LifeWorld *world) {
    int total = 0;
    for (int y = 0; y < world->height; ++y) {
        for (int x = 0; x < world->width; ++x) {
            total += world->cells[y][x];
        }
    }
    return total;
}
