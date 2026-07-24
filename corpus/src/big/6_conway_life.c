/*
 * conway_life.c -- Conway's Game of Life on a fixed-size toroidal grid.
 *
 * The Game of Life is a zero-player cellular automaton: each cell is alive or
 * dead, and the next generation is computed purely from the eight neighbours of
 * each cell. This module stores two generations as flat integer buffers and
 * advances the simulation by double-buffering between them. The grid wraps
 * around at the edges (a torus) so gliders can travel forever.
 */

#include <stdint.h>
#include <string.h>

#define LIFE_WIDTH  32
#define LIFE_HEIGHT 32
#define LIFE_CELLS  (LIFE_WIDTH * LIFE_HEIGHT)

/*
 * A LifeBoard holds the current generation plus a scratch buffer for the next
 * one. Cells are stored row-major; each byte is 0 (dead) or 1 (alive).
 */
typedef struct {
    uint8_t current[LIFE_CELLS]; /* the live generation the caller observes  */
    uint8_t scratch[LIFE_CELLS]; /* working space for the generation we build */
} LifeBoard;

/*
 * Clear every cell to dead. Must be called before a board is used so the
 * scratch buffer never contains uninitialised garbage. O(W*H).
 */
void life_clear(LifeBoard *board) {
    memset(board->current, 0, LIFE_CELLS);
    memset(board->scratch, 0, LIFE_CELLS);
}

/*
 * Read the cell at (x, y) with toroidal wrap-around. Coordinates outside the
 * grid are folded back in, so x == -1 reads the rightmost column. Returns 1 if
 * the cell is alive, 0 otherwise. O(1).
 */
static int life_get(const LifeBoard *board, int x, int y) {
    /* Add the dimension before taking the modulus so negative inputs stay
     * non-negative (C's % can yield a negative result otherwise). */
    int wrapped_x = ((x % LIFE_WIDTH) + LIFE_WIDTH) % LIFE_WIDTH;
    int wrapped_y = ((y % LIFE_HEIGHT) + LIFE_HEIGHT) % LIFE_HEIGHT;
    return board->current[wrapped_y * LIFE_WIDTH + wrapped_x];
}

/*
 * Set the cell at (x, y) to the given state (any non-zero value means alive).
 * Coordinates are assumed in-range; used to seed patterns. O(1).
 */
void life_set(LifeBoard *board, int x, int y, int alive) {
    board->current[y * LIFE_WIDTH + x] = alive ? 1 : 0;
}

/*
 * Count how many of the eight Moore neighbours of (x, y) are alive. Uses
 * toroidal lookups so edge cells see the opposite side of the board. The
 * return value is in the range 0..8. O(1).
 */
static int life_neighbours(const LifeBoard *board, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
                continue; /* the cell itself is not its own neighbour */
            count += life_get(board, x + dx, y + dy);
        }
    }
    return count;
}

/*
 * Advance the board by exactly one generation. The next state is written into
 * the scratch buffer and then swapped in, so reads during the step always see
 * a consistent previous generation. Applies the classic B3/S23 rule:
 * a dead cell with 3 live neighbours is born, a live cell survives on 2 or 3.
 * O(W*H).
 */
void life_step(LifeBoard *board) {
    for (int y = 0; y < LIFE_HEIGHT; y++) {
        for (int x = 0; x < LIFE_WIDTH; x++) {
            int neighbours = life_neighbours(board, x, y);
            int alive = board->current[y * LIFE_WIDTH + x];
            int next;
            if (alive)
                next = (neighbours == 2 || neighbours == 3); /* survival */
            else
                next = (neighbours == 3);                    /* birth */
            board->scratch[y * LIFE_WIDTH + x] = (uint8_t)next;
        }
    }
    /* Promote the freshly computed generation to be the current one. */
    memcpy(board->current, board->scratch, LIFE_CELLS);
}

/*
 * Count the total number of live cells on the board. Useful for detecting a
 * dead (empty) board or comparing population over time. O(W*H).
 */
int life_population(const LifeBoard *board) {
    int total = 0;
    for (int i = 0; i < LIFE_CELLS; i++)
        total += board->current[i];
    return total;
}

/*
 * Run the simulation forward by the requested number of generations and return
 * the final population. Convenience wrapper over repeated life_step calls.
 * O(generations * W * H).
 */
int life_run(LifeBoard *board, int generations) {
    for (int g = 0; g < generations; g++)
        life_step(board);
    return life_population(board);
}
