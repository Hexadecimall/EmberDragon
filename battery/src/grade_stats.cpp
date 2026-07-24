// Grade/stats calculator: mean, min, max, stddev, and letter grade over a fixed array.
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

static char letter(double score) {
    if (score >= 90.0) return 'A';
    if (score >= 80.0) return 'B';
    if (score >= 70.0) return 'C';
    if (score >= 60.0) return 'D';
    return 'F';
}

int main() {
    std::vector<double> scores = {91.5, 78.0, 64.25, 88.0, 99.0, 55.5, 72.0};

    double sum = 0.0, lo = scores[0], hi = scores[0];
    for (double s : scores) {
        sum += s;
        if (s < lo) lo = s;
        if (s > hi) hi = s;
    }
    double mean = sum / static_cast<double>(scores.size());

    double var = 0.0;
    for (double s : scores) {
        double d = s - mean;
        var += d * d;
    }
    var /= static_cast<double>(scores.size());
    double stddev = std::sqrt(var);

    std::cout << "n=" << scores.size() << "\n";
    std::cout << "mean=" << mean << " min=" << lo << " max=" << hi << "\n";
    std::cout << "stddev=" << stddev << "\n";

    int counts[5] = {0, 0, 0, 0, 0};
    for (double s : scores) {
        char g = letter(s);
        counts[g - 'A'] += (g == 'F') ? 0 : 1; // keep A..D distinct
        if (g == 'F') counts[4]++;
    }
    const char *names = "ABCDF";
    for (int i = 0; i < 5; i++) {
        std::cout << names[i] << ": " << counts[i] << "\n";
    }
    std::cout << "class grade: " << letter(mean) << "\n";
    return 0;
}
