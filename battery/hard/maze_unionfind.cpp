// Weighted union-find (disjoint set) with path compression, used two ways:
//   1) connected components of an abstract graph,
//   2) a grid-maze solver: cells are nodes, open neighbors are unioned, and we
//      test start/goal connectivity, then BFS for the actual shortest path.
// Deterministic, no input, std lib only.
#include <vector>
#include <queue>
#include <array>
#include <string>
#include <cassert>
#include <cstdio>

namespace {

class DisjointSet {
public:
    explicit DisjointSet(int n) : parent_(n), rank_(n, 0), count_(n) {
        for (int i = 0; i < n; ++i) parent_[i] = i;
    }

    int find(int x) {
        while (parent_[x] != x) {
            parent_[x] = parent_[parent_[x]];  // path halving
            x = parent_[x];
        }
        return x;
    }

    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rank_[ra] < rank_[rb]) std::swap(ra, rb);
        parent_[rb] = ra;
        if (rank_[ra] == rank_[rb]) ++rank_[ra];
        --count_;
        return true;
    }

    bool connected(int a, int b) { return find(a) == find(b); }
    int components() const { return count_; }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
    int count_;
};

struct Maze {
    std::vector<std::string> grid;  // '#' wall, '.' open, 'S' start, 'G' goal
    int rows() const { return static_cast<int>(grid.size()); }
    int cols() const { return static_cast<int>(grid[0].size()); }
    int id(int r, int c) const { return r * cols() + c; }
    bool open(int r, int c) const {
        return r >= 0 && r < rows() && c >= 0 && c < cols() && grid[r][c] != '#';
    }
    void find_marker(char m, int& r, int& c) const {
        for (int i = 0; i < rows(); ++i)
            for (int j = 0; j < cols(); ++j)
                if (grid[i][j] == m) { r = i; c = j; return; }
        r = c = -1;
    }
};

// BFS shortest path length through open cells; -1 if no path.
int maze_bfs(const Maze& mz, int sr, int sc, int gr, int gc) {
    const int R = mz.rows(), C = mz.cols();
    std::vector<int> dist(R * C, -1);
    std::queue<std::array<int, 2>> q;
    dist[mz.id(sr, sc)] = 0;
    q.push({sr, sc});
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};
    while (!q.empty()) {
        auto cur = q.front();
        q.pop();
        int r = cur[0], c = cur[1];
        if (r == gr && c == gc) return dist[mz.id(r, c)];
        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k], nc = c + dc[k];
            if (mz.open(nr, nc) && dist[mz.id(nr, nc)] == -1) {
                dist[mz.id(nr, nc)] = dist[mz.id(r, c)] + 1;
                q.push({nr, nc});
            }
        }
    }
    return -1;
}

// Build a DisjointSet over all open cells, unioning orthogonally adjacent ones.
bool maze_reachable(const Maze& mz, int sr, int sc, int gr, int gc) {
    DisjointSet ds(mz.rows() * mz.cols());
    for (int r = 0; r < mz.rows(); ++r)
        for (int c = 0; c < mz.cols(); ++c) {
            if (!mz.open(r, c)) continue;
            if (mz.open(r + 1, c)) ds.unite(mz.id(r, c), mz.id(r + 1, c));
            if (mz.open(r, c + 1)) ds.unite(mz.id(r, c), mz.id(r, c + 1));
        }
    return ds.connected(mz.id(sr, sc), mz.id(gr, gc));
}

}  // namespace

int main() {
    // Part 1: union-find connected components on an abstract graph.
    DisjointSet ds(10);
    ds.unite(0, 1);
    ds.unite(1, 2);
    ds.unite(3, 4);
    ds.unite(5, 6);
    ds.unite(6, 7);
    ds.unite(7, 5);          // redundant, already same set
    assert(ds.components() == 5);  // {0,1,2}{3,4}{5,6,7}{8}{9}
    assert(ds.connected(0, 2));
    assert(!ds.connected(0, 3));
    ds.unite(2, 3);
    assert(ds.connected(0, 4));
    assert(ds.components() == 4);

    // Part 2: maze solver.
    // A solvable boustrophedon (snake) corridor: full rows joined by single
    // open columns. Shortest path winds the whole grid.
    Maze mz;
    mz.grid = {
        "S........",
        "#######.#",
        ".........",
        ".#######.",
        ".........",
        ".#######.",
        "........G",
    };
    int sr, sc, gr, gc;
    mz.find_marker('S', sr, sc);
    mz.find_marker('G', gr, gc);
    assert(sr == 0 && sc == 0);
    assert(gr == 6 && gc == 8);

    bool reach = maze_reachable(mz, sr, sc, gr, gc);
    int d = maze_bfs(mz, sr, sc, gr, gc);
    assert(reach == (d != -1));
    assert(reach);          // this maze is solvable
    assert(d == 14);        // unique shortest snake path length

    // A walled-off maze: goal isolated behind solid walls.
    Maze blocked;
    blocked.grid = {
        "S.#.G",
        "..#..",
        "..#..",
    };
    int br, bc, bgr, bgc;
    blocked.find_marker('S', br, bc);
    blocked.find_marker('G', bgr, bgc);
    assert(!maze_reachable(blocked, br, bc, bgr, bgc));
    assert(maze_bfs(blocked, br, bc, bgr, bgc) == -1);

    std::printf("maze/union-find ok: comps=%d reach=%d dist=%d\n",
                ds.components(), reach ? 1 : 0, d);
    return 0;
}
