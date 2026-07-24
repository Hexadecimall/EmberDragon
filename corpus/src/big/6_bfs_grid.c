/*
 * bfs_grid.c -- Breadth-first shortest-path search on a 4-connected grid.
 *
 * Treats a rectangular grid of passable/blocked cells as an unweighted graph
 * and finds the fewest-steps path from a start cell to a goal cell. Because BFS
 * explores in waves of increasing distance, the first time it reaches the goal
 * it has found a shortest path. The path is reconstructed via a parent map.
 */

#include <stdlib.h>
#include <string.h>

/*
 * A GridGraph stores the obstacle layout: 0 means passable, non-zero means
 * blocked. Cells are addressed row-major as blocked[y * width + x].
 */
typedef struct {
    int                  width;
    int                  height;
    const unsigned char *blocked; /* width*height; borrowed, not owned */
} GridGraph;

/* Sentinel stored in the parent array for cells not yet discovered. */
#define BFS_UNVISITED (-1)

/*
 * Return 1 if (x, y) is inside the grid and passable, else 0. Combining the
 * bounds and obstacle checks keeps the neighbour loop concise. O(1).
 */
static int is_passable(const GridGraph *g, int x, int y) {
    if (x < 0 || y < 0 || x >= g->width || y >= g->height)
        return 0;
    return g->blocked[y * g->width + x] == 0;
}

/*
 * Compute a shortest path from (start_x, start_y) to (goal_x, goal_y) using BFS.
 *
 * On success the path (including both endpoints) is written into *out_path as a
 * newly allocated array of cell indices, *out_len is set to its length, and the
 * function returns the path length. The caller must free(*out_path).
 *
 * Returns 0 and leaves *out_path NULL if either endpoint is blocked or the goal
 * is unreachable. Runs in O(w*h) time and memory.
 */
int bfs_shortest_path(const GridGraph *g,
                      int start_x, int start_y,
                      int goal_x, int goal_y,
                      int **out_path, int *out_len) {
    *out_path = NULL;
    *out_len = 0;

    int n = g->width * g->height;
    if (!is_passable(g, start_x, start_y) || !is_passable(g, goal_x, goal_y))
        return 0;

    /* parent[c] is the cell we arrived from, used to walk the path backwards.
     * A FIFO queue of cell indices drives the wavefront expansion. */
    int *parent = (int *)malloc(n * sizeof(int));
    int *queue = (int *)malloc(n * sizeof(int));
    if (!parent || !queue) {
        free(parent);
        free(queue);
        return 0;
    }
    for (int i = 0; i < n; i++)
        parent[i] = BFS_UNVISITED;

    int start = start_y * g->width + start_x;
    int goal = goal_y * g->width + goal_x;
    int head = 0, tail = 0; /* queue is [head, tail) */

    queue[tail++] = start;
    parent[start] = start; /* the start is its own parent: marks it visited */

    /* The four axis-aligned moves, expanded in a fixed order. */
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    int found = 0;
    while (head < tail) {
        int cur = queue[head++];
        if (cur == goal) {
            found = 1;
            break; /* first arrival is a shortest arrival under BFS */
        }
        int cx = cur % g->width;
        int cy = cur / g->width;
        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (!is_passable(g, nx, ny))
                continue;
            int next = ny * g->width + nx;
            if (parent[next] != BFS_UNVISITED)
                continue; /* already discovered on an equal-or-shorter wave */
            parent[next] = cur;
            queue[tail++] = next;
        }
    }

    if (!found) {
        free(parent);
        free(queue);
        return 0;
    }

    /* Count the path length by following parents from goal to start. */
    int length = 1;
    for (int c = goal; c != start; c = parent[c])
        length++;

    int *path = (int *)malloc(length * sizeof(int));
    if (!path) {
        free(parent);
        free(queue);
        return 0;
    }
    /* Fill the path back-to-front so the caller receives start -> goal order. */
    int idx = length - 1;
    for (int c = goal; ; c = parent[c]) {
        path[idx--] = c;
        if (c == start)
            break;
    }

    free(parent);
    free(queue);
    *out_path = path;
    *out_len = length;
    return length;
}
