/*
 * Breadth-first shortest-path search on a 4-connected integer grid.
 *
 * Cells are either walkable or blocked. Given a start and goal, this module
 * finds the minimum number of steps between them using a ring-buffer queue,
 * and can reconstruct the actual path by walking parent pointers back from
 * the goal. BFS guarantees the shortest path on an unweighted grid.
 */

#include <stdint.h>
#include <string.h>

#define GRID_MAX_CELLS 4096  /* upper bound on width * height we support */

/*
 * A navigation grid. 'blocked[i]' is non-zero where cell index i is an
 * obstacle. Cell index for (x, y) is y * width + x.
 */
typedef struct {
    int     width;
    int     height;
    uint8_t blocked[GRID_MAX_CELLS];
} NavGrid;

/* A grid coordinate, returned when reconstructing a path. */
typedef struct {
    int x;
    int y;
} Point;

/*
 * Initialize an open (fully walkable) grid of the given size.
 * Returns 1 on success, or 0 if the dimensions are non-positive or the cell
 * count exceeds GRID_MAX_CELLS.
 */
int navgrid_init(NavGrid *grid, int width, int height) {
    if (width <= 0 || height <= 0) {
        return 0;
    }
    if (width * height > GRID_MAX_CELLS) {
        return 0;  /* would not fit in the fixed obstacle array */
    }
    grid->width = width;
    grid->height = height;
    memset(grid->blocked, 0, sizeof(grid->blocked));
    return 1;
}

/*
 * Mark a cell as an obstacle (blocked = 1) or clear it (blocked = 0).
 * Out-of-bounds coordinates are silently ignored. O(1).
 */
void navgrid_block(NavGrid *grid, int x, int y, int blocked) {
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return;
    }
    grid->blocked[y * grid->width + x] = blocked ? 1 : 0;
}

/*
 * Test whether (x, y) is inside the grid and walkable.
 * Returns 1 if the cell can be entered, 0 otherwise (out of bounds or blocked).
 */
static int navgrid_walkable(const NavGrid *grid, int x, int y) {
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return 0;
    }
    return grid->blocked[y * grid->width + x] == 0;
}

/*
 * Compute the shortest 4-connected path length from (sx, sy) to (gx, gy).
 *
 * Internally runs BFS from the start, recording each cell's predecessor in
 * 'parent' (indexed by cell). If 'out_path' is non-NULL and 'max_path' > 0,
 * the path from start to goal (inclusive) is written into it, start first.
 *
 * Returns:
 *   the number of steps (edges) on the shortest path, or
 *   0 if start == goal, or
 *   -1 if no path exists or either endpoint is itself unwalkable.
 *
 * Note: if the true path is longer than 'max_path', the step count is still
 * returned correctly but only the first 'max_path' points are written.
 * Runs in O(width * height) time.
 */
int navgrid_shortest_path(const NavGrid *grid,
                          int sx, int sy, int gx, int gy,
                          Point *out_path, int max_path) {
    if (!navgrid_walkable(grid, sx, sy) || !navgrid_walkable(grid, gx, gy)) {
        return -1;  /* an endpoint sits on an obstacle or off the board */
    }

    int cells = grid->width * grid->height;
    int start = sy * grid->width + sx;
    int goal  = gy * grid->width + gx;

    if (start == goal) {
        if (out_path && max_path > 0) {
            out_path[0].x = sx;
            out_path[0].y = sy;
        }
        return 0;
    }

    /* parent[i] = predecessor cell index of i on the BFS tree, or -1 if i has
       not yet been discovered. This doubles as the "visited" marker. */
    int parent[GRID_MAX_CELLS];
    for (int i = 0; i < cells; ++i) {
        parent[i] = -1;
    }

    /* Ring-buffer queue of cell indices. A grid BFS enqueues each cell at most
       once, so 'cells' slots are always sufficient. */
    int queue[GRID_MAX_CELLS];
    int head = 0, tail = 0;
    queue[tail++] = start;
    parent[start] = start;  /* the start is its own parent (a sentinel root) */

    /* The four orthogonal moves: right, left, down, up. */
    static const int DX[4] = { 1, -1, 0, 0 };
    static const int DY[4] = { 0, 0, 1, -1 };

    int found = 0;
    while (head < tail && !found) {
        int cur = queue[head++];
        int cx = cur % grid->width;
        int cy = cur / grid->width;

        for (int d = 0; d < 4; ++d) {
            int nx = cx + DX[d];
            int ny = cy + DY[d];
            if (!navgrid_walkable(grid, nx, ny)) {
                continue;
            }
            int ni = ny * grid->width + nx;
            if (parent[ni] != -1) {
                continue;  /* already discovered; BFS keeps the shorter route */
            }
            parent[ni] = cur;
            if (ni == goal) {
                found = 1;  /* stop as soon as the goal is reached */
                break;
            }
            queue[tail++] = ni;
        }
    }

    if (!found) {
        return -1;  /* goal unreachable from start */
    }

    /* Reconstruct length by walking parent pointers back to the start. */
    int length = 0;
    for (int c = goal; c != start; c = parent[c]) {
        length++;
    }

    /* Optionally emit the path in forward (start -> goal) order. We first
       collect it backward, then reverse into the caller's buffer. */
    if (out_path && max_path > 0) {
        int tmp[GRID_MAX_CELLS];
        int n = 0;
        for (int c = goal; ; c = parent[c]) {
            tmp[n++] = c;
            if (c == start) break;
        }
        /* tmp now holds goal..start; copy reversed and clipped to max_path. */
        int count = n < max_path ? n : max_path;
        for (int i = 0; i < count; ++i) {
            int c = tmp[n - 1 - i];
            out_path[i].x = c % grid->width;
            out_path[i].y = c / grid->width;
        }
    }

    return length;
}
