/*
 * Dijkstra-style cost-field expansion over a weighted integer grid.
 *
 * Every cell has an integer movement cost (>= 1, or a sentinel meaning
 * impassable). Starting from one or more source cells, this module computes
 * the minimum accumulated cost to reach every other cell, producing a
 * "distance field" of the kind used for flow-field pathfinding. The frontier
 * is managed with a binary min-heap keyed on accumulated cost.
 */

#include <stdint.h>

#define COST_MAX_CELLS 4096
#define COST_INFINITY  0x7FFFFFFF  /* unreachable / not-yet-settled marker */
#define COST_WALL      0           /* a cell cost of 0 means impassable */

/*
 * A weighted grid. 'cost[i]' is the cost to ENTER cell i; a value of COST_WALL
 * marks the cell as impassable. Index of (x, y) is y * width + x.
 */
typedef struct {
    int     width;
    int     height;
    int32_t cost[COST_MAX_CELLS];
} CostGrid;

/* One entry in the min-heap: a cell index keyed by its tentative distance. */
typedef struct {
    int32_t dist;
    int32_t cell;
} HeapNode;

/*
 * A binary min-heap of HeapNodes ordered by 'dist'. 'size' is the current
 * element count; 'nodes' is a caller-or-stack-provided backing array.
 */
typedef struct {
    HeapNode *nodes;
    int       size;
    int       capacity;
} MinHeap;

/*
 * Initialize a grid where every cell has uniform entry cost. Returns 1 on
 * success, or 0 if dimensions are invalid or exceed COST_MAX_CELLS.
 */
int costgrid_init(CostGrid *grid, int width, int height, int32_t uniform_cost) {
    if (width <= 0 || height <= 0 || width * height > COST_MAX_CELLS) {
        return 0;
    }
    grid->width = width;
    grid->height = height;
    int n = width * height;
    for (int i = 0; i < n; ++i) {
        grid->cost[i] = uniform_cost;
    }
    return 1;
}

/*
 * Set the entry cost of a single cell. Pass COST_WALL (0) to make it
 * impassable. Out-of-bounds coordinates are ignored. O(1).
 */
void costgrid_set(CostGrid *grid, int x, int y, int32_t cost) {
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return;
    }
    grid->cost[y * grid->width + x] = cost;
}

/*
 * Restore the heap property by sifting the element at 'i' upward toward the
 * root while it is cheaper than its parent. Used after an insertion. O(log n).
 */
static void heap_sift_up(MinHeap *heap, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap->nodes[parent].dist <= heap->nodes[i].dist) {
            break;  /* parent is already no greater: heap order holds */
        }
        HeapNode tmp = heap->nodes[parent];
        heap->nodes[parent] = heap->nodes[i];
        heap->nodes[i] = tmp;
        i = parent;
    }
}

/*
 * Restore the heap property by sifting the root downward toward the smaller of
 * its children until both children are larger. Used after removing the min.
 * O(log n).
 */
static void heap_sift_down(MinHeap *heap, int i) {
    for (;;) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < heap->size &&
            heap->nodes[left].dist < heap->nodes[smallest].dist) {
            smallest = left;
        }
        if (right < heap->size &&
            heap->nodes[right].dist < heap->nodes[smallest].dist) {
            smallest = right;
        }
        if (smallest == i) {
            break;  /* both children are >= current: done */
        }
        HeapNode tmp = heap->nodes[smallest];
        heap->nodes[smallest] = heap->nodes[i];
        heap->nodes[i] = tmp;
        i = smallest;
    }
}

/*
 * Push a (dist, cell) pair onto the heap. Silently drops the entry if the heap
 * is at capacity (which cannot happen when sized to the grid's cell count, as
 * each cell may be enqueued multiple times but capacity is set generously).
 * O(log n).
 */
static void heap_push(MinHeap *heap, int32_t dist, int32_t cell) {
    if (heap->size >= heap->capacity) {
        return;
    }
    heap->nodes[heap->size].dist = dist;
    heap->nodes[heap->size].cell = cell;
    heap->size++;
    heap_sift_up(heap, heap->size - 1);
}

/*
 * Remove and return the minimum-distance node. The caller must ensure the heap
 * is non-empty. O(log n).
 */
static HeapNode heap_pop(MinHeap *heap) {
    HeapNode top = heap->nodes[0];
    heap->size--;
    heap->nodes[0] = heap->nodes[heap->size];  /* move last element to root */
    heap_sift_down(heap, 0);
    return top;
}

/*
 * Build a distance field: for every cell, the minimum total entry cost to
 * reach it from the single source (src_x, src_y).
 *
 * Results are written to 'out_dist' (width*height ints, caller-provided):
 * COST_INFINITY for unreachable cells and walls, otherwise the cheapest cost.
 * The source cell's distance is 0 even though it has an entry cost, because we
 * never "enter" the start.
 *
 * Returns the number of cells that turned out to be reachable (including the
 * source), or -1 if the source is out of bounds or itself a wall.
 *
 * Each cell is settled at most once; with a binary heap this is the classic
 * O(E log V) Dijkstra, here O(width * height * log(width * height)).
 */
int costgrid_distance_field(const CostGrid *grid, int src_x, int src_y,
                            int32_t *out_dist) {
    if (src_x < 0 || src_x >= grid->width ||
        src_y < 0 || src_y >= grid->height) {
        return -1;
    }
    int src = src_y * grid->width + src_x;
    if (grid->cost[src] == COST_WALL) {
        return -1;  /* cannot start inside a wall */
    }

    int n = grid->width * grid->height;
    for (int i = 0; i < n; ++i) {
        out_dist[i] = COST_INFINITY;  /* every cell starts unreached */
    }

    /* The heap may hold stale duplicates, so give it room for a few entries
       per cell. 4x the cell count is comfortably sufficient in practice. */
    HeapNode storage[COST_MAX_CELLS * 4];
    MinHeap heap = { storage, 0, COST_MAX_CELLS * 4 };

    out_dist[src] = 0;
    heap_push(&heap, 0, src);

    static const int DX[4] = { 1, -1, 0, 0 };
    static const int DY[4] = { 0, 0, 1, -1 };

    int settled = 0;
    while (heap.size > 0) {
        HeapNode node = heap_pop(&heap);
        int cur = node.cell;

        /* Skip stale heap entries: if we already found a cheaper route to this
           cell, the popped distance is out of date and can be discarded. */
        if (node.dist > out_dist[cur]) {
            continue;
        }
        settled++;

        int cx = cur % grid->width;
        int cy = cur / grid->width;
        for (int d = 0; d < 4; ++d) {
            int nx = cx + DX[d];
            int ny = cy + DY[d];
            if (nx < 0 || nx >= grid->width || ny < 0 || ny >= grid->height) {
                continue;
            }
            int ni = ny * grid->width + nx;
            int32_t enter = grid->cost[ni];
            if (enter == COST_WALL) {
                continue;  /* impassable neighbor */
            }
            int32_t candidate = out_dist[cur] + enter;
            /* Relax the edge: record and enqueue only strict improvements. */
            if (candidate < out_dist[ni]) {
                out_dist[ni] = candidate;
                heap_push(&heap, candidate, ni);
            }
        }
    }

    return settled;
}
