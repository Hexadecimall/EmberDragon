// Early returns & guard clauses: input validation/classification with bail-outs.
#include <iostream>
#include <string>
#include <cstdint>

// Returns a classification code; every failure path returns early.
static int classify_triangle(long a, long b, long c) {
    if (a <= 0 || b <= 0 || c <= 0) return -1;          // non-positive side
    if (a + b <= c || a + c <= b || b + c <= a) return -2; // degenerate
    if (a == b && b == c) return 3;                      // equilateral
    if (a == b || b == c || a == c) return 2;            // isosceles
    return 1;                                            // scalene
}

static const char *name_for(int code) {
    switch (code) {
        case -1: return "invalid (non-positive side)";
        case -2: return "invalid (fails inequality)";
        case 1:  return "scalene";
        case 2:  return "isosceles";
        case 3:  return "equilateral";
        default: return "unknown";
    }
}

int main() {
    long tests[][3] = {
        {3, 4, 5}, {5, 5, 5}, {5, 5, 8}, {1, 2, 3}, {-1, 4, 5}, {10, 1, 1}
    };
    for (auto &t : tests) {
        int code = classify_triangle(t[0], t[1], t[2]);
        std::cout << t[0] << ',' << t[1] << ',' << t[2]
                  << " -> " << name_for(code) << '\n';
    }
    return 0;
}
