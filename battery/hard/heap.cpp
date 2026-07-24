// Binary min-heap priority queue with decrease-key support via index map.
#include <vector>
#include <cstdint>
#include <cstdio>

struct MinHeap {
    struct Node { int key; int prio; };
    std::vector<Node> data;          // 1-based heap array
    std::vector<int>  pos;           // key -> index in data (-1 if absent)

    explicit MinHeap(int capacity) : pos(capacity, -1) {
        data.push_back({-1, 0});     // sentinel slot 0
    }

    bool empty() const { return data.size() <= 1; }
    int  size()  const { return static_cast<int>(data.size()) - 1; }

    void swap_nodes(int a, int b) {
        std::swap(data[a], data[b]);
        pos[data[a].key] = a;
        pos[data[b].key] = b;
    }

    void sift_up(int i) {
        while (i > 1 && data[i].prio < data[i / 2].prio) {
            swap_nodes(i, i / 2);
            i /= 2;
        }
    }

    void sift_down(int i) {
        int n = static_cast<int>(data.size());
        while (true) {
            int l = 2 * i, r = 2 * i + 1, m = i;
            if (l < n && data[l].prio < data[m].prio) m = l;
            if (r < n && data[r].prio < data[m].prio) m = r;
            if (m == i) break;
            swap_nodes(i, m);
            i = m;
        }
    }

    void push(int key, int prio) {
        data.push_back({key, prio});
        int i = static_cast<int>(data.size()) - 1;
        pos[key] = i;
        sift_up(i);
    }

    void decrease(int key, int new_prio) {
        int i = pos[key];
        if (i < 0 || new_prio >= data[i].prio) return;
        data[i].prio = new_prio;
        sift_up(i);
    }

    int pop_min() {
        int key = data[1].key;
        pos[key] = -1;
        int last = static_cast<int>(data.size()) - 1;
        if (last > 1) {
            swap_nodes(1, last);
            data.pop_back();
            sift_down(1);
        } else {
            data.pop_back();
        }
        return key;
    }
};

int main() {
    MinHeap h(16);
    int prios[16] = {50, 13, 27, 99, 4, 61, 8, 7, 42, 31, 19, 2, 88, 71, 33, 60};
    for (int k = 0; k < 16; ++k) h.push(k, prios[k]);

    // Decrease a few keys so the heap reorders.
    h.decrease(3, 1);    // key 3 jumps to the front
    h.decrease(13, 5);
    h.decrease(8, 6);

    long long checksum = 0;
    int last_prio = -1000000;
    bool sorted = true;
    while (!h.empty()) {
        int key = h.pop_min();
        int p = prios[key];
        if (key == 3)  p = 1;
        if (key == 13) p = 5;
        if (key == 8)  p = 6;
        if (p < last_prio) sorted = false;
        last_prio = p;
        checksum = checksum * 31 + key * 100 + p;
    }
    printf("sorted=%d checksum=%lld size=%d\n", sorted ? 1 : 0, checksum, h.size());
    return sorted ? 0 : 1;
}
