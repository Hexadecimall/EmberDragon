/*
 * dijkstra_weighted.cpp -- Dijkstra shortest path on a weighted terrain grid.
 *
 * Each cell carries an integer movement cost (its "terrain"); stepping onto a
 * cell pays that cost. Dijkstra's algorithm finds the minimum-cost route from a
 * source to a target over the 4-connected grid using a binary min-heap of
 * (distance, cell) pairs. All arithmetic is integer; impassable cells are marked
 * with a sentinel cost and are simply never expanded.
 */

#include <cstdint>
#include <cstdlib>

/* Terrain cost meaning a cell can never be entered. */
#define TERRAIN_WALL  0
/* Distance sentinel standing in for "infinity / not yet reached". */
#define DIST_INFINITE INT32_MAX

/*
 * TerrainGrid maps each cell to a positive entry cost, or TERRAIN_WALL (0) for
 * an obstacle. Costs are addressed row-major as cost[y * width + x].
 */
struct TerrainGrid {
    int            width;
    int            height;
    const int32_t *cost; /* borrowed per-cell entry costs, not owned */
};

/* One entry in the priority queue: the tentative distance to `cell`. */
struct HeapEntry {
    int32_t dist;
    int     cell;
};

/*
 * Move the entry at `pos` up the heap until the min-heap order on dist holds.
 * Called after appending a new entry. O(log n).
 */
static void sift_up(HeapEntry *heap, int pos) {
    while (pos > 0) {
        int parent = (pos - 1) / 2;
        if (heap[parent].dist <= heap[pos].dist)
            break; /* parent is no larger: ordering restored */
        HeapEntry tmp = heap[parent];
        heap[parent] = heap[pos];
        heap[pos] = tmp;
        pos = parent;
    }
}

/*
 * Push the root down to its proper position after the minimum is popped, always
 * descending toward the smaller child. O(log n).
 */
static void sift_down(HeapEntry *heap, int size, int pos) {
    for (;;) {
        int left = 2 * pos + 1;
        int right = 2 * pos + 2;
        int smallest = pos;
        if (left < size && heap[left].dist < heap[smallest].dist)
            smallest = left;
        if (right < size && heap[right].dist < heap[smallest].dist)
            smallest = right;
        if (smallest == pos)
            break;
        HeapEntry tmp = heap[smallest];
        heap[smallest] = heap[pos];
        heap[pos] = tmp;
        pos = smallest;
    }
}

/*
 * Compute the minimum total terrain cost to travel from (sx, sy) to (gx, gy).
 *
 * The returned cost is the sum of the entry costs of every cell on the path
 * except the source (you do not pay to stand on the start). Returns -1 if either
 * endpoint is a wall or the target is unreachable. Time O(N log N), memory O(N),
 * with N = width*height.
 */
int dijkstra_min_cost(const TerrainGrid &grid, int sx, int sy, int gx, int gy) {
    int n = grid.width * grid.height;

    auto cost_of = [&](int x, int y) -> int32_t {
        return grid.cost[y * grid.width + x];
    };
    auto in_bounds = [&](int x, int y) -> bool {
        return x >= 0 && y >= 0 && x < grid.width && y < grid.height;
    };

    /* A path that starts or ends inside a wall is meaningless. */
    if (cost_of(sx, sy) == TERRAIN_WALL || cost_of(gx, gy) == TERRAIN_WALL)
        return -1;

    int32_t *dist = (int32_t *)malloc(n * sizeof(int32_t));
    HeapEntry *heap = (HeapEntry *)malloc(n * sizeof(HeapEntry));
    if (!dist || !heap) {
        free(dist);
        free(heap);
        return -1;
    }
    for (int i = 0; i < n; i++)
        dist[i] = DIST_INFINITE;

    int source = sy * grid.width + sx;
    int target = gy * grid.width + gx;
    dist[source] = 0; /* no cost to begin at the source */

    int heap_size = 0;
    heap[heap_size++] = {0, source};

    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    while (heap_size > 0) {
        HeapEntry top = heap[0];
        heap[0] = heap[--heap_size];
        sift_down(heap, heap_size, 0);

        /* Lazy deletion: an outdated, larger entry for this cell can linger in
         * the heap. Skip it once a better distance has already been settled. */
        if (top.dist > dist[top.cell])
            continue;
        if (top.cell == target)
            break; /* target popped with its final distance: we are done */

        int cx = top.cell % grid.width;
        int cy = top.cell / grid.width;
        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (!in_bounds(nx, ny))
                continue;
            int32_t step = cost_of(nx, ny);
            if (step == TERRAIN_WALL)
                continue; /* cannot enter an obstacle */

            int next = ny * grid.width + nx;
            int32_t through = top.dist + step;
            if (through < dist[next]) {
                /* Relaxation succeeded: record the cheaper route and queue it. */
                dist[next] = through;
                heap[heap_size] = {through, next};
                sift_up(heap, heap_size);
                heap_size++;
            }
        }
    }

    int result = (dist[target] == DIST_INFINITE) ? -1 : dist[target];
    free(dist);
    free(heap);
    return result;
}
