#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int row;
    int col;
} Point;

typedef struct {
    int32_t *pixels;
    int width;
    int height;
} Image;

typedef struct {
    Point *items;
    int count;
    int capacity;
} PointStack;

static int image_index(const Image *img, int row, int col) {
    return row * img->width + col;
}

int image_get(const Image *img, int row, int col) {
    if (row < 0 || row >= img->height) return -1;
    if (col < 0 || col >= img->width) return -1;
    return img->pixels[image_index(img, row, col)];
}

void image_set(Image *img, int row, int col, int32_t color) {
    if (row < 0 || row >= img->height) return;
    if (col < 0 || col >= img->width) return;
    img->pixels[image_index(img, row, col)] = color;
}

void stack_init(PointStack *stack, int capacity) {
    stack->items = (Point *)malloc(sizeof(Point) * capacity);
    stack->count = 0;
    stack->capacity = capacity;
}

int stack_push(PointStack *stack, int row, int col) {
    if (stack->count >= stack->capacity) return 0;
    stack->items[stack->count].row = row;
    stack->items[stack->count].col = col;
    stack->count++;
    return 1;
}

int stack_pop(PointStack *stack, Point *out) {
    if (stack->count == 0) return 0;
    stack->count--;
    *out = stack->items[stack->count];
    return 1;
}

int flood_fill(Image *img, int start_row, int start_col, int32_t new_color) {
    int32_t target = image_get(img, start_row, start_col);
    if (target == new_color) return 0;
    if (target < 0) return 0;

    PointStack stack;
    stack_init(&stack, img->width * img->height);
    stack_push(&stack, start_row, start_col);

    int filled = 0;
    Point current;
    while (stack_pop(&stack, &current)) {
        if (image_get(img, current.row, current.col) != target) continue;
        image_set(img, current.row, current.col, new_color);
        filled++;
        stack_push(&stack, current.row - 1, current.col);
        stack_push(&stack, current.row + 1, current.col);
        stack_push(&stack, current.row, current.col - 1);
        stack_push(&stack, current.row, current.col + 1);
    }
    free(stack.items);
    return filled;
}
