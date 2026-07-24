// Tiny LCG RNG feeding a histogram + integer mean/variance; float math at end.
#include <cstdio>
#include <cstdint>
#include <cmath>

struct LCG {
    uint64_t state;
    uint32_t next() {                       // classic 64-bit MMIX-style LCG
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return (uint32_t)(state >> 32);     // high bits are the good ones
    }
};

int main(int argc, char **) {
    LCG rng{ 0x1234567 + (uint64_t)argc };
    uint32_t bins[8] = {0};
    uint64_t sum = 0, sumsq = 0;
    const int N = 4000;

    for (int i = 0; i < N; i++) {
        uint32_t r = rng.next();
        uint32_t bucket = r >> 29;          // top 3 bits -> 0..7
        bins[bucket]++;
        uint32_t v = r % 100;               // 0..99
        sum   += v;
        sumsq += (uint64_t)v * v;
    }

    double mean = (double)sum / N;
    double var  = (double)sumsq / N - mean * mean;
    double sd   = std::sqrt(var);

    int maxbin = 0;
    for (int i = 1; i < 8; i++) if (bins[i] > bins[maxbin]) maxbin = i;

    // round derived floats to ints for clean, deterministic-ish output
    printf("mean=%d sd_x100=%d maxbin=%d count=%u\n",
           (int)(mean + 0.5), (int)(sd * 100 + 0.5), maxbin, bins[maxbin]);
    return maxbin;
}
