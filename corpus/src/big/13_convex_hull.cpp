/*
 * convex_hull.cpp — integer convex hull via Andrew's monotone chain.
 *
 * Computes the convex hull (the smallest convex polygon enclosing a set of
 * lattice points) using Andrew's monotone-chain algorithm. The points are
 * sorted once, then the lower and upper hulls are built with a stack-based
 * sweep. Every geometric decision is an integer cross-product sign, so the hull
 * is exact. Overall cost is O(n log n), dominated by the sort.
 */

#include <cstdint>
#include <cstdlib>

/* A lattice point. */
struct Point {
    int64_t x;
    int64_t y;
};

/*
 * Cross product of vectors (b - o) and (c - o). Its sign tells us the turn
 * direction at o when going o -> b -> c.
 * Returns > 0 for a counter-clockwise turn, < 0 for clockwise, 0 if collinear.
 */
static int64_t cross(const Point &o, const Point &b, const Point &c) {
    return (b.x - o.x) * (c.y - o.y) - (b.y - o.y) * (c.x - o.x);
}

/*
 * Comparator for qsort: orders points by x ascending, breaking ties by y
 * ascending. This lexicographic order is what the monotone-chain sweep needs.
 * Returns a negative/zero/positive int per the qsort contract.
 */
static int comparePoints(const void *pa, const void *pb) {
    const Point *a = (const Point *)pa;
    const Point *b = (const Point *)pb;
    if (a->x != b->x) return a->x < b->x ? -1 : 1;
    if (a->y != b->y) return a->y < b->y ? -1 : 1;
    return 0;
}

/*
 * Compute the convex hull of `n` points.
 *
 * Parameters:
 *   points — input array (its order is irrelevant; it is copied and sorted).
 *   n      — number of input points.
 *   hull   — caller-supplied output buffer; must hold at least 2*n points in
 *            the worst case (degenerate inputs). Vertices are written in
 *            counter-clockwise order with no repeated closing vertex.
 *
 * Returns the number of vertices written to `hull`. Returns n for inputs of 0,
 * 1, or 2 points (the hull is just the points themselves). Collinear points on
 * an edge are excluded because we discard non-left turns with a strict test.
 * Complexity: O(n log n).
 */
int convexHull(const Point *points, int n, Point *hull) {
    if (n < 3) {
        /* Too few points to form a polygon; the hull is the point set itself. */
        for (int i = 0; i < n; i++) hull[i] = points[i];
        return n;
    }

    /* Work on a sorted copy so the caller's array is left untouched. */
    Point *sorted = (Point *)malloc((size_t)n * sizeof(Point));
    for (int i = 0; i < n; i++) sorted[i] = points[i];
    qsort(sorted, (size_t)n, sizeof(Point), comparePoints);

    int k = 0; /* current number of vertices on the hull-under-construction */

    /* Build the lower hull: sweep left to right, keeping only left turns. */
    for (int i = 0; i < n; i++) {
        /* Pop vertices that would make a clockwise or collinear turn. */
        while (k >= 2 && cross(hull[k - 2], hull[k - 1], sorted[i]) <= 0) {
            k--;
        }
        hull[k++] = sorted[i];
    }

    /* Build the upper hull: sweep right to left. `lower` fixes the floor so we
     * never collapse past the points the lower hull already committed. */
    int lower = k + 1;
    for (int i = n - 2; i >= 0; i--) {
        while (k >= lower && cross(hull[k - 2], hull[k - 1], sorted[i]) <= 0) {
            k--;
        }
        hull[k++] = sorted[i];
    }

    free(sorted);

    /* The last vertex equals the first (the sweep closes the loop), so drop it. */
    return k - 1;
}

/*
 * Twice the area of a simple polygon via the shoelace formula. We return double
 * the area to keep the value an exact integer (a polygon's area can be a
 * half-integer). The sign reflects winding: positive for counter-clockwise.
 * Parameters: poly — vertex array; n — vertex count.
 * Returns 2 * signed_area. Take the absolute value for an unsigned area.
 */
int64_t doubledPolygonArea(const Point *poly, int n) {
    int64_t sum = 0;
    for (int i = 0; i < n; i++) {
        const Point &cur = poly[i];
        const Point &nxt = poly[(i + 1) % n]; /* wrap to close the polygon */
        sum += cur.x * nxt.y - nxt.x * cur.y;
    }
    return sum;
}
