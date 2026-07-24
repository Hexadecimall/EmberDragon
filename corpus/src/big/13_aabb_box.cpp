/*
 * aabb_box.cpp — axis-aligned bounding boxes over the integer lattice.
 *
 * A small value-type for integer axis-aligned bounding boxes (AABBs) with the
 * usual broad-phase operations: containment, overlap, intersection, union, and
 * growth. All coordinates are integers and all boxes are treated as closed
 * (both the min and max edges belong to the box), which is the natural
 * convention for tile and pixel grids.
 */

#include <cstdint>

/*
 * A closed axis-aligned box. Invariant for a *valid* box: minX <= maxX and
 * minY <= maxY. The empty box is represented with min > max and is produced by
 * makeEmpty(); intersection() may return such a box when there is no overlap.
 */
struct Box {
    int64_t minX;
    int64_t minY;
    int64_t maxX;
    int64_t maxY;
};

/*
 * Build a box from two corner points in any order, normalizing so the min/max
 * invariant holds regardless of which corner the caller passed first.
 * Returns the smallest box containing both corners.
 */
Box makeBox(int64_t x0, int64_t y0, int64_t x1, int64_t y1) {
    Box b;
    b.minX = x0 < x1 ? x0 : x1;
    b.maxX = x0 > x1 ? x0 : x1;
    b.minY = y0 < y1 ? y0 : y1;
    b.maxY = y0 > y1 ? y0 : y1;
    return b;
}

/*
 * Produce a canonical empty box. We seed min above max so that any union with a
 * real point or box immediately yields a valid box and an empty box reports a
 * non-positive area. Returns the sentinel empty box.
 */
Box makeEmpty() {
    Box b;
    b.minX = 1;
    b.minY = 1;
    b.maxX = 0; /* maxX < minX marks emptiness */
    b.maxY = 0;
    return b;
}

/*
 * Whether a box is empty (encloses no cells) under the closed-box convention.
 * Returns true if either axis has its max strictly below its min.
 */
bool isEmpty(const Box &b) {
    return b.maxX < b.minX || b.maxY < b.minY;
}

/*
 * Width of the box in integer cells, counting both edges (closed box). For a
 * one-column box minX == maxX the width is 1.
 * Returns the cell width, or 0 if the box is empty.
 */
int64_t boxWidth(const Box &b) {
    if (isEmpty(b)) return 0;
    return b.maxX - b.minX + 1;
}

/*
 * Height of the box in integer cells, counting both edges (closed box).
 * Returns the cell height, or 0 if the box is empty.
 */
int64_t boxHeight(const Box &b) {
    if (isEmpty(b)) return 0;
    return b.maxY - b.minY + 1;
}

/*
 * Area of the box in cells.
 * Returns width * height (0 for an empty box).
 */
int64_t boxArea(const Box &b) {
    return boxWidth(b) * boxHeight(b);
}

/*
 * Point-in-box test for a closed box (edges included).
 * Returns true if (px, py) lies within or on the boundary of b.
 */
bool containsPoint(const Box &b, int64_t px, int64_t py) {
    return px >= b.minX && px <= b.maxX && py >= b.minY && py <= b.maxY;
}

/*
 * Overlap test: do two closed boxes share at least one cell? Two boxes are
 * disjoint exactly when one lies entirely to a side of the other on some axis.
 * Returns true if the boxes intersect.
 */
bool boxesOverlap(const Box &a, const Box &b) {
    if (isEmpty(a) || isEmpty(b)) return false;
    if (a.maxX < b.minX || b.maxX < a.minX) return false; /* separated in X */
    if (a.maxY < b.minY || b.maxY < a.minY) return false; /* separated in Y */
    return true;
}

/*
 * Geometric intersection of two boxes.
 * Returns the overlapping box, or an empty box (via makeEmpty's convention) if
 * they do not overlap.
 */
Box boxIntersection(const Box &a, const Box &b) {
    Box r;
    r.minX = a.minX > b.minX ? a.minX : b.minX;
    r.minY = a.minY > b.minY ? a.minY : b.minY;
    r.maxX = a.maxX < b.maxX ? a.maxX : b.maxX;
    r.maxY = a.maxY < b.maxY ? a.maxY : b.maxY;
    return r; /* may be empty; callers should check isEmpty() */
}

/*
 * Smallest box enclosing both inputs (their bounding union). An empty input is
 * ignored so that union with the empty box is the identity.
 * Returns the union box.
 */
Box boxUnion(const Box &a, const Box &b) {
    if (isEmpty(a)) return b;
    if (isEmpty(b)) return a;
    Box r;
    r.minX = a.minX < b.minX ? a.minX : b.minX;
    r.minY = a.minY < b.minY ? a.minY : b.minY;
    r.maxX = a.maxX > b.maxX ? a.maxX : b.maxX;
    r.maxY = a.maxY > b.maxY ? a.maxY : b.maxY;
    return r;
}

/*
 * Grow (or shrink, with negative margin) a box uniformly on every side.
 * A large negative margin can invert the box; the result then reports as empty.
 * Returns the inflated box.
 */
Box inflate(const Box &b, int64_t margin) {
    Box r;
    r.minX = b.minX - margin;
    r.minY = b.minY - margin;
    r.maxX = b.maxX + margin;
    r.maxY = b.maxY + margin;
    return r;
}
