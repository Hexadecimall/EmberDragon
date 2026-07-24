// Iterative radix-2 Cooley-Tukey FFT on a fixed 16-point real input array.
// Self-contained, deterministic, std-lib only. Computes the forward DFT via
// in-place bit-reversal + butterfly stages, then reconstructs the signal with
// an inverse FFT and accumulates a checksum that main() returns.
#include <array>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cmath>

namespace {

constexpr std::size_t N = 16;                 // must be a power of two
using cd = std::complex<double>;

// Reverse the low log2(N) bits of an index (used to permute the input).
std::size_t bit_reverse(std::size_t x, unsigned bits) {
    std::size_t r = 0;
    for (unsigned i = 0; i < bits; ++i) {
        r = (r << 1) | (x & 1u);
        x >>= 1;
    }
    return r;
}

// In-place iterative FFT. sign = -1 forward, +1 inverse (without 1/N scaling).
void fft(std::array<cd, N>& a, int sign) {
    unsigned bits = 0;
    while ((std::size_t(1) << bits) < N) ++bits;

    // Bit-reversal permutation.
    for (std::size_t i = 0; i < N; ++i) {
        std::size_t j = bit_reverse(i, bits);
        if (j > i) std::swap(a[i], a[j]);
    }

    // Butterfly stages: length doubles each pass.
    for (std::size_t len = 2; len <= N; len <<= 1) {
        double theta = sign * 2.0 * M_PI / static_cast<double>(len);
        cd wlen(std::cos(theta), std::sin(theta));
        for (std::size_t start = 0; start < N; start += len) {
            cd w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                cd u = a[start + k];
                cd v = a[start + k + len / 2] * w;
                a[start + k]           = u + v;
                a[start + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

// Quantize a double to an integer grid so the checksum is stable across libm.
std::int64_t quantize(double x) {
    return static_cast<std::int64_t>(std::llround(x * 1000.0));
}

} // namespace

int main() {
    // Deterministic test signal: sum of two sinusoids plus a ramp.
    std::array<cd, N> a{};
    for (std::size_t n = 0; n < N; ++n) {
        double t = static_cast<double>(n);
        double s = 3.0 * std::sin(2.0 * M_PI * t / N)
                 + 1.5 * std::cos(2.0 * M_PI * 3.0 * t / N)
                 + 0.25 * t;
        a[n] = cd(s, 0.0);
    }

    // Forward transform.
    fft(a, -1);

    // Spectral magnitude checksum.
    std::int64_t spectral = 0;
    for (std::size_t k = 0; k < N; ++k) {
        spectral += quantize(std::abs(a[k]));
    }

    // Inverse transform (with 1/N scaling) should recover the input.
    fft(a, +1);
    double inv_scale = 1.0 / static_cast<double>(N);
    std::int64_t recon = 0;
    for (std::size_t n = 0; n < N; ++n) {
        recon += quantize(a[n].real() * inv_scale);
    }

    // Combine into a small deterministic exit code (0..255).
    std::int64_t mixed = (spectral ^ (recon * 31)) & 0xFF;
    return static_cast<int>(mixed);
}
