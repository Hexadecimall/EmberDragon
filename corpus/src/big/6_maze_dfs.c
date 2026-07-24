/*
 * maze_dfs.c -- Perfect-maze generation via randomised depth-first search.
 *
 * The grid is a lattice of cells, each carrying a bitmask of which of its four
 * walls are still standing. A recursive backtracker carves passages by walking
 * to an unvisited neighbour and knocking out the wall between them, producing a
 * "perfect" maze: exactly one path between any two cells, with no loops.
 */

#include <stdint.h>
#include <stdlib.h>

/* Wall bits. Each cell tracks four independent walls as flags so the shared
 * wall between two neighbours can be cleared from both sides. */
#define WALL_NORTH 0x1
#define WALL_EAST  0x2
#define WALL_SOUTH 0x4
#define WALL_WEST  0x8
#define WALL_ALL   (WALL_NORTH | WALL_EAST | WALL_SOUTH | WALL_WEST)

/*
 * A Maze is a row-major grid of cells. Each cell starts fully walled; carving
 * removes wall bits. `visited` mirrors the cell layout and tracks generation
 * progress.
 */
typedef struct {
    int      width;
    int      height;
    uint8_t *walls;   /* width*height wall masks, owned by the Maze   */
    uint8_t *visited; /* width*height flags used during generation    */
} Maze;

/*
 * Allocate a fully walled-off maze of the given size. Returns NULL on bad
 * dimensions or allocation failure; the caller frees it with maze_destroy.
 * O(w*h).
 */
Maze *maze_create(int width, int height) {
    if (width <= 0 || height <= 0)
        return NULL;
    Maze *m = (Maze *)malloc(sizeof(Maze));
    if (!m)
        return NULL;
    m->width = width;
    m->height = height;
    size_t cells = (size_t)width * height;
    m->walls = (uint8_t *)malloc(cells);
    m->visited = (uint8_t *)malloc(cells);
    if (!m->walls || !m->visited) {
        free(m->walls);   /* free(NULL) is safe, so unconditional cleanup is OK */
        free(m->visited);
        free(m);
        return NULL;
    }
    for (size_t i = 0; i < cells; i++) {
        m->walls[i] = WALL_ALL; /* every wall up before carving begins */
        m->visited[i] = 0;
    }
    return m;
}

/*
 * Release a maze and its buffers. Safe to call with NULL. O(1).
 */
void maze_destroy(Maze *m) {
    if (!m)
        return;
    free(m->walls);
    free(m->visited);
    free(m);
}

/*
 * Remove the wall between adjacent cells (x, y) and the neighbour reached by
 * moving in direction `dir`. The opposite wall is cleared on the neighbour so
 * the passage is bidirectional. Assumes the neighbour is in-bounds. O(1).
 */
static void carve_passage(Maze *m, int x, int y, int dir) {
    int here = y * m->width + x;
    if (dir == WALL_NORTH) {
        m->walls[here] &= ~WALL_NORTH;
        m->walls[(y - 1) * m->width + x] &= ~WALL_SOUTH;
    } else if (dir == WALL_EAST) {
        m->walls[here] &= ~WALL_EAST;
        m->walls[y * m->width + (x + 1)] &= ~WALL_WEST;
    } else if (dir == WALL_SOUTH) {
        m->walls[here] &= ~WALL_SOUTH;
        m->walls[(y + 1) * m->width + x] &= ~WALL_NORTH;
    } else { /* WALL_WEST */
        m->walls[here] &= ~WALL_WEST;
        m->walls[y * m->width + (x - 1)] &= ~WALL_EAST;
    }
}

/*
 * Recursive backtracker. Marks (x, y) visited, then repeatedly picks a random
 * unvisited neighbour, carves toward it, and recurses. When no unvisited
 * neighbour remains the call returns, letting the caller back up the stack.
 * Visits every cell exactly once: O(w*h).
 */
static void carve_from(Maze *m, int x, int y) {
    m->visited[y * m->width + x] = 1;

    /* Try the four directions in a shuffled order so the maze is irregular. */
    int dirs[4] = {WALL_NORTH, WALL_EAST, WALL_SOUTH, WALL_WEST};
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        int swap = dirs[i];
        dirs[i] = dirs[j];
        dirs[j] = swap;
    }

    for (int i = 0; i < 4; i++) {
        int nx = x, ny = y;
        if (dirs[i] == WALL_NORTH) ny--;
        else if (dirs[i] == WALL_EAST) nx++;
        else if (dirs[i] == WALL_SOUTH) ny++;
        else nx--;

        /* Skip neighbours off the grid or already part of the maze. */
        if (nx < 0 || ny < 0 || nx >= m->width || ny >= m->height)
            continue;
        if (m->visited[ny * m->width + nx])
            continue;

        carve_passage(m, x, y, dirs[i]);
        carve_from(m, nx, ny); /* depth-first dive into the new cell */
    }
}

/*
 * Generate a fresh perfect maze starting from the top-left cell. Resets the
 * visited markers first so the same Maze can be regenerated. Seed rand() before
 * calling for a different layout each run. O(w*h).
 */
void maze_generate(Maze *m) {
    for (int i = 0; i < m->width * m->height; i++)
        m->visited[i] = 0;
    carve_from(m, 0, 0);
}

/*
 * Test whether two horizontally or vertically adjacent cells are connected
 * (the wall between them has been carved away). Returns 1 if you can walk
 * directly from (x, y) in direction `dir`, else 0. O(1).
 */
int maze_is_open(const Maze *m, int x, int y, int dir) {
    return (m->walls[y * m->width + x] & dir) == 0;
}
