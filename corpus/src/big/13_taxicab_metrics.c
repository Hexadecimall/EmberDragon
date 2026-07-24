/*
 * taxicab_metrics.c — Manhattan and Chebyshev distance metrics on a grid.
 *
 * Utilities for the two integer "city-block" metrics used throughout grid
 * pathfinding and tile games. Manhattan distance (L1) counts orthogonal steps;
 * Chebyshev distance (L-infinity) counts king moves where diagonals are free.
 * Includes nearest-point search and a bounding-radius helper, all in integers.
 */

#include <stdint.h>
#include <stdlib.h>

/* An integer grid cell coordinate. */
typedef struct {
    int32_t row;
    int32_t col;
} Cell;

/*
 * Absolute value for 32-bit integers, kept local so we never pull in <math.h>.
 * Returns |v|.
 */
static int32_t absInt(int32_t v) {
    return v < 0 ? -v : v;
}

/*
 * Manhattan (taxicab / L1) distance between two cells: the number of
 * orthogonal moves needed to get from a to b on a 4-connected grid.
 * Returns |a.row-b.row| + |a.col-b.col|.
 */
int32_t manhattanDistance(Cell a, Cell b) {
    return absInt(a.row - b.row) + absInt(a.col - b.col);
}

/*
 * Chebyshev (chessboard / L-infinity) distance: the number of king moves
 * between a and b on an 8-connected grid, where a diagonal step covers one row
 * and one column at once.
 * Returns max(|a.row-b.row|, |a.col-b.col|).
 */
int32_t chebyshevDistance(Cell a, Cell b) {
    int32_t dr = absInt(a.row - b.row);
    int32_t dc = absInt(a.col - b.col);
    return dr > dc ? dr : dc;
}

/*
 * Diagonal-aware "octile-step" count using only integers: the count of
 * diagonal moves plus the leftover orthogonal moves. This equals the number of
 * unit king moves and matches Chebyshev distance, but is computed by splitting
 * the path into a diagonal run and a straight run — useful when diagonal and
 * straight costs differ and a caller wants the two counts separately.
 * Writes the diagonal-step count to *diagOut and the straight-step count to
 * *straightOut. Returns their sum (the total move count).
 */
int32_t octileSteps(Cell a, Cell b, int32_t *diagOut, int32_t *straightOut) {
    int32_t dr = absInt(a.row - b.row);
    int32_t dc = absInt(a.col - b.col);
    int32_t diag = dr < dc ? dr : dc;     /* every diagonal consumes one of each axis */
    int32_t straight = (dr > dc ? dr : dc) - diag; /* remainder along the longer axis */
    if (diagOut) *diagOut = diag;
    if (straightOut) *straightOut = straight;
    return diag + straight;
}

/*
 * Find the index of the cell in `cells` closest to `target` under Manhattan
 * distance. Linear scan, O(n).
 * Parameters: cells — array of candidate cells; count — its length.
 * Returns the index of the nearest cell, or -1 if count is zero. Ties resolve
 * to the earliest index encountered.
 */
int nearestManhattan(const Cell *cells, int count, Cell target) {
    if (count <= 0) return -1;
    int best = 0;
    int32_t bestDist = manhattanDistance(cells[0], target);
    for (int i = 1; i < count; i++) {
        int32_t d = manhattanDistance(cells[i], target);
        if (d < bestDist) {     /* strict '<' keeps the first cell on ties */
            bestDist = d;
            best = i;
        }
    }
    return best;
}

/*
 * Count how many of the given cells fall within a closed Manhattan ball of the
 * given radius around `center` (i.e. distance <= radius). O(n).
 * Returns the number of cells inside the diamond-shaped region.
 */
int countWithinRadius(const Cell *cells, int count, Cell center, int32_t radius) {
    int inside = 0;
    for (int i = 0; i < count; i++) {
        if (manhattanDistance(cells[i], center) <= radius) {
            inside++;
        }
    }
    return inside;
}

/*
 * Whether two cells are orthogonally adjacent (share an edge) on a 4-connected
 * grid. Equivalent to a Manhattan distance of exactly 1.
 * Returns 1 if adjacent, 0 otherwise.
 */
int areOrthogonallyAdjacent(Cell a, Cell b) {
    return manhattanDistance(a, b) == 1;
}
