// AVL tree with insert, erase, in-order verification, and height balance check.
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

struct AVL {
    struct Node {
        int key;
        int height = 1;
        int left = -1, right = -1;
    };
    std::vector<Node> pool;
    int root = -1;

    int h(int n) const { return n < 0 ? 0 : pool[n].height; }
    int bf(int n) const { return n < 0 ? 0 : h(pool[n].left) - h(pool[n].right); }

    void fix(int n) {
        int hl = h(pool[n].left), hr = h(pool[n].right);
        pool[n].height = 1 + (hl > hr ? hl : hr);
    }

    int rot_right(int y) {
        int x = pool[y].left;
        pool[y].left = pool[x].right;
        pool[x].right = y;
        fix(y); fix(x);
        return x;
    }

    int rot_left(int x) {
        int y = pool[x].right;
        pool[x].right = pool[y].left;
        pool[y].left = x;
        fix(x); fix(y);
        return y;
    }

    int rebalance(int n) {
        fix(n);
        int b = bf(n);
        if (b > 1) {
            if (bf(pool[n].left) < 0) pool[n].left = rot_left(pool[n].left);
            return rot_right(n);
        }
        if (b < -1) {
            if (bf(pool[n].right) > 0) pool[n].right = rot_right(pool[n].right);
            return rot_left(n);
        }
        return n;
    }

    int insert(int n, int key) {
        if (n < 0) {
            pool.push_back({key, 1, -1, -1});
            return (int)pool.size() - 1;
        }
        if (key < pool[n].key) pool[n].left = insert(pool[n].left, key);
        else if (key > pool[n].key) pool[n].right = insert(pool[n].right, key);
        else return n;
        return rebalance(n);
    }

    int min_node(int n) const { while (pool[n].left >= 0) n = pool[n].left; return n; }

    int erase(int n, int key) {
        if (n < 0) return -1;
        if (key < pool[n].key) pool[n].left = erase(pool[n].left, key);
        else if (key > pool[n].key) pool[n].right = erase(pool[n].right, key);
        else {
            if (pool[n].left < 0) return pool[n].right;
            if (pool[n].right < 0) return pool[n].left;
            int m = min_node(pool[n].right);
            pool[n].key = pool[m].key;
            pool[n].right = erase(pool[n].right, pool[m].key);
        }
        return rebalance(n);
    }

    void insert(int key) { root = insert(root, key); }
    void erase(int key)  { root = erase(root, key); }

    // In-order traversal collecting keys; also asserts AVL balance invariant.
    bool check(int n, std::vector<int> &out) const {
        if (n < 0) return true;
        if (!check(pool[n].left, out)) return false;
        out.push_back(pool[n].key);
        if (!check(pool[n].right, out)) return false;
        int b = bf(n);
        return b >= -1 && b <= 1;
    }
};

int main() {
    AVL tree;
    const int N = 2000;
    // Deterministic shuffle-ish insertion order.
    uint32_t x = 123456789u;
    for (int i = 0; i < N; ++i) {
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        tree.insert((int)(x % 100000));
    }
    // Erase every value below 50000.
    uint32_t y = 123456789u;
    for (int i = 0; i < N; ++i) {
        y ^= y << 13; y ^= y >> 17; y ^= y << 5;
        int k = (int)(y % 100000);
        if (k < 50000) tree.erase(k);
    }
    std::vector<int> keys;
    bool balanced = tree.check(tree.root, keys);
    bool ordered = true;
    long long checksum = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i && keys[i] <= keys[i - 1]) ordered = false;
        checksum = checksum * 1000003 + keys[i];
    }
    bool no_low = true;
    for (int k : keys) if (k < 50000) no_low = false;
    printf("balanced=%d ordered=%d no_low=%d remaining=%zu checksum=%lld\n",
           balanced, ordered, no_low, keys.size(), checksum);
    return (balanced && ordered && no_low) ? 0 : 1;
}
