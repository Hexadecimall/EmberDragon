// Fixed-point Mandelbrot escape-count grid (pure integer arithmetic).
// Self-contained, deterministic, std-lib only. No floating point in the inner
// loop: coordinates and iterates are Q16.16 fixed-point integers. Computes a
// histogram of escape counts over a grid and returns a checksum from main().
#include <array>
#include <cstdint>

namespace {

// Q16.16 fixed-point: 16 integer bits, 16 fractional bits.
constexpr int FRAC = 16;
constexpr std::int64_t ONE = std::int64_t(1) << FRAC;

// Fixed-point multiply with rounding (inputs/outputs Q16.16).
inline std::int64_t fmul(std::int64_t a, std::int64_t b) {
    return (a * b) >> FRAC;          // 64-bit product avoids overflow here
}

constexpr int WIDTH    = 48;
constexpr int HEIGHT   = 32;
constexpr int MAX_ITER = 256;

// Map an integer pixel coordinate to a Q16.16 value in [lo, hi].
inline std::int64_t lerp_fixed(int idx, int count, std::int64_t lo, std::int64_t hi) {
    // lo + (hi - lo) * idx / (count - 1), all in fixed-point.
    std::int64_t span = hi - lo;
    std::int64_t t = (std::int64_t(idx) << FRAC) / (count - 1);   // Q16.16 in [0,1]
    return lo + fmul(span, t);
}

// Escape-time iteration count for one complex point (cre, cim) in Q16.16.
int escape_count(std::int64_t cre, std::int64_t cim) {
    std::int64_t zr = 0, zi = 0;
    const std::int64_t four = 4 * ONE;
    for (int i = 0; i < MAX_ITER; ++i) {
        std::int64_t zr2 = fmul(zr, zr);
        std::int64_t zi2 = fmul(zi, zi);
        if (zr2 + zi2 > four) return i;       // |z|^2 > 4 -> escaped
        std::int64_t two_zr_zi = 2 * fmul(zr, zi);
        zr = zr2 - zi2 + cre;
        zi = two_zr_zi + cim;
    }
    return MAX_ITER;                          // did not escape
}

} // namespace

int main() {
    // View window in the complex plane, expressed in Q16.16.
    const std::int64_t re_lo = -(ONE * 5 / 2);   // -2.5
    const std::int64_t re_hi =  (ONE);           //  1.0
    const std::int64_t im_lo = -(ONE * 5 / 4);   // -1.25
    const std::int64_t im_hi =  (ONE * 5 / 4);   //  1.25

    // Bucket escape counts into a small histogram.
    std::array<std::int64_t, 9> hist{};
    std::int64_t total_iters = 0;
    std::int64_t in_set = 0;

    for (int py = 0; py < HEIGHT; ++py) {
        std::int64_t cim = lerp_fixed(py, HEIGHT, im_lo, im_hi);
        for (int px = 0; px < WIDTH; ++px) {
            std::int64_t cre = lerp_fixed(px, WIDTH, re_lo, re_hi);
            int n = escape_count(cre, cim);
            total_iters += n;
            if (n == MAX_ITER) ++in_set;

            // Histogram bucket: log2-ish bins by leading position.
            int bucket = 0;
            int v = n;
            while (v > 1 && bucket < 8) { v >>= 1; ++bucket; }
            ++hist[bucket];
        }
    }

    // Fold everything into a deterministic checksum.
    std::int64_t checksum = total_iters * 2654435761LL + in_set * 40503LL;
    for (std::size_t b = 0; b < hist.size(); ++b)
        checksum ^= hist[b] << (b * 3);

    return static_cast<int>(checksum & 0xFF);
}
