// BFS (explicit queue) and DFS (explicit stack + recursive) traversals over an
// adjacency matrix, plus shortest unweighted path via BFS parent tracking.
// Deterministic, no input, std lib only.
#include <vector>
#include <queue>
#include <stack>
#include <array>
#include <cassert>
#include <cstdio>

namespace {

// Dense graph backed by an adjacency matrix. Neighbors visited in index order
// so traversals are fully deterministic.
class MatrixGraph {
public:
    explicit MatrixGraph(int n) : n_(n), m_(n * n, 0) {}

    void connect(int u, int v) {  // undirected
        m_[u * n_ + v] = 1;
        m_[v * n_ + u] = 1;
    }
    void link(int u, int v) {     // directed
        m_[u * n_ + v] = 1;
    }
    bool edge(int u, int v) const { return m_[u * n_ + v] != 0; }
    int size() const { return n_; }

private:
    int n_;
    std::vector<uint8_t> m_;
};

std::vector<int> bfs_order(const MatrixGraph& g, int src) {
    const int n = g.size();
    std::vector<int> order;
    std::vector<char> seen(n, 0);
    std::queue<int> q;
    seen[src] = 1;
    q.push(src);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (int v = 0; v < n; ++v) {
            if (g.edge(u, v) && !seen[v]) {
                seen[v] = 1;
                q.push(v);
            }
        }
    }
    return order;
}

std::vector<int> dfs_iter_order(const MatrixGraph& g, int src) {
    const int n = g.size();
    std::vector<int> order;
    std::vector<char> seen(n, 0);
    std::stack<int> st;
    st.push(src);
    while (!st.empty()) {
        int u = st.top();
        st.pop();
        if (seen[u]) continue;
        seen[u] = 1;
        order.push_back(u);
        // Push in reverse so smallest index is processed first.
        for (int v = n - 1; v >= 0; --v)
            if (g.edge(u, v) && !seen[v]) st.push(v);
    }
    return order;
}

void dfs_rec(const MatrixGraph& g, int u, std::vector<char>& seen,
             std::vector<int>& order) {
    seen[u] = 1;
    order.push_back(u);
    for (int v = 0; v < g.size(); ++v)
        if (g.edge(u, v) && !seen[v]) dfs_rec(g, v, seen, order);
}

// Shortest unweighted path length from src to dst, or -1 if unreachable.
int bfs_dist(const MatrixGraph& g, int src, int dst) {
    const int n = g.size();
    std::vector<int> dist(n, -1);
    std::queue<int> q;
    dist[src] = 0;
    q.push(src);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (u == dst) return dist[u];
        for (int v = 0; v < n; ++v)
            if (g.edge(u, v) && dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
    }
    return dist[dst];
}

}  // namespace

int main() {
    //    0
    //   / \
    //  1   2
    //  |  / \
    //  3 4   5
    //   \|  /
    //    6
    MatrixGraph g(7);
    g.connect(0, 1);
    g.connect(0, 2);
    g.connect(1, 3);
    g.connect(2, 4);
    g.connect(2, 5);
    g.connect(3, 6);
    g.connect(4, 6);
    g.connect(5, 6);

    std::vector<int> bfs = bfs_order(g, 0);
    std::array<int, 7> bfs_expected = {0, 1, 2, 3, 4, 5, 6};
    assert(bfs.size() == 7);
    for (int i = 0; i < 7; ++i) assert(bfs[i] == bfs_expected[i]);

    std::vector<int> dfs_it = dfs_iter_order(g, 0);
    assert(dfs_it.size() == 7);
    assert(dfs_it.front() == 0);

    std::vector<char> seen(7, 0);
    std::vector<int> dfs_r;
    dfs_rec(g, 0, seen, dfs_r);
    std::array<int, 7> dfs_expected = {0, 1, 3, 6, 4, 2, 5};
    assert(dfs_r.size() == 7);
    for (int i = 0; i < 7; ++i) assert(dfs_r[i] == dfs_expected[i]);

    assert(bfs_dist(g, 0, 6) == 3);
    assert(bfs_dist(g, 0, 0) == 0);
    assert(bfs_dist(g, 1, 5) == 3);

    // A disconnected vertex is unreachable.
    MatrixGraph d(3);
    d.connect(0, 1);
    assert(bfs_dist(d, 0, 2) == -1);

    std::printf("bfs/dfs ok: bfs=%zu dfs_iter=%zu dfs_rec=%zu d(0,6)=%d\n",
                bfs.size(), dfs_it.size(), dfs_r.size(), bfs_dist(g, 0, 6));
    return 0;
}
