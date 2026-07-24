/*
 * langton_ant.c -- Simulation of Langton's Ant on a toroidal grid.
 *
 * Langton's Ant is a two-state cellular automaton driven by a single moving
 * agent. On a white cell the ant turns right, flips the cell to black, and steps
 * forward; on a black cell it turns left, flips to white, and steps forward.
 * From simple rules emerges chaotic behaviour that eventually settles into a
 * periodic "highway". The grid wraps around so the ant never leaves the field.
 */

#include <stdint.h>
#include <string.h>

/* Compass headings, ordered clockwise so a right turn is +1 (mod 4). */
typedef enum {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3
} Direction;

/*
 * The Ant tracks its position, heading, and the board it walks on. Cells are
 * stored row-major as one byte each: 0 = white, 1 = black.
 */
typedef struct {
    int       width;
    int       height;
    uint8_t  *cells;    /* width*height colour bytes, owned by the Ant */
    int       x;        /* current column */
    int       y;        /* current row */
    Direction heading;  /* the way the ant currently faces */
} Ant;

/*
 * Initialise an Ant in place over a caller-provided cell buffer. The buffer is
 * cleared to all-white and the ant is placed at the centre facing up. `cells`
 * must hold width*height bytes and outlive the Ant. O(w*h).
 */
void ant_init(Ant *ant, uint8_t *cells, int width, int height) {
    ant->width = width;
    ant->height = height;
    ant->cells = cells;
    memset(cells, 0, (size_t)width * height); /* start on a blank field */
    ant->x = width / 2;
    ant->y = height / 2;
    ant->heading = DIR_UP;
}

/*
 * Advance one logical step. Returns the displacement vector for the current
 * heading via *dx/*dy without moving the ant; centralises the heading-to-vector
 * mapping so the step logic reads cleanly. O(1).
 */
static void heading_vector(Direction heading, int *dx, int *dy) {
    switch (heading) {
        case DIR_UP:    *dx = 0;  *dy = -1; break;
        case DIR_RIGHT: *dx = 1;  *dy = 0;  break;
        case DIR_DOWN:  *dx = 0;  *dy = 1;  break;
        case DIR_LEFT:  *dx = -1; *dy = 0;  break;
    }
}

/*
 * Execute one step of Langton's Ant: turn based on the current cell's colour,
 * flip that cell, then move forward one cell with toroidal wrap. O(1).
 */
void ant_step(Ant *ant) {
    int idx = ant->y * ant->width + ant->x;

    if (ant->cells[idx] == 0) {
        /* White cell: turn right (clockwise) and blacken it. */
        ant->heading = (Direction)((ant->heading + 1) & 3);
        ant->cells[idx] = 1;
    } else {
        /* Black cell: turn left (counter-clockwise) and whiten it.
         * Adding 3 mod 4 is the same as subtracting 1 without going negative. */
        ant->heading = (Direction)((ant->heading + 3) & 3);
        ant->cells[idx] = 0;
    }

    int dx, dy;
    heading_vector(ant->heading, &dx, &dy);

    /* Move forward, wrapping around the edges so the ant stays on the field. */
    ant->x = (ant->x + dx + ant->width) % ant->width;
    ant->y = (ant->y + dy + ant->height) % ant->height;
}

/*
 * Run the ant for `steps` iterations and return the number of black cells
 * remaining on the board afterwards. Useful as a checksum of the simulation
 * state. O(steps + w*h).
 */
int ant_run(Ant *ant, int steps) {
    for (int i = 0; i < steps; i++)
        ant_step(ant);

    int black = 0;
    for (int i = 0; i < ant->width * ant->height; i++)
        black += ant->cells[i];
    return black;
}
