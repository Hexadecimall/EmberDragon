// Open-addressing hash map (Robin Hood probing) with insert + lookup.
#include <vector>
#include <cstdint>
#include <cstdio>

struct RobinMap {
    struct Slot { uint64_t key; int val; int dist; bool used; };
    std::vector<Slot> slots;
    size_t count = 0;
    size_t mask;

    explicit RobinMap(size_t cap_pow2) : slots(cap_pow2), mask(cap_pow2 - 1) {
        for (auto &s : slots) { s.used = false; s.dist = 0; }
    }

    static uint64_t mix(uint64_t x) {           // splitmix64 finalizer
        x += 0x9E3779B97F4A7C15ULL;
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
        return x ^ (x >> 31);
    }

    void grow() {
        std::vector<Slot> old = std::move(slots);
        slots.assign(old.size() * 2, Slot{});
        for (auto &s : slots) { s.used = false; s.dist = 0; }
        mask = slots.size() - 1;
        count = 0;
        for (auto &s : old) if (s.used) insert(s.key, s.val);
    }

    void insert(uint64_t key, int val) {
        if ((count + 1) * 10 > slots.size() * 9) grow();   // load factor 0.9
        Slot cur{key, val, 0, true};
        size_t i = mix(key) & mask;
        while (true) {
            if (!slots[i].used) { slots[i] = cur; ++count; return; }
            if (slots[i].key == cur.key) { slots[i].val = cur.val; return; }
            if (slots[i].dist < cur.dist) {                // steal from the rich
                std::swap(slots[i], cur);
            }
            i = (i + 1) & mask;
            ++cur.dist;
        }
    }

    bool find(uint64_t key, int &out) const {
        size_t i = mix(key) & mask;
        int d = 0;
        while (slots[i].used) {
            if (slots[i].key == key) { out = slots[i].val; return true; }
            if (slots[i].dist < d) return false;           // would have stolen
            i = (i + 1) & mask;
            ++d;
        }
        return false;
    }
};

int main() {
    RobinMap m(8);
    const int N = 5000;
    // Insert key -> key*key (mod) deterministically.
    for (int k = 0; k < N; ++k) {
        uint64_t key = (uint64_t)k * 2654435761ULL;
        m.insert(key, (int)((key >> 13) & 0x7FFFFFFF));
    }
    // Overwrite a slice.
    for (int k = 0; k < 100; ++k) {
        uint64_t key = (uint64_t)k * 2654435761ULL;
        m.insert(key, -k);
    }
    long long checksum = 0;
    int hits = 0, misses = 0;
    for (int k = 0; k < N; ++k) {
        uint64_t key = (uint64_t)k * 2654435761ULL;
        int v;
        if (m.find(key, v)) { ++hits; checksum = checksum * 1000003 + v; }
        else ++misses;
    }
    // Probe keys that were never inserted.
    int phantom = 0;
    for (int k = N; k < N + 500; ++k) {
        uint64_t key = (uint64_t)k * 2654435761ULL;
        int v;
        if (m.find(key, v)) ++phantom;
    }
    printf("hits=%d misses=%d phantom=%d count=%zu checksum=%lld\n",
           hits, misses, phantom, m.count, checksum);
    return (hits == N && misses == 0 && phantom == 0) ? 0 : 1;
}
