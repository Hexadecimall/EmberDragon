#include <stdint.h>
#include <stdlib.h>

class WeightedGrid {
public:
    static const int kSize = 12;
    static const int kInfinity = 1000000;

    WeightedGrid() {
        for (int row = 0; row < kSize; row++)
            for (int col = 0; col < kSize; col++)
                weight_[row][col] = 1;
    }

    void setWeight(int row, int col, int cost) {
        if (inBounds(row, col)) weight_[row][col] = cost;
    }

    bool inBounds(int row, int col) const {
        return row >= 0 && row < kSize && col >= 0 && col < kSize;
    }

    int cellWeight(int row, int col) const {
        return weight_[row][col];
    }

private:
    int weight_[kSize][kSize];
};

struct SearchState {
    int dist[WeightedGrid::kSize][WeightedGrid::kSize];
    bool visited[WeightedGrid::kSize][WeightedGrid::kSize];
};

static void resetState(SearchState *state) {
    for (int row = 0; row < WeightedGrid::kSize; row++) {
        for (int col = 0; col < WeightedGrid::kSize; col++) {
            state->dist[row][col] = WeightedGrid::kInfinity;
            state->visited[row][col] = false;
        }
    }
}

static bool pickClosest(const SearchState *state, int *outRow, int *outCol) {
    int best = WeightedGrid::kInfinity;
    int bestRow = -1;
    int bestCol = -1;
    for (int row = 0; row < WeightedGrid::kSize; row++) {
        for (int col = 0; col < WeightedGrid::kSize; col++) {
            if (state->visited[row][col]) continue;
            if (state->dist[row][col] < best) {
                best = state->dist[row][col];
                bestRow = row;
                bestCol = col;
            }
        }
    }
    if (bestRow < 0) return false;
    *outRow = bestRow;
    *outCol = bestCol;
    return true;
}

int shortestPathCost(const WeightedGrid &grid, int startRow, int startCol,
                     int goalRow, int goalCol) {
    SearchState state;
    resetState(&state);
    state.dist[startRow][startCol] = grid.cellWeight(startRow, startCol);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    int row, col;
    while (pickClosest(&state, &row, &col)) {
        state.visited[row][col] = true;
        if (row == goalRow && col == goalCol)
            return state.dist[row][col];
        for (int k = 0; k < 4; k++) {
            int nr = row + dr[k];
            int nc = col + dc[k];
            if (!grid.inBounds(nr, nc)) continue;
            if (state.visited[nr][nc]) continue;
            int candidate = state.dist[row][col] + grid.cellWeight(nr, nc);
            if (candidate < state.dist[nr][nc])
                state.dist[nr][nc] = candidate;
        }
    }
    return -1;
}
