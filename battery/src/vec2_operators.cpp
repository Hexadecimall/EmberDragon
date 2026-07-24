// C++ class Vec2 with operator+ and operator== plus a scalar scale method.
#include <iostream>

class Vec2 {
private:
    double x_;
    double y_;

public:
    Vec2(double x, double y) : x_(x), y_(y) {}

    Vec2 operator+(const Vec2 &rhs) const {
        return Vec2(x_ + rhs.x_, y_ + rhs.y_);
    }

    bool operator==(const Vec2 &rhs) const {
        return x_ == rhs.x_ && y_ == rhs.y_;
    }

    Vec2 scaled(double k) const {
        return Vec2(x_ * k, y_ * k);
    }

    double x() const { return x_; }
    double y() const { return y_; }
};

int main() {
    Vec2 a(1.0, 2.0);
    Vec2 b(3.0, -1.0);

    Vec2 sum = a + b;
    Vec2 doubled = sum.scaled(2.0);

    std::cout << "sum   = (" << sum.x() << ", " << sum.y() << ")\n";
    std::cout << "scale = (" << doubled.x() << ", " << doubled.y() << ")\n";

    Vec2 expected(4.0, 1.0);
    std::cout << "equal = " << (sum == expected ? "yes" : "no") << "\n";
    return 0;
}
