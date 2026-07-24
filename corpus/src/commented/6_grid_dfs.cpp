/*
 * Iterative depth-first reachability and connected-component labeling on a
 * 4-connected integer maze.
 *
 * A Maze is a grid of open/wall cells. This module answers "can I reach B from
 * A?" with an explicit-stack DFS (no recursion, so deep mazes won't blow the
 * call stack) and assigns a distinct region id to every connected blob of open
 * cells, which is handy for flood-style analysis.
 */

#include <cstdint>
#include <cstring>

/*
 * A maze grid. 'open[i]' is 1 where cell i is passable and 0 where it is a
 * wall. Cell (x, y) lives at index y * width + x. The backing array is owned
 * by the Maze and sized once at construction.
 */
class Maze {
public:
    /*
     * Construct a maze of the given dimensions with every cell open.
     * If width or height is non-positive, builds a 0x0 empty maze instead.
     */
    Maze(int width, int height)
        : width_(width > 0 ? width : 0),
          height_(height > 0 ? height : 0),
          open_(nullptr),
          stack_(nullptr) {
        int n = width_ * height_;
        if (n > 0) {
            open_ = new uint8_t[n];
            stack_ = new int[n];          // scratch stack, reused across calls
            std::memset(open_, 1, n);     // start fully passable
        }
    }

    /* Release the cell and scratch buffers. */
    ~Maze() {
        delete[] open_;
        delete[] stack_;
    }

    int width()  const { return width_; }
    int height() const { return height_; }

    /*
     * Set a cell to wall (passable = false) or open (passable = true).
     * Out-of-bounds coordinates are ignored. O(1).
     */
    void setWall(int x, int y, bool wall) {
        if (!inBounds(x, y)) return;
        open_[y * width_ + x] = wall ? 0 : 1;
    }

    /*
     * Report whether two cells are connected through open cells using a
     * 4-connected iterative DFS from the source.
     *
     * Returns true if (tx, ty) is reachable from (sx, sy); false if either
     * endpoint is a wall or off the grid, or if no open path connects them.
     * Runs in O(width * height); uses the preallocated scratch stack and a
     * local visited array, so it allocates nothing on the hot path beyond that.
     */
    bool reachable(int sx, int sy, int tx, int ty) const {
        if (!isOpen(sx, sy) || !isOpen(tx, ty)) {
            return false;  // a walled or off-board endpoint is never reachable
        }
        int n = width_ * height_;
        // visited[i] guards against re-pushing a cell and looping forever.
        uint8_t *visited = new uint8_t[n];
        std::memset(visited, 0, n);

        int top = 0;
        int start = sy * width_ + sx;
        int goal  = ty * width_ + tx;
        stack_[top++] = start;
        visited[start] = 1;

        bool found = false;
        while (top > 0 && !found) {
            int cur = stack_[--top];     // LIFO pop: this is what makes it DFS
            if (cur == goal) {
                found = true;
                break;
            }
            int cx = cur % width_;
            int cy = cur / width_;
            static const int DX[4] = { 1, -1, 0, 0 };
            static const int DY[4] = { 0, 0, 1, -1 };
            for (int d = 0; d < 4; ++d) {
                int nx = cx + DX[d];
                int ny = cy + DY[d];
                if (!isOpen(nx, ny)) continue;
                int ni = ny * width_ + nx;
                if (visited[ni]) continue;  // already on/through the stack
                visited[ni] = 1;
                stack_[top++] = ni;
            }
        }

        delete[] visited;
        return found;
    }

    /*
     * Label every open cell with the id of the connected component it belongs
     * to. 'labels' must point to width*height ints supplied by the caller; on
     * return, walls are labeled 0 and each open region gets a unique id 1, 2,
     * 3, ... assigned in row-major discovery order.
     *
     * Returns the number of distinct open components found. O(width * height).
     */
    int labelComponents(int *labels) const {
        int n = width_ * height_;
        for (int i = 0; i < n; ++i) {
            labels[i] = 0;  // 0 means "unlabeled wall or not-yet-seen open cell"
        }

        int nextId = 0;
        static const int DX[4] = { 1, -1, 0, 0 };
        static const int DY[4] = { 0, 0, 1, -1 };

        for (int i = 0; i < n; ++i) {
            // Seed a new component only at an open, still-unlabeled cell.
            if (open_[i] == 0 || labels[i] != 0) {
                continue;
            }
            ++nextId;
            int top = 0;
            stack_[top++] = i;
            labels[i] = nextId;

            // Flood this component out via DFS, stamping the same id everywhere.
            while (top > 0) {
                int cur = stack_[--top];
                int cx = cur % width_;
                int cy = cur / width_;
                for (int d = 0; d < 4; ++d) {
                    int nx = cx + DX[d];
                    int ny = cy + DY[d];
                    if (!isOpen(nx, ny)) continue;
                    int ni = ny * width_ + nx;
                    if (labels[ni] != 0) continue;
                    labels[ni] = nextId;
                    stack_[top++] = ni;
                }
            }
        }
        return nextId;
    }

private:
    /* True if (x, y) is a valid grid coordinate. */
    bool inBounds(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    /* True if (x, y) is in bounds and passable. */
    bool isOpen(int x, int y) const {
        return inBounds(x, y) && open_[y * width_ + x] != 0;
    }

    int      width_;
    int      height_;
    uint8_t *open_;   // width*height passability flags, owned
    int     *stack_;  // reusable DFS scratch stack, owned
};
