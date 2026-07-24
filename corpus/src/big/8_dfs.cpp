/*
 * dfs.cpp
 *
 * Depth-first search and connected-component labelling on an undirected graph.
 * The traversal is written iteratively with an explicit stack so it does not
 * risk blowing the call stack on deep or pathological graphs.
 */

#include <cstdlib>

/*
 * An undirected graph backed by per-vertex neighbour vectors implemented by
 * hand (no STL containers) so the binary stays small and self-contained.
 */
class Graph {
public:
    /*
     * Construct a graph with `vertexCount` vertices and no edges.
     * Allocates one growable neighbour list per vertex.
     */
    explicit Graph(int vertexCount)
        : count_(vertexCount) {
        lists_ = new NeighborList[vertexCount];
    }

    /* Destructor: release every per-vertex neighbour buffer. */
    ~Graph() {
        delete[] lists_;
    }

    /* Number of vertices in the graph. */
    int size() const { return count_; }

    /*
     * Add an undirected edge between a and b. Out-of-range endpoints are
     * silently ignored so callers can build graphs without per-call checks.
     */
    void addEdge(int a, int b) {
        if (a < 0 || a >= count_ || b < 0 || b >= count_) {
            return;
        }
        lists_[a].push(b);
        lists_[b].push(a);
    }

    /*
     * Iterative depth-first search from `start`. Marks every vertex reachable
     * from start as visited in the caller-supplied `visited` array (length
     * size()). Returns the number of vertices newly reached. O(V + E).
     */
    int depthFirst(int start, bool *visited) const {
        if (start < 0 || start >= count_) {
            return 0;
        }
        int *stack = new int[count_];
        int top = 0;
        int reached = 0;

        stack[top++] = start;
        while (top > 0) {
            int v = stack[--top];   /* pop the next vertex to expand */
            if (visited[v]) {
                continue;           /* may have been queued more than once */
            }
            visited[v] = true;
            reached++;
            /* Push every not-yet-visited neighbour for later expansion. */
            const NeighborList &nl = lists_[v];
            for (int i = 0; i < nl.length; i++) {
                if (!visited[nl.data[i]]) {
                    stack[top++] = nl.data[i];
                }
            }
        }

        delete[] stack;
        return reached;
    }

    /*
     * Label connected components. Writes into `component` (length size()) a
     * component id in [0, k) for each vertex, where vertices in the same
     * component share an id. Returns k, the number of components. O(V + E).
     */
    int connectedComponents(int *component) const {
        bool *visited = new bool[count_];
        for (int v = 0; v < count_; v++) {
            visited[v] = false;
            component[v] = -1;
        }

        int label = 0;
        for (int v = 0; v < count_; v++) {
            if (visited[v]) {
                continue;   /* already absorbed into an earlier component */
            }
            /* Each unvisited vertex starts a fresh component flood-fill. */
            floodFill(v, label, visited, component);
            label++;
        }

        delete[] visited;
        return label;
    }

private:
    /* A hand-rolled growable array of neighbour vertex ids. */
    struct NeighborList {
        int *data = nullptr;
        int length = 0;
        int capacity = 0;

        /* Append one neighbour, doubling capacity when the buffer is full. */
        void push(int value) {
            if (length == capacity) {
                int newCap = capacity == 0 ? 4 : capacity * 2;
                int *grown = new int[newCap];
                for (int i = 0; i < length; i++) {
                    grown[i] = data[i];
                }
                delete[] data;
                data = grown;
                capacity = newCap;
            }
            data[length++] = value;
        }

        ~NeighborList() { delete[] data; }
    };

    /*
     * Iterative flood fill assigning `label` to the whole component of `seed`.
     * Shared by connectedComponents; mirrors depthFirst but also records ids.
     */
    void floodFill(int seed, int label, bool *visited, int *component) const {
        int *stack = new int[count_];
        int top = 0;
        stack[top++] = seed;
        while (top > 0) {
            int v = stack[--top];
            if (visited[v]) {
                continue;
            }
            visited[v] = true;
            component[v] = label;
            const NeighborList &nl = lists_[v];
            for (int i = 0; i < nl.length; i++) {
                if (!visited[nl.data[i]]) {
                    stack[top++] = nl.data[i];
                }
            }
        }
        delete[] stack;
    }

    int count_;            /* number of vertices */
    NeighborList *lists_;  /* one neighbour list per vertex */
};
