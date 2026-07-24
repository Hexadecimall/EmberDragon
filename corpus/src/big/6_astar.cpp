/*
 * astar.cpp -- A* shortest-path search on a 4-connected integer grid.
 *
 * A* augments Dijkstra's algorithm with an admissible heuristic (here the
 * Manhattan distance) to steer exploration toward the goal, finding an optimal
 * path while visiting far fewer cells. This implementation uses a hand-written
 * binary min-heap keyed on the estimated total cost f = g + h, so it pulls no
 * STL containers and stays in pure integer arithmetic.
 */

#include <cstdint>
#include <cstdlib>

/*
 * Grid describes a uniform-cost map. A cell is passable when its byte is zero.
 * Cells are addressed row-major as cells[y * width + x].
 */
struct Grid {
    int                  width;
    int                  height;
    const unsigned char *cells; /* borrowed obstacle map, not owned */
};

/* A node waiting in the open set, ordered by f within the heap. */
struct OpenNode {
    int index; /* flat cell index */
    int f;     /* g (steps so far) + h (heuristic to goal) */
};

/*
 * Manhattan distance between two cells. This is admissible on a 4-connected
 * grid with unit step costs (it never overestimates), which is what guarantees
 * A* returns an optimal path. O(1).
 */
static int manhattan(int ax, int ay, int bx, int by) {
    int dx = ax - bx;
    int dy = ay - by;
    if (dx < 0) dx = -dx; /* integer abs, no <cmath> needed */
    if (dy < 0) dy = -dy;
    return dx + dy;
}

/*
 * Restore the min-heap property by sifting the element at `pos` upward toward
 * the root until its parent has a smaller-or-equal f. Used after a push. O(log n).
 */
static void heap_sift_up(OpenNode *heap, int pos) {
    while (pos > 0) {
        int parent = (pos - 1) / 2;
        if (heap[parent].f <= heap[pos].f)
            break; /* parent already dominates: heap is valid above here */
        OpenNode tmp = heap[parent];
        heap[parent] = heap[pos];
        heap[pos] = tmp;
        pos = parent;
    }
}

/*
 * Sift the root downward into its correct place after the minimum is removed.
 * Always swaps with the smaller child so the new root settles correctly.
 * O(log n).
 */
static void heap_sift_down(OpenNode *heap, int size, int pos) {
    for (;;) {
        int left = 2 * pos + 1;
        int right = 2 * pos + 2;
        int smallest = pos;
        if (left < size && heap[left].f < heap[smallest].f)
            smallest = left;
        if (right < size && heap[right].f < heap[smallest].f)
            smallest = right;
        if (smallest == pos)
            break; /* both children dominate: position is correct */
        OpenNode tmp = heap[smallest];
        heap[smallest] = heap[pos];
        heap[pos] = tmp;
        pos = smallest;
    }
}

/*
 * Find an optimal path from (sx, sy) to (gx, gy) with A*.
 *
 * On success writes a newly allocated array of cell indices (start..goal) to
 * *out_path, stores its length in *out_len, and returns that length. The caller
 * must free(*out_path). Returns 0 with *out_path == nullptr when an endpoint is
 * blocked or no path exists. Time O(N log N), memory O(N), N = width*height.
 */
int astar_search(const Grid &grid, int sx, int sy, int gx, int gy,
                 int **out_path, int *out_len) {
    *out_path = nullptr;
    *out_len = 0;

    int n = grid.width * grid.height;
    auto passable = [&](int x, int y) -> bool {
        if (x < 0 || y < 0 || x >= grid.width || y >= grid.height)
            return false;
        return grid.cells[y * grid.width + x] == 0;
    };
    if (!passable(sx, sy) || !passable(gx, gy))
        return 0;

    /* g_score[c] = cheapest known step count from the start to c.
     * came_from[c] = predecessor on that cheapest path. -1 = unknown. */
    int *g_score = (int *)malloc(n * sizeof(int));
    int *came_from = (int *)malloc(n * sizeof(int));
    OpenNode *heap = (OpenNode *)malloc(n * sizeof(OpenNode));
    if (!g_score || !came_from || !heap) {
        free(g_score); free(came_from); free(heap);
        return 0;
    }
    for (int i = 0; i < n; i++) {
        g_score[i] = -1; /* -1 doubles as "infinite / never reached" */
        came_from[i] = -1;
    }

    int start = sy * grid.width + sx;
    int goal = gy * grid.width + gx;
    g_score[start] = 0;

    int heap_size = 0;
    heap[heap_size++] = {start, manhattan(sx, sy, gx, gy)};

    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    bool found = false;
    while (heap_size > 0) {
        OpenNode top = heap[0];
        heap[0] = heap[--heap_size]; /* move last node to root, then re-sink */
        heap_sift_down(heap, heap_size, 0);

        if (top.index == goal) {
            found = true;
            break;
        }

        int cx = top.index % grid.width;
        int cy = top.index / grid.width;
        int tentative = g_score[top.index] + 1; /* each step costs exactly 1 */

        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (!passable(nx, ny))
                continue;
            int next = ny * grid.width + nx;
            /* Relax the neighbour only if we found a strictly cheaper route.
             * The -1 sentinel makes a first visit always qualify. */
            if (g_score[next] != -1 && g_score[next] <= tentative)
                continue;
            g_score[next] = tentative;
            came_from[next] = top.index;
            int f = tentative + manhattan(nx, ny, gx, gy);
            heap[heap_size] = {next, f};
            heap_sift_up(heap, heap_size);
            heap_size++;
        }
    }

    if (!found) {
        free(g_score); free(came_from); free(heap);
        return 0;
    }

    int length = g_score[goal] + 1; /* steps plus the start cell itself */
    int *path = (int *)malloc(length * sizeof(int));
    if (!path) {
        free(g_score); free(came_from); free(heap);
        return 0;
    }
    int idx = length - 1;
    for (int c = goal; c != -1; c = came_from[c])
        path[idx--] = c; /* unwind predecessors into start..goal order */

    free(g_score); free(came_from); free(heap);
    *out_path = path;
    *out_len = length;
    return length;
}
