#include <stdint.h>
#include <string.h>

#define MAZE_DIM 20

typedef struct {
    uint8_t blocked[MAZE_DIM][MAZE_DIM];
} Maze;

typedef struct {
    int16_t row;
    int16_t col;
    int32_t depth;
} Frontier;

typedef struct {
    Frontier slots[MAZE_DIM * MAZE_DIM];
    int head;
    int tail;
} Queue;

void maze_open_all(Maze *maze) {
    memset(maze->blocked, 0, sizeof(maze->blocked));
}

void maze_add_wall(Maze *maze, int row, int col) {
    if (row < 0 || row >= MAZE_DIM) return;
    if (col < 0 || col >= MAZE_DIM) return;
    maze->blocked[row][col] = 1;
}

static int maze_passable(const Maze *maze, int row, int col) {
    if (row < 0 || row >= MAZE_DIM) return 0;
    if (col < 0 || col >= MAZE_DIM) return 0;
    return maze->blocked[row][col] == 0;
}

static void queue_reset(Queue *queue) {
    queue->head = 0;
    queue->tail = 0;
}

static void queue_push(Queue *queue, int row, int col, int depth) {
    queue->slots[queue->tail].row = (int16_t)row;
    queue->slots[queue->tail].col = (int16_t)col;
    queue->slots[queue->tail].depth = depth;
    queue->tail++;
}

static int queue_pop(Queue *queue, Frontier *out) {
    if (queue->head == queue->tail) return 0;
    *out = queue->slots[queue->head];
    queue->head++;
    return 1;
}

int maze_distance(const Maze *maze, int startRow, int startCol,
                  int goalRow, int goalCol) {
    uint8_t seen[MAZE_DIM][MAZE_DIM];
    memset(seen, 0, sizeof(seen));

    Queue queue;
    queue_reset(&queue);

    if (!maze_passable(maze, startRow, startCol)) return -1;
    seen[startRow][startCol] = 1;
    queue_push(&queue, startRow, startCol, 0);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    Frontier node;
    while (queue_pop(&queue, &node)) {
        if (node.row == goalRow && node.col == goalCol)
            return node.depth;
        for (int k = 0; k < 4; k++) {
            int nr = node.row + dr[k];
            int nc = node.col + dc[k];
            if (!maze_passable(maze, nr, nc)) continue;
            if (seen[nr][nc]) continue;
            seen[nr][nc] = 1;
            queue_push(&queue, nr, nc, node.depth + 1);
        }
    }
    return -1;
}
