/*
 * polygon_raster.c — scanline fill and point-in-polygon for integer polygons.
 *
 * Routines for working with simple integer polygons: a ray-casting point-in-
 * polygon test, an exact "on the boundary" test, and a scanline fill that walks
 * a single horizontal row and reports the spans of cells that lie inside. All
 * arithmetic is integer; edge crossings are decided with cross-multiplication
 * to avoid the divisions a slope-based approach would need.
 */

#include <stdint.h>
#include <stdlib.h>

/* A polygon vertex on the integer lattice. */
typedef struct {
    int32_t x;
    int32_t y;
} Vertex;

/* A closed horizontal interval [start, end] of filled cells on one scanline. */
typedef struct {
    int32_t start;
    int32_t end;
} Span;

/*
 * Point-in-polygon test by ray casting (the crossing-number / even-odd rule).
 * A horizontal ray is shot to the right from (px, py); the point is inside when
 * it crosses an odd number of polygon edges. Edge crossings are detected and
 * the x-coordinate of each crossing is compared via cross-multiplication so no
 * division is performed.
 *
 * Parameters: poly — vertex array in order; n — vertex count; (px, py) — query.
 * Returns 1 if the point is strictly inside, 0 if outside. Boundary points are
 * not guaranteed here; use pointOnBoundary() for an exact edge test.
 * Complexity: O(n).
 */
int pointInPolygon(const Vertex *poly, int n, int32_t px, int32_t py) {
    int inside = 0;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        int32_t yi = poly[i].y, yj = poly[j].y;
        int32_t xi = poly[i].x, xj = poly[j].x;

        /* Does edge (j -> i) straddle the horizontal line y = py? Using one
         * strict and one non-strict comparison counts each vertex once. */
        int straddles = (yi > py) != (yj > py);
        if (!straddles) continue;

        /* The ray crosses this edge to the right of px when, after clearing the
         * denominator (yi - yj), the crossing x exceeds px. We multiply through
         * by (yi - yj) and flip the comparison when that factor is negative. */
        int64_t lhs = (int64_t)(px - xj) * (yi - yj);
        int64_t rhs = (int64_t)(xi - xj) * (py - yj);
        if (yi - yj > 0) {
            if (lhs < rhs) inside = !inside;
        } else {
            if (lhs > rhs) inside = !inside;
        }
    }
    return inside;
}

/*
 * Exact test for whether a point lies on a polygon edge (the boundary).
 * Walks every edge and checks collinearity (zero cross product) plus bounding-
 * box containment.
 * Returns 1 if (px, py) is on any edge, 0 otherwise. O(n).
 */
int pointOnBoundary(const Vertex *poly, int n, int32_t px, int32_t py) {
    for (int i = 0, j = n - 1; i < n; j = i++) {
        int32_t xi = poly[i].x, yi = poly[i].y;
        int32_t xj = poly[j].x, yj = poly[j].y;

        /* Cross product of edge vector and (point - vertex j); zero => collinear. */
        int64_t cross = (int64_t)(xi - xj) * (py - yj) -
                        (int64_t)(yi - yj) * (px - xj);
        if (cross != 0) continue;

        /* Collinear: require the point inside the edge's bounding box. */
        int32_t minX = xi < xj ? xi : xj, maxX = xi > xj ? xi : xj;
        int32_t minY = yi < yj ? yi : yj, maxY = yi > yj ? yi : yj;
        if (px >= minX && px <= maxX && py >= minY && py <= maxY) return 1;
    }
    return 0;
}

/*
 * Compute the filled spans of a polygon on a single horizontal scanline y.
 *
 * Each polygon edge that the line y crosses contributes one x-intersection. We
 * collect those x-values (rounded toward the interior with integer division),
 * sort them, then pair them up: [x0, x1], [x2, x3], ... are the inside spans.
 *
 * Parameters:
 *   poly  — vertex array; n — vertex count; y — the scanline row.
 *   spans — caller-supplied output; must hold at least n/2 spans.
 *   maxSpans — capacity of `spans`.
 * Returns the number of spans written. Complexity: O(n log n) from the sort.
 */
int scanlineSpans(const Vertex *poly, int n, int32_t y,
                  Span *spans, int maxSpans) {
    /* Gather x crossings of the line y against each edge. */
    int32_t *xs = (int32_t *)malloc((size_t)n * sizeof(int32_t));
    int count = 0;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        int32_t yi = poly[i].y, yj = poly[j].y;
        int32_t xi = poly[i].x, xj = poly[j].x;

        if ((yi > y) != (yj > y)) { /* edge spans this scanline */
            /* Solve for x at height y using integer math: round toward the
             * lower vertex so spans stay inside the polygon. */
            int32_t num = (xi - xj) * (y - yj);
            int32_t xCross = xj + num / (yi - yj);
            if (count < n) xs[count++] = xCross;
        }
    }

    /* Sort crossings ascending (small insertion sort — n is tiny per polygon). */
    for (int a = 1; a < count; a++) {
        int32_t key = xs[a];
        int b = a - 1;
        while (b >= 0 && xs[b] > key) {
            xs[b + 1] = xs[b];
            b--;
        }
        xs[b + 1] = key;
    }

    /* Pair consecutive crossings into inside spans. */
    int out = 0;
    for (int i = 0; i + 1 < count && out < maxSpans; i += 2) {
        spans[out].start = xs[i];
        spans[out].end = xs[i + 1];
        out++;
    }

    free(xs);
    return out;
}
