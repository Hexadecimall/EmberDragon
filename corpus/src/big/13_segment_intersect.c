/*
 * segment_intersect.c — exact integer segment intersection tests.
 *
 * Determines whether two closed line segments with integer endpoints intersect,
 * using only orientation (cross-product) signs. Because every test reduces to
 * the sign of an integer cross product, the result is exact — there are no
 * floating-point tolerances and no missed boundary cases. Handles the general
 * crossing case as well as all collinear-overlap special cases.
 */

#include <stdint.h>

/* A point on the integer lattice. */
typedef struct {
    int64_t x;
    int64_t y;
} Point;

/* A directed (but treated as undirected) segment from `a` to `b`. */
typedef struct {
    Point a;
    Point b;
} Segment;

/*
 * Orientation of the ordered triple (p, q, r) via the cross product of
 * (q - p) and (r - p).
 * Returns 0 if collinear, 1 if clockwise, 2 if counter-clockwise. The 0/1/2
 * encoding (rather than -1/0/1) lets callers compare orientations for equality
 * cheaply when deciding whether two points straddle a line.
 */
static int orientation(Point p, Point q, Point r) {
    int64_t val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;
    return val > 0 ? 1 : 2;
}

/*
 * Given three collinear points p, q, r, test whether q lies on segment pr.
 * Precondition: the three points are already known to be collinear.
 * Returns 1 if q is within the axis-aligned bounding box of p and r (which,
 * for collinear points, means q is on the segment), 0 otherwise.
 */
static int onSegment(Point p, Point q, Point r) {
    int64_t minX = p.x < r.x ? p.x : r.x;
    int64_t maxX = p.x > r.x ? p.x : r.x;
    int64_t minY = p.y < r.y ? p.y : r.y;
    int64_t maxY = p.y > r.y ? p.y : r.y;
    return q.x >= minX && q.x <= maxX && q.y >= minY && q.y <= maxY;
}

/*
 * Test whether two closed segments intersect, including touching at an endpoint
 * and partial/total collinear overlap.
 *
 * The general case: the segments cross when each segment straddles the line of
 * the other, which happens exactly when the two endpoints of one segment have
 * opposite orientations relative to the other segment. The special cases handle
 * collinear configurations where an endpoint of one segment lies on the other.
 *
 * Returns 1 if the segments share at least one point, 0 if they are disjoint.
 */
int segmentsIntersect(Segment s1, Segment s2) {
    Point p1 = s1.a, q1 = s1.b;
    Point p2 = s2.a, q2 = s2.b;

    /* Orientation of each endpoint of one segment w.r.t. the other segment. */
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    /* General case: the segments properly straddle each other. */
    if (o1 != o2 && o3 != o4) {
        return 1;
    }

    /* Special cases: a collinear endpoint that lands on the other segment. */
    if (o1 == 0 && onSegment(p1, p2, q1)) return 1; /* p2 lies on s1 */
    if (o2 == 0 && onSegment(p1, q2, q1)) return 1; /* q2 lies on s1 */
    if (o3 == 0 && onSegment(p2, p1, q2)) return 1; /* p1 lies on s2 */
    if (o4 == 0 && onSegment(p2, q1, q2)) return 1; /* q1 lies on s2 */

    return 0; /* no shared point */
}

/*
 * Determine whether a point lies exactly on a segment (endpoints included).
 * Returns 1 if `pt` is on segment `s`, 0 otherwise.
 */
int pointOnSegment(Point pt, Segment s) {
    /* Must be collinear with the endpoints and within their bounding box. */
    if (orientation(s.a, s.b, pt) != 0) return 0;
    return onSegment(s.a, pt, s.b);
}

/*
 * Classify how two segments relate without computing the meeting point.
 * Returns: 0 — disjoint; 1 — they cross at a single interior point; 2 — they
 * are collinear and overlap (share a sub-segment or a single shared endpoint
 * while collinear).
 */
int classifyIntersection(Segment s1, Segment s2) {
    if (!segmentsIntersect(s1, s2)) return 0;

    /* If all four cross-orientations are zero, the segments are collinear. */
    int collinear = orientation(s1.a, s1.b, s2.a) == 0 &&
                    orientation(s1.a, s1.b, s2.b) == 0;
    return collinear ? 2 : 1;
}
