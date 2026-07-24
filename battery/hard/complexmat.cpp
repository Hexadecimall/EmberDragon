// Operator-overload-heavy Complex + 2x2 Matrix algebra.
#include <cmath>
#include <cstdint>

struct Complex {
    double re, im;
    Complex(double r = 0.0, double i = 0.0) : re(r), im(i) {}

    Complex operator+(const Complex& o) const { return {re + o.re, im + o.im}; }
    Complex operator-(const Complex& o) const { return {re - o.re, im - o.im}; }
    Complex operator*(const Complex& o) const {
        return {re * o.re - im * o.im, re * o.im + im * o.re};
    }
    Complex operator/(const Complex& o) const {
        double d = o.re * o.re + o.im * o.im;
        return {(re * o.re + im * o.im) / d, (im * o.re - re * o.im) / d};
    }
    Complex operator-() const { return {-re, -im}; }
    Complex& operator+=(const Complex& o) { re += o.re; im += o.im; return *this; }
    Complex& operator*=(const Complex& o) { *this = *this * o; return *this; }
    bool operator==(const Complex& o) const { return re == o.re && im == o.im; }

    double abs2() const { return re * re + im * im; }
    Complex conj() const { return {re, -im}; }
};

// scalar on the left
Complex operator*(double s, const Complex& c) { return {s * c.re, s * c.im}; }

struct Mat2 {
    Complex a, b, c, d;  // [[a b][c d]]

    Mat2 operator+(const Mat2& o) const { return {a + o.a, b + o.b, c + o.c, d + o.d}; }
    Mat2 operator*(const Mat2& o) const {
        return {a * o.a + b * o.c, a * o.b + b * o.d,
                c * o.a + d * o.c, c * o.b + d * o.d};
    }
    Mat2 operator*(const Complex& s) const { return {a * s, b * s, c * s, d * s}; }
    Complex det() const { return a * d - b * c; }
    Mat2 transpose() const { return {a, c, b, d}; }

    // index into a flattened 2x2 by operator[]
    Complex operator[](int i) const {
        switch (i & 3) {
            case 0: return a;
            case 1: return b;
            case 2: return c;
            default: return d;
        }
    }
};

static Mat2 mat_pow(Mat2 m, unsigned e) {
    Mat2 result{Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)};
    while (e) {
        if (e & 1u) result = result * m;
        m = m * m;
        e >>= 1;
    }
    return result;
}

int main() {
    Complex x(1.0, 2.0), y(-3.0, 0.5);
    Complex sum = x + y;
    Complex prod = x * y;
    Complex quot = x / y;
    Complex neg = -x;
    sum += prod;
    prod *= y.conj();

    Mat2 m{Complex(0, 1), Complex(1, 0), Complex(1, 0), Complex(0, -1)};
    Mat2 n = m * m + m.transpose();
    Mat2 p = mat_pow(m, 5u);
    Complex scaled_det = (n * Complex(2.0, 0.0)).det();

    double acc = 0.0;
    acc += sum.abs2() + prod.abs2() + quot.abs2() + neg.abs2();
    acc += scaled_det.abs2() + p.det().abs2();
    for (int i = 0; i < 4; ++i) acc += p[i].abs2();
    acc += (x == y) ? 1000.0 : 7.0;

    uint64_t bits;
    __builtin_memcpy(&bits, &acc, sizeof(bits));
    return (int)((bits >> 17) & 0x3f);
}
