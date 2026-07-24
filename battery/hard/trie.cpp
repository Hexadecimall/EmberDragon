// Lowercase-a..z trie with insert, exact membership, and prefix-count.
#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <cstdio>

struct Trie {
    struct Node {
        std::array<int, 26> next;   // child indices, -1 = none
        int passing = 0;            // words passing through this node
        int ending  = 0;            // words ending exactly here
        Node() { next.fill(-1); }
    };
    std::vector<Node> nodes;

    Trie() { nodes.emplace_back(); }   // root = index 0

    void insert(const std::string &w) {
        int cur = 0;
        for (char ch : w) {
            int c = ch - 'a';
            if (nodes[cur].next[c] < 0) {
                nodes[cur].next[c] = (int)nodes.size();
                nodes.emplace_back();
            }
            cur = nodes[cur].next[c];
            nodes[cur].passing++;
        }
        nodes[cur].ending++;
    }

    int count_exact(const std::string &w) const {
        int cur = 0;
        for (char ch : w) {
            int c = ch - 'a';
            int nx = nodes[cur].next[c];
            if (nx < 0) return 0;
            cur = nx;
        }
        return nodes[cur].ending;
    }

    int count_prefix(const std::string &p) const {
        int cur = 0;
        for (char ch : p) {
            int c = ch - 'a';
            int nx = nodes[cur].next[c];
            if (nx < 0) return 0;
            cur = nx;
        }
        return nodes[cur].passing;
    }
};

// Deterministic pseudo-random word generator (no I/O).
static std::string make_word(uint32_t seed) {
    uint32_t x = seed * 2654435761u + 1013904223u;
    int len = 2 + (x % 6);
    std::string s;
    for (int i = 0; i < len; ++i) {
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        s.push_back('a' + (x % 4));   // restrict to a..d so prefixes collide
    }
    return s;
}

int main() {
    Trie t;
    const int N = 3000;
    long long inserted_chars = 0;
    for (int i = 0; i < N; ++i) {
        std::string w = make_word(i);
        inserted_chars += (long long)w.size();
        t.insert(w);
    }
    // Every word starts with one of a,b,c,d -> the four prefix counts sum to N.
    int total = 0;
    for (char c = 'a'; c <= 'd'; ++c) {
        std::string p(1, c);
        total += t.count_prefix(p);
    }
    // Membership: each generated word must be found at least once.
    int present = 0;
    for (int i = 0; i < N; ++i) {
        if (t.count_exact(make_word(i)) >= 1) ++present;
    }
    // A prefix that cannot exist (we only used a..d).
    int none = t.count_prefix("zzz");
    printf("nodes=%zu total_prefix=%d present=%d none=%d chars=%lld\n",
           t.nodes.size(), total, present, none, inserted_chars);
    return (total == N && present == N && none == 0) ? 0 : 1;
}
