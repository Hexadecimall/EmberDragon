/*
 * vector.cpp — Integer 3D vector math for grid- and lattice-based geometry.
 *
 * A small value-type vector class with the usual lattice operations:
 * addition, subtraction, scaling, dot product, cross product, squared
 * length, and Manhattan/Chebyshev distances. Everything stays in exact
 * 64-bit integers, which is ideal for voxel grids and tile maps where
 * floating-point drift is undesirable.
 */

#include <cstdint>

/* A 3D vector over the integer lattice Z^3. */
class Vec3 {
public:
    int64_t x, y, z;

    /* Construct the zero vector. */
    Vec3() : x(0), y(0), z(0) {}

    /* Construct from explicit components. */
    Vec3(int64_t xi, int64_t yi, int64_t zi) : x(xi), y(yi), z(zi) {}

    /* Componentwise sum: returns this + other. */
    Vec3 add(const Vec3 &other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    /* Componentwise difference: returns this - other. */
    Vec3 subtract(const Vec3 &other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    /* Scalar multiply: returns this * factor. */
    Vec3 scale(int64_t factor) const {
        return Vec3(x * factor, y * factor, z * factor);
    }

    /*
     * Dot product with `other`. Returns x1*x2 + y1*y2 + z1*z2.
     * A zero result means the two vectors are perpendicular.
     */
    int64_t dot(const Vec3 &other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    /*
     * Cross product: returns a vector perpendicular to both operands,
     * following the right-hand rule. The result is the zero vector iff
     * the inputs are parallel (or one is zero).
     */
    Vec3 cross(const Vec3 &other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    /*
     * Squared Euclidean length. Returned instead of the true length so
     * the result stays an exact integer (no sqrt). Useful for distance
     * comparisons, where comparing squared magnitudes is equivalent.
     */
    int64_t lengthSquared() const {
        return x * x + y * y + z * z;
    }

    /* Returns 1 if this is the zero vector, 0 otherwise. */
    int isZero() const {
        return (x == 0 && y == 0 && z == 0) ? 1 : 0;
    }
};

/*
 * Absolute value of a signed 64-bit integer.
 *   v: the input value.
 * Returns |v|. Factored out so the distance helpers avoid <cstdlib>.
 */
static int64_t absInt(int64_t v) {
    return (v < 0) ? -v : v;
}

/*
 * Manhattan (L1) distance between two lattice points: the sum of the
 * absolute coordinate differences. This is the minimal number of axis-
 * aligned unit steps between them.
 *   a, b: the two points.
 * Returns |dx| + |dy| + |dz|.
 */
int64_t manhattanDistance(const Vec3 &a, const Vec3 &b) {
    Vec3 d = a.subtract(b);
    return absInt(d.x) + absInt(d.y) + absInt(d.z);
}

/*
 * Chebyshev (L-infinity) distance: the largest single-axis difference.
 * On a grid that permits diagonal moves, this is the minimal step count.
 *   a, b: the two points.
 * Returns max(|dx|, |dy|, |dz|).
 */
int64_t chebyshevDistance(const Vec3 &a, const Vec3 &b) {
    Vec3 d = a.subtract(b);
    int64_t ax = absInt(d.x), ay = absInt(d.y), az = absInt(d.z);
    int64_t best = ax;
    if (ay > best) best = ay;  /* keep the running maximum */
    if (az > best) best = az;
    return best;
}

/*
 * Test whether two vectors are parallel (collinear). They are parallel
 * exactly when their cross product is the zero vector.
 *   a, b: the two vectors.
 * Returns 1 if parallel, 0 otherwise.
 */
int areParallel(const Vec3 &a, const Vec3 &b) {
    return a.cross(b).isZero();
}
