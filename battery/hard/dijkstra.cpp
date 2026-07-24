// Dijkstra shortest path on an adjacency-list weighted graph using a binary heap.
// Self-contained, deterministic, no I/O input. Builds a fixed graph, runs Dijkstra
// from a source, reconstructs a path, and exercises results via assertions.
#include <vector>
#include <queue>
#include <limits>
#include <cstdint>
#include <cassert>
#include <cstdio>

namespace {

struct Edge {
    int to;
    int64_t w;
};

class Graph {
public:
    explicit Graph(int n) : adj_(n) {}

    void add_directed(int u, int v, int64_t w) {
        adj_[u].push_back(Edge{v, w});
    }
    void add_undirected(int u, int v, int64_t w) {
        add_directed(u, v, w);
        add_directed(v, u, w);
    }

    int size() const { return static_cast<int>(adj_.size()); }
    const std::vector<Edge>& neighbors(int u) const { return adj_[u]; }

private:
    std::vector<std::vector<Edge>> adj_;
};

constexpr int64_t kInf = std::numeric_limits<int64_t>::max();

struct HeapItem {
    int64_t dist;
    int node;
    bool operator>(const HeapItem& o) const { return dist > o.dist; }
};
// Returns dist[] and prev[] (parent in shortest-path tree, -1 if none).
void dijkstra(const Graph& g, int src,
              std::vector<int64_t>& dist, std::vector<int>& prev) {
    const int n = g.size();
    dist.assign(n, kInf);
    prev.assign(n, -1);
    std::priority_queue<HeapItem, std::vector<HeapItem>, std::greater<HeapItem>> pq;

    dist[src] = 0;
    pq.push(HeapItem{0, src});

    while (!pq.empty()) {
        HeapItem cur = pq.top();
        pq.pop();
        if (cur.dist > dist[cur.node]) continue;  // stale entry, skip
        for (const Edge& e : g.neighbors(cur.node)) {
            if (dist[cur.node] == kInf) continue;
            int64_t nd = cur.dist + e.w;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = cur.node;
                pq.push(HeapItem{nd, e.to});
            }
        }
    }
}

std::vector<int> rebuild_path(const std::vector<int>& prev, int dst) {
    std::vector<int> path;
    for (int at = dst; at != -1; at = prev[at]) path.push_back(at);
    return std::vector<int>(path.rbegin(), path.rend());
}

}  // namespace

int main() {
    Graph g(6);
    g.add_undirected(0, 1, 7);
    g.add_undirected(0, 2, 9);
    g.add_undirected(0, 5, 14);
    g.add_undirected(1, 2, 10);
    g.add_undirected(1, 3, 15);
    g.add_undirected(2, 3, 11);
    g.add_undirected(2, 5, 2);
    g.add_undirected(3, 4, 6);
    g.add_undirected(4, 5, 9);

    std::vector<int64_t> dist;
    std::vector<int> prev;
    dijkstra(g, 0, dist, prev);

    assert(dist[0] == 0);
    assert(dist[1] == 7);
    assert(dist[2] == 9);
    assert(dist[3] == 20);
    assert(dist[4] == 20);
    assert(dist[5] == 11);

    std::vector<int> path = rebuild_path(prev, 4);
    assert(path.front() == 0 && path.back() == 4);

    int64_t walked = 0;
    for (size_t i = 1; i < path.size(); ++i) {
        int u = path[i - 1], v = path[i];
        int64_t step = kInf;
        for (const Edge& e : g.neighbors(u))
            if (e.to == v) step = std::min(step, e.w);
        assert(step != kInf);
        walked += step;
    }
    assert(walked == dist[4]);

    std::printf("dijkstra ok: dist[4]=%lld path_len=%zu\n",
                static_cast<long long>(dist[4]), path.size());
    return 0;
}
