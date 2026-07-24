/*
 * bresenham_line.c — integer line rasterization with Bresenham's algorithm.
 *
 * Rasterizes a straight line between two integer endpoints into the sequence of
 * grid cells it passes through, using Bresenham's classic error-accumulation
 * method. No multiplication or division is needed per step and no floating point
 * is used: the line is traced purely with integer additions and comparisons.
 */

#include <stdint.h>

/* An integer pixel/cell coordinate produced by the rasterizer. */
typedef struct {
    int32_t x;
    int32_t y;
} Pixel;

/*
 * Absolute value for 32-bit integers (local helper, avoids <math.h>).
 * Returns |v|.
 */
static int32_t absInt(int32_t v) {
    return v < 0 ? -v : v;
}

/*
 * Rasterize the line segment from (x0, y0) to (x1, y1) into grid cells.
 *
 * This is the general (all-octants) integer Bresenham line. The error term
 * `err` tracks how far the ideal line has drifted from the current cell; when
 * it crosses a threshold we step along the minor axis. `sx`/`sy` carry the sign
 * of travel so the same loop handles every direction.
 *
 * Parameters:
 *   x0, y0 — start cell (included in the output).
 *   x1, y1 — end cell (included in the output).
 *   out    — caller-supplied buffer; must hold at least
 *            (max(|dx|, |dy|) + 1) pixels.
 *   maxOut — capacity of `out`; the function never writes more than this.
 *
 * Returns the number of cells written. The first cell is always the start and,
 * space permitting, the last is the end. Complexity: O(length of the line).
 */
int rasterizeLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                  Pixel *out, int maxOut) {
    int32_t dx = absInt(x1 - x0);
    int32_t dy = absInt(y1 - y0);
    int32_t sx = x0 < x1 ? 1 : -1;   /* horizontal step direction */
    int32_t sy = y0 < y1 ? 1 : -1;   /* vertical step direction */
    int32_t err = dx - dy;           /* accumulated decision variable */
    int count = 0;

    for (;;) {
        if (count >= maxOut) break;  /* respect the output capacity */
        out[count].x = x0;
        out[count].y = y0;
        count++;

        if (x0 == x1 && y0 == y1) break; /* reached the endpoint */

        /* e2 is twice the error; comparing against -dy and dx (without halving)
         * keeps the whole decision in integers. */
        int32_t e2 = 2 * err;
        if (e2 > -dy) {  /* time to step in x */
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {   /* time to step in y */
            err += dx;
            y0 += sy;
        }
    }
    return count;
}

/*
 * Length of a Bresenham line in cells, i.e. how many cells rasterizeLine would
 * emit, computed without actually walking the line.
 * Returns max(|dx|, |dy|) + 1 (the Chebyshev distance plus the starting cell).
 */
int lineCellCount(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    int32_t dx = absInt(x1 - x0);
    int32_t dy = absInt(y1 - y0);
    return (dx > dy ? dx : dy) + 1;
}

/*
 * Test whether a target cell lies on the rasterized line between the endpoints.
 * Rather than rasterize and scan, we exploit the line's algebra: the point must
 * be collinear (zero cross product) and inside the endpoints' bounding box.
 * This matches the *mathematical* line; a Bresenham trace approximates it, so
 * this is the exact on-line predicate for integer-collinear points.
 * Returns 1 if (px, py) is on the segment, 0 otherwise.
 */
int cellOnLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
               int32_t px, int32_t py) {
    int64_t cross = (int64_t)(x1 - x0) * (py - y0) -
                    (int64_t)(y1 - y0) * (px - x0);
    if (cross != 0) return 0; /* not collinear with the endpoints */

    /* Collinear: confirm it falls between the endpoints on both axes. */
    int32_t minX = x0 < x1 ? x0 : x1, maxX = x0 > x1 ? x0 : x1;
    int32_t minY = y0 < y1 ? y0 : y1, maxY = y0 > y1 ? y0 : y1;
    return px >= minX && px <= maxX && py >= minY && py <= maxY;
}
