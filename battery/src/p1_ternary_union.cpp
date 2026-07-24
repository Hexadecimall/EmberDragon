// Nested ternaries + tagged union (variant) classifier.
#include <cstdio>
#include <cstdint>

enum class Kind : uint8_t { Int, Flt, Chr };

struct Value {
    Kind tag;
    union { int64_t i; double d; char c; } u;
};

// Deeply nested ternary picks a one-char "grade" from the magnitude.
static char grade(double m) {
    return m < 0.0   ? '!'
         : m < 1.0   ? 'a'
         : m < 10.0  ? (m < 5.0 ? 'b' : 'c')
         : m < 100.0 ? (m < 50.0 ? 'd' : 'e')
                     : 'f';
}

static double magnitude(const Value& v) {
    return v.tag == Kind::Int ? (double)v.u.i
         : v.tag == Kind::Flt ? (v.u.d < 0 ? -v.u.d : v.u.d)
                              : (double)(int)v.u.c;
}

int main() {
    Value vs[4];
    vs[0] = { Kind::Int, {} }; vs[0].u.i = 42;
    vs[1] = { Kind::Flt, {} }; vs[1].u.d = -3.5;
    vs[2] = { Kind::Chr, {} }; vs[2].u.c = 'Z';   // 90
    vs[3] = { Kind::Int, {} }; vs[3].u.i = -7;
    for (int k = 0; k < 4; ++k) {
        double m = magnitude(vs[k]);
        std::printf("v%d tag=%u mag=%.2f grade=%c\n",
                    k, (unsigned)vs[k].tag, m, grade(m));
    }
    return 0;
}
