// Base64 encoder + decoder with a self-verifying round-trip.
// Standard RFC 4648 alphabet, '=' padding, strict decode (rejects bad chars
// and malformed padding). No input I/O: encodes/decodes a fixed corpus and
// confirms decode(encode(x)) == x for every entry, deterministically.
#include <string>
#include <vector>
#include <cstdint>

namespace b64 {

static const char* kAlphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string encode(const std::vector<uint8_t>& in) {
    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);
    size_t i = 0;
    size_t full = in.size() / 3;
    for (size_t b = 0; b < full; ++b) {
        uint32_t n = (uint32_t(in[i]) << 16) | (uint32_t(in[i + 1]) << 8) | in[i + 2];
        i += 3;
        out.push_back(kAlphabet[(n >> 18) & 0x3f]);
        out.push_back(kAlphabet[(n >> 12) & 0x3f]);
        out.push_back(kAlphabet[(n >> 6) & 0x3f]);
        out.push_back(kAlphabet[n & 0x3f]);
    }
    size_t rem = in.size() - i;
    if (rem == 1) {
        uint32_t n = uint32_t(in[i]) << 16;
        out.push_back(kAlphabet[(n >> 18) & 0x3f]);
        out.push_back(kAlphabet[(n >> 12) & 0x3f]);
        out.push_back('=');
        out.push_back('=');
    } else if (rem == 2) {
        uint32_t n = (uint32_t(in[i]) << 16) | (uint32_t(in[i + 1]) << 8);
        out.push_back(kAlphabet[(n >> 18) & 0x3f]);
        out.push_back(kAlphabet[(n >> 12) & 0x3f]);
        out.push_back(kAlphabet[(n >> 6) & 0x3f]);
        out.push_back('=');
    }
    return out;
}

static int decodeChar(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

// Returns true on success; fills `out`. Strict: length must be a multiple of 4,
// padding only allowed at the end, no stray characters.
bool decode(const std::string& in, std::vector<uint8_t>& out) {
    out.clear();
    if (in.size() % 4 != 0) return false;
    for (size_t i = 0; i < in.size(); i += 4) {
        int q[4];
        int pad = 0;
        for (int k = 0; k < 4; ++k) {
            char c = in[i + k];
            if (c == '=') {
                // Padding may only occupy the last one or two positions.
                if (i + 4 != in.size() || k < 2) return false;
                q[k] = 0;
                ++pad;
            } else {
                if (pad) return false; // data after padding within the group
                int d = decodeChar(c);
                if (d < 0) return false;
                q[k] = d;
            }
        }
        uint32_t n = (uint32_t(q[0]) << 18) | (uint32_t(q[1]) << 12) |
                     (uint32_t(q[2]) << 6) | uint32_t(q[3]);
        out.push_back(uint8_t((n >> 16) & 0xff));
        if (pad < 2) out.push_back(uint8_t((n >> 8) & 0xff));
        if (pad < 1) out.push_back(uint8_t(n & 0xff));
    }
    return true;
}

} // namespace b64

int main() {
    // Corpus of varying lengths to exercise all padding cases (0/1/2 leftover).
    std::vector<std::vector<uint8_t>> corpus;
    corpus.push_back({});                                  // empty
    corpus.push_back({'M'});                               // 1 byte  -> "TQ=="
    corpus.push_back({'M', 'a'});                          // 2 bytes -> 3 + pad
    corpus.push_back({'M', 'a', 'n'});                     // 3 bytes -> "TWFu"
    corpus.push_back({0x00, 0xff, 0x10, 0x80, 0x7f});      // binary mix
    std::vector<uint8_t> big;
    for (int i = 0; i < 130; ++i) big.push_back(uint8_t((i * 37 + 11) & 0xff));
    corpus.push_back(big);                                 // long binary run

    int roundTrips = 0;
    uint32_t acc = 2166136261u;
    for (const auto& msg : corpus) {
        std::string enc = b64::encode(msg);
        std::vector<uint8_t> dec;
        bool ok = b64::decode(enc, dec);
        if (ok && dec == msg) ++roundTrips;
        for (char c : enc) { acc ^= (uint8_t)c; acc *= 16777619u; }
    }

    int rejected = 0;   // negative tests: each malformed input must be rejected
    const char* bad[] = { "AB",        // length not multiple of 4
                          "A===",      // too much padding
                          "AB=C",      // data after padding
                          "@@@@",      // illegal characters
                          "====" };    // all padding
    std::vector<uint8_t> tmp;
    for (const char* s : bad)
        if (!b64::decode(std::string(s), tmp)) ++rejected;

    int score = roundTrips * 10 + rejected;
    return (int)((acc ^ (uint32_t)score) & 0x7f);
}
