/*
 * lattice_point.c — 2D integer point and vector primitives.
 *
 * Foundational integer-geometry toolkit operating on lattice points (points
 * whose coordinates are integers). Provides exact vector arithmetic, the cross
 * and dot products, orientation tests, and squared distances. Everything is
 * computed with 64-bit integer arithmetic so results are exact and immune to
 * the rounding errors that plague floating-point geometry.
 */

#include <stdint.h>

/* A point or displacement vector on the integer lattice. */
typedef struct {
    int64_t x;
    int64_t y;
} Point;

/*
 * Construct a Point from raw coordinates.
 * Returns the point (x, y) by value.
 */
Point pointMake(int64_t x, int64_t y) {
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

/*
 * Vector addition: component-wise sum of two vectors.
 * Returns a + b.
 */
Point vectorAdd(Point a, Point b) {
    return pointMake(a.x + b.x, a.y + b.y);
}

/*
 * Vector subtraction: the displacement from b to a.
 * Returns a - b, i.e. the vector pointing from b toward a.
 */
Point vectorSubtract(Point a, Point b) {
    return pointMake(a.x - b.x, a.y - b.y);
}

/*
 * Scale a vector by an integer factor.
 * Returns k * v.
 */
Point vectorScale(Point v, int64_t k) {
    return pointMake(v.x * k, v.y * k);
}

/*
 * Dot product of two vectors. Positive when the vectors point in broadly the
 * same direction, zero when perpendicular, negative when opposed.
 * Returns a . b as a 64-bit integer.
 */
int64_t dotProduct(Point a, Point b) {
    return a.x * b.x + a.y * b.y;
}

/*
 * 2D cross product (the z-component of the 3D cross of a and b).
 * Returns a.x*b.y - a.y*b.x. The sign encodes turn direction: positive means
 * b is counter-clockwise from a, negative clockwise, zero collinear.
 */
int64_t crossProduct(Point a, Point b) {
    return a.x * b.y - a.y * b.x;
}

/*
 * Orientation of the ordered triple (a, b, c): which way you turn when walking
 * a -> b -> c. This is the sign of the cross product of (b - a) and (c - a).
 * Returns +1 for a counter-clockwise (left) turn, -1 for clockwise (right),
 * and 0 if the three points are collinear.
 */
int orientation(Point a, Point b, Point c) {
    int64_t cross = crossProduct(vectorSubtract(b, a), vectorSubtract(c, a));
    if (cross > 0) return 1;
    if (cross < 0) return -1;
    return 0;
}

/*
 * Squared Euclidean distance between two points. We return the squared value
 * deliberately: it stays an exact integer, whereas the true distance would
 * require a square root and hence floating point. Squared distances preserve
 * ordering, so they suffice for nearest-point comparisons.
 * Returns |a - b|^2.
 */
int64_t squaredDistance(Point a, Point b) {
    int64_t dx = a.x - b.x;
    int64_t dy = a.y - b.y;
    return dx * dx + dy * dy;
}

/*
 * Test whether three points are collinear (lie on a single straight line).
 * Returns 1 if collinear, 0 otherwise.
 */
int areCollinear(Point a, Point b, Point c) {
    return orientation(a, b, c) == 0;
}

/*
 * Twice the signed area of the triangle (a, b, c). We return double the area
 * because the true area can be a half-integer; doubling keeps it an integer.
 * The sign follows the winding: positive for counter-clockwise vertices.
 * Returns 2 * signed_area.
 */
int64_t doubledTriangleArea(Point a, Point b, Point c) {
    return crossProduct(vectorSubtract(b, a), vectorSubtract(c, a));
}
