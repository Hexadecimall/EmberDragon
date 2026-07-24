// Temperature/units converter: C<->F<->K plus miles<->km, driven by argv or a demo table.
#include <iostream>
#include <string>
#include <cmath>

static double c_to_f(double c) { return c * 9.0 / 5.0 + 32.0; }
static double f_to_c(double f) { return (f - 32.0) * 5.0 / 9.0; }
static double c_to_k(double c) { return c + 273.15; }
static double mi_to_km(double mi) { return mi * 1.609344; }

static double convert(const std::string &kind, double v) {
    if (kind == "c2f") return c_to_f(v);
    if (kind == "f2c") return f_to_c(v);
    if (kind == "c2k") return c_to_k(v);
    if (kind == "mi2km") return mi_to_km(v);
    return NAN;
}

int main(int argc, char **argv) {
    if (argc == 3) {
        double r = convert(argv[1], std::stod(argv[2]));
        if (std::isnan(r)) {
            std::cerr << "unknown conversion: " << argv[1] << "\n";
            return 1;
        }
        std::cout << argv[1] << "(" << argv[2] << ") = " << r << "\n";
        return 0;
    }

    // Deterministic demo when no args supplied.
    const double samples[] = {0.0, 37.0, 100.0};
    for (double c : samples) {
        std::cout << c << "C = " << c_to_f(c) << "F = "
                  << c_to_k(c) << "K\n";
    }
    std::cout << "26.2 mi = " << mi_to_km(26.2) << " km\n";
    return 0;
}
