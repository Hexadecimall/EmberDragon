/*
 * grid_circle.c — midpoint circle rasterization on an integer grid.
 *
 * Draws circles and filled disks onto a grid using the midpoint (Bresenham)
 * circle algorithm, which plots one octant with integer-only error tracking and
 * mirrors it into the other seven by symmetry. Also provides an exact integer
 * point-in-disk test based on comparing squared distances, never a square root.
 */

#include <stdint.h>

/* An integer grid cell produced by the rasterizer. */
typedef struct {
    int32_t x;
    int32_t y;
} Cell;

/*
 * Emit a cell to the output buffer if there is room. Centralizing the bounds
 * check keeps the eight symmetric plots below readable.
 * Returns 1 if the cell was written, 0 if the buffer was full.
 */
static int emit(Cell *out, int *count, int maxOut, int32_t x, int32_t y) {
    if (*count >= maxOut) return 0;
    out[*count].x = x;
    out[*count].y = y;
    (*count)++;
    return 1;
}

/*
 * Rasterize the outline of a circle centered at (cx, cy) with the given radius.
 *
 * The midpoint algorithm walks the second octant (from the top toward the
 * 45-degree diagonal). At each step the decision variable `d` decides whether
 * to move straight in or also step inward; both updates are integer additions.
 * Each plotted (x, y) is reflected to all eight octants.
 *
 * Parameters:
 *   cx, cy — center cell; radius — circle radius in cells (radius 0 plots the
 *            center only); out/maxOut — output buffer and its capacity.
 * Returns the number of cells written. Cells may repeat at the octant seams
 * (e.g. the axis points), which callers that need a unique set should dedupe.
 * Complexity: O(radius).
 */
int rasterizeCircle(int32_t cx, int32_t cy, int32_t radius,
                    Cell *out, int maxOut) {
    int count = 0;
    if (radius < 0) return 0;

    int32_t x = radius;
    int32_t y = 0;
    int32_t d = 1 - radius; /* initial midpoint decision value */

    while (x >= y) {
        /* Plot the eight mirror images of (x, y) around the center. */
        emit(out, &count, maxOut, cx + x, cy + y);
        emit(out, &count, maxOut, cx - x, cy + y);
        emit(out, &count, maxOut, cx + x, cy - y);
        emit(out, &count, maxOut, cx - x, cy - y);
        emit(out, &count, maxOut, cx + y, cy + x);
        emit(out, &count, maxOut, cx - y, cy + x);
        emit(out, &count, maxOut, cx + y, cy - x);
        emit(out, &count, maxOut, cx - y, cy - x);

        y++;
        if (d < 0) {
            /* Midpoint stayed inside the circle: only the y step is needed. */
            d += 2 * y + 1;
        } else {
            /* Midpoint fell outside: step inward in x as well. */
            x--;
            d += 2 * (y - x) + 1;
        }
    }
    return count;
}

/*
 * Exact point-in-disk test: is the cell (px, py) within `radius` of the center?
 * Compares the squared distance against the squared radius so the whole test is
 * integer arithmetic. Uses '<=' to include the boundary ring.
 * Returns 1 if the cell lies in the closed disk, 0 otherwise.
 */
int cellInDisk(int32_t cx, int32_t cy, int32_t radius,
               int32_t px, int32_t py) {
    int64_t dx = px - cx;
    int64_t dy = py - cy;
    return dx * dx + dy * dy <= (int64_t)radius * radius;
}

/*
 * Rasterize a filled disk by scanning each row and, for each row, walking the
 * columns within the row's half-width. The half-width per row is found by
 * incrementing until the squared distance would exceed the squared radius — no
 * square root, just an integer comparison loop.
 *
 * Parameters mirror rasterizeCircle. Returns the number of cells written, or
 * stops early (returning what fit) if the buffer fills. Complexity: O(radius^2),
 * proportional to the disk's area.
 */
int rasterizeDisk(int32_t cx, int32_t cy, int32_t radius,
                  Cell *out, int maxOut) {
    int count = 0;
    if (radius < 0) return 0;
    int64_t r2 = (int64_t)radius * radius;

    for (int32_t dy = -radius; dy <= radius; dy++) {
        /* Find the widest dx whose squared distance still fits in the disk. */
        int32_t halfWidth = 0;
        while ((int64_t)(halfWidth + 1) * (halfWidth + 1) + (int64_t)dy * dy <= r2) {
            halfWidth++;
        }
        for (int32_t dx = -halfWidth; dx <= halfWidth; dx++) {
            if (!emit(out, &count, maxOut, cx + dx, cy + dy)) {
                return count; /* buffer exhausted */
            }
        }
    }
    return count;
}
