// Polymorphic shape hierarchy: virtual base + 3 derived, called through base*.
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

class Shape {
public:
    Shape(double ox, double oy) : ox_(ox), oy_(oy) {}
    virtual ~Shape() = default;

    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual int sides() const = 0;
    // non-pure virtual with a default body the decompiler must see overridden
    virtual double bounding_radius() const {
        return std::sqrt(area() / 3.14159265358979323846);
    }
    double cx() const { return ox_; }
    double cy() const { return oy_; }

protected:
    double ox_, oy_;
};

class Circle : public Shape {
public:
    Circle(double x, double y, double r) : Shape(x, y), r_(r) {}
    double area() const override { return 3.14159265358979323846 * r_ * r_; }
    double perimeter() const override { return 2.0 * 3.14159265358979323846 * r_; }
    int sides() const override { return 0; }
    double bounding_radius() const override { return r_; }

private:
    double r_;
};

class Rectangle : public Shape {
public:
    Rectangle(double x, double y, double w, double h)
        : Shape(x, y), w_(w), h_(h) {}
    double area() const override { return w_ * h_; }
    double perimeter() const override { return 2.0 * (w_ + h_); }
    int sides() const override { return 4; }
    double bounding_radius() const override {
        return 0.5 * std::sqrt(w_ * w_ + h_ * h_);
    }

private:
    double w_, h_;
};

class Triangle : public Shape {
public:
    Triangle(double x, double y, double a, double b, double c)
        : Shape(x, y), a_(a), b_(b), c_(c) {}
    double perimeter() const override { return a_ + b_ + c_; }
    double area() const override {
        double s = perimeter() * 0.5;
        return std::sqrt(s * (s - a_) * (s - b_) * (s - c_));
    }
    int sides() const override { return 3; }

private:
    double a_, b_, c_;
};

static uint64_t fingerprint(const std::vector<std::unique_ptr<Shape>>& v) {
    double total_area = 0.0, total_perim = 0.0, total_radius = 0.0;
    int side_sum = 0;
    for (const auto& s : v) {
        total_area += s->area();          // virtual dispatch
        total_perim += s->perimeter();    // virtual dispatch
        total_radius += s->bounding_radius();
        side_sum += s->sides() + (int)(s->cx() + s->cy());
    }
    uint64_t h = 1469598103934665603ull;
    double parts[3] = {total_area, total_perim, total_radius};
    for (double d : parts) {
        uint64_t bits;
        __builtin_memcpy(&bits, &d, sizeof(bits));
        h = (h ^ bits) * 1099511628211ull;
    }
    return h ^ (uint64_t)side_sum;
}

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(1.0, 2.0, 3.0));
    shapes.push_back(std::make_unique<Rectangle>(0.0, 0.0, 4.0, 5.0));
    shapes.push_back(std::make_unique<Triangle>(2.0, 2.0, 3.0, 4.0, 5.0));
    shapes.push_back(std::make_unique<Circle>(-1.0, -1.0, 1.5));

    uint64_t fp = fingerprint(shapes);
    int total_sides = 0;
    for (const auto& s : shapes) total_sides += s->sides();
    return (int)((fp & 0x7f) ^ total_sides);
}
