// Small neural-network forward pass over fixed weights.
// Self-contained, deterministic, std-lib only. A 3-layer MLP (4 -> 6 -> 5 -> 3)
// with ReLU on the hidden layers and a numerically-stable softmax on the output.
// All weights and biases are baked in; main() returns the argmax class plus a
// quantized confidence so the exit code is fully deterministic.
#include <array>
#include <cstdint>
#include <cmath>

namespace {

constexpr int IN  = 4;
constexpr int H1  = 6;
constexpr int H2  = 5;
constexpr int OUT = 3;

// ----- Fixed parameters (hand-chosen, deterministic) -----
constexpr std::array<std::array<double, IN>, H1> W1 = {{
    {{ 0.20, -0.15,  0.30,  0.10}},
    {{-0.25,  0.40, -0.05,  0.22}},
    {{ 0.12,  0.18, -0.30,  0.05}},
    {{ 0.33, -0.21,  0.14, -0.40}},
    {{-0.10,  0.27,  0.19,  0.31}},
    {{ 0.05, -0.33,  0.41, -0.12}}
}};
constexpr std::array<double, H1> B1 = {{ 0.10, -0.20, 0.05, 0.00, 0.15, -0.07 }};

constexpr std::array<std::array<double, H1>, H2> W2 = {{
    {{ 0.21, -0.11,  0.34,  0.07, -0.25,  0.18}},
    {{-0.30,  0.22,  0.09, -0.14,  0.27,  0.05}},
    {{ 0.16,  0.31, -0.20,  0.12,  0.04, -0.28}},
    {{ 0.08, -0.19,  0.25,  0.33, -0.06,  0.14}},
    {{-0.22,  0.13,  0.17, -0.29,  0.20,  0.10}}
}};
constexpr std::array<double, H2> B2 = {{ 0.04, -0.12, 0.08, 0.00, 0.11 }};

constexpr std::array<std::array<double, H2>, OUT> W3 = {{
    {{ 0.30, -0.18,  0.22,  0.10, -0.25}},
    {{-0.15,  0.28,  0.06, -0.20,  0.33}},
    {{ 0.12,  0.09, -0.31,  0.27,  0.05}}
}};
constexpr std::array<double, OUT> B3 = {{ 0.02, -0.05, 0.03 }};

inline double relu(double x) { return x > 0.0 ? x : 0.0; }

// Generic dense layer: out[i] = act( sum_j W[i][j] * in[j] + b[i] ).
template <int R, int C>
std::array<double, R> dense(const std::array<double, C>& in,
                            const std::array<std::array<double, C>, R>& W,
                            const std::array<double, R>& b,
                            bool apply_relu) {
    std::array<double, R> out{};
    for (int i = 0; i < R; ++i) {
        double acc = b[i];
        for (int j = 0; j < C; ++j) acc += W[i][j] * in[j];
        out[i] = apply_relu ? relu(acc) : acc;
    }
    return out;
}

// Numerically stable softmax (subtract the max logit).
std::array<double, OUT> softmax(const std::array<double, OUT>& logits) {
    double m = logits[0];
    for (int i = 1; i < OUT; ++i) if (logits[i] > m) m = logits[i];
    std::array<double, OUT> e{};
    double sum = 0.0;
    for (int i = 0; i < OUT; ++i) { e[i] = std::exp(logits[i] - m); sum += e[i]; }
    for (int i = 0; i < OUT; ++i) e[i] /= sum;
    return e;
}

} // namespace

int main() {
    // Fixed input feature vector.
    std::array<double, IN> x = {{ 0.5, -0.3, 0.8, 0.1 }};

    // Forward pass through the network.
    auto h1 = dense<H1, IN>(x,  W1, B1, true);    // ReLU
    auto h2 = dense<H2, H1>(h1, W2, B2, true);     // ReLU
    auto logits = dense<OUT, H2>(h2, W3, B3, false);
    auto probs  = softmax(logits);

    // Argmax class and its confidence.
    int best = 0;
    for (int i = 1; i < OUT; ++i) if (probs[i] > probs[best]) best = i;

    // Quantize confidence into 0..99 for a stable exit code.
    int conf = static_cast<int>(std::lround(probs[best] * 100.0));
    if (conf > 99) conf = 99;

    // Encode class in the high nibble, confidence in the low byte region.
    return static_cast<int>((best << 6) | (conf & 0x3F));
}
