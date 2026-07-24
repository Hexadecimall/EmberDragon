#include <stdint.h>

class Vector3 {
public:
    int64_t x;
    int64_t y;
    int64_t z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(int64_t px, int64_t py, int64_t pz) : x(px), y(py), z(pz) {}

    Vector3 add(const Vector3 &other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 subtract(const Vector3 &other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 scale(int64_t factor) const {
        return Vector3(x * factor, y * factor, z * factor);
    }

    int64_t dot(const Vector3 &other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3 &other) const {
        int64_t cx = y * other.z - z * other.y;
        int64_t cy = z * other.x - x * other.z;
        int64_t cz = x * other.y - y * other.x;
        return Vector3(cx, cy, cz);
    }

    int64_t lengthSquared() const {
        return x * x + y * y + z * z;
    }

    bool isZero() const {
        return x == 0 && y == 0 && z == 0;
    }

    bool equals(const Vector3 &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

int64_t manhattanDistance(const Vector3 &a, const Vector3 &b) {
    int64_t dx = a.x - b.x;
    int64_t dy = a.y - b.y;
    int64_t dz = a.z - b.z;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    if (dz < 0) dz = -dz;
    return dx + dy + dz;
}

bool isOrthogonal(const Vector3 &a, const Vector3 &b) {
    return a.dot(b) == 0;
}

Vector3 negate(const Vector3 &v) {
    return Vector3(-v.x, -v.y, -v.z);
}

int64_t scalarTripleProduct(const Vector3 &a, const Vector3 &b, const Vector3 &c) {
    Vector3 crossed = b.cross(c);
    return a.dot(crossed);
}
