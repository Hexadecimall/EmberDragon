// Topological sort of a DAG via Kahn's algorithm (BFS on indegrees) and via
// DFS post-order, with cycle detection. Also computes longest-path layering.
// Deterministic, no input, std lib only.
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <cstdio>

namespace {

class DAG {
public:
    explicit DAG(int n) : adj_(n), indeg_(n, 0) {}

    void add_edge(int u, int v) {
        adj_[u].push_back(v);
        ++indeg_[v];
    }
    int size() const { return static_cast<int>(adj_.size()); }
    const std::vector<int>& succ(int u) const { return adj_[u]; }
    int indegree(int u) const { return indeg_[u]; }

private:
    std::vector<std::vector<int>> adj_;
    std::vector<int> indeg_;
};

// Kahn: repeatedly pop a zero-indegree node. Uses a min-heap on node id so the
// output is the lexicographically smallest valid topo order (deterministic).
// Returns false if a cycle is present (fewer than n nodes emitted).
bool kahn(const DAG& g, std::vector<int>& order) {
    const int n = g.size();
    std::vector<int> indeg(n);
    for (int i = 0; i < n; ++i) indeg[i] = g.indegree(i);

    std::priority_queue<int, std::vector<int>, std::greater<int>> ready;
    for (int i = 0; i < n; ++i)
        if (indeg[i] == 0) ready.push(i);

    order.clear();
    while (!ready.empty()) {
        int u = ready.top();
        ready.pop();
        order.push_back(u);
        for (int v : g.succ(u))
            if (--indeg[v] == 0) ready.push(v);
    }
    return static_cast<int>(order.size()) == n;
}

// DFS post-order topo sort with 3-color cycle detection.
class DfsTopo {
public:
    explicit DfsTopo(const DAG& g) : g_(g), color_(g.size(), 0) {}

    bool run(std::vector<int>& order) {
        order_.clear();
        for (int i = 0; i < g_.size(); ++i)
            if (color_[i] == 0 && !visit(i)) return false;
        std::reverse(order_.begin(), order_.end());
        order = order_;
        return true;
    }

private:
    bool visit(int u) {
        color_[u] = 1;  // gray = on stack
        for (int v : g_.succ(u)) {
            if (color_[v] == 1) return false;        // back edge -> cycle
            if (color_[v] == 0 && !visit(v)) return false;
        }
        color_[u] = 2;  // black = done
        order_.push_back(u);
        return true;
    }

    const DAG& g_;
    std::vector<int> color_;
    std::vector<int> order_;
};

// Longest path length (in edges) ending at each node, processed in topo order.
std::vector<int> layer_depths(const DAG& g, const std::vector<int>& topo) {
    std::vector<int> depth(g.size(), 0);
    for (int u : topo)
        for (int v : g.succ(u))
            depth[v] = std::max(depth[v], depth[u] + 1);
    return depth;
}

bool is_valid_topo(const DAG& g, const std::vector<int>& order) {
    std::vector<int> pos(g.size(), -1);
    for (int i = 0; i < static_cast<int>(order.size()); ++i) pos[order[i]] = i;
    for (int u = 0; u < g.size(); ++u)
        for (int v : g.succ(u))
            if (pos[u] >= pos[v]) return false;
    return true;
}

}  // namespace

int main() {
    // Classic course-prereq style DAG.
    DAG g(8);
    g.add_edge(0, 3);
    g.add_edge(0, 4);
    g.add_edge(1, 3);
    g.add_edge(2, 4);
    g.add_edge(2, 7);
    g.add_edge(3, 5);
    g.add_edge(3, 6);
    g.add_edge(4, 6);
    g.add_edge(5, 7);
    g.add_edge(6, 7);

    std::vector<int> kahn_order, dfs_order;
    bool k_ok = kahn(g, kahn_order);
    DfsTopo dt(g);
    bool d_ok = dt.run(dfs_order);

    assert(k_ok && d_ok);
    assert(kahn_order.size() == 8);
    assert(dfs_order.size() == 8);
    assert(is_valid_topo(g, kahn_order));
    assert(is_valid_topo(g, dfs_order));

    // Lexicographically smallest order is uniquely determined.
    std::vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7};
    for (int i = 0; i < 8; ++i) assert(kahn_order[i] == expected[i]);

    std::vector<int> depth = layer_depths(g, kahn_order);
    assert(depth[0] == 0);
    assert(depth[7] == 3);  // longest chain to 7 is 3 edges, e.g. 0->3->6->7

    // Cycle must be detected by both methods.
    DAG cyc(3);
    cyc.add_edge(0, 1);
    cyc.add_edge(1, 2);
    cyc.add_edge(2, 0);
    std::vector<int> tmp;
    DfsTopo dc(cyc);
    assert(!kahn(cyc, tmp));
    assert(!dc.run(tmp));

    std::printf("toposort ok: n=%zu depth[7]=%d\n", kahn_order.size(), depth[7]);
    return 0;
}
