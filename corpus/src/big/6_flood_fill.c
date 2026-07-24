/*
 * flood_fill.c -- Iterative four-connected flood fill on an integer raster.
 *
 * Given a rectangular grid of colour values, replace a contiguous region of one
 * colour (reachable from a seed pixel by up/down/left/right moves) with a new
 * colour. The fill is implemented with an explicit stack rather than recursion
 * so it cannot overflow the call stack on large connected regions.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * A Raster is a heap-allocated 2D buffer of 32-bit colour indices stored
 * row-major. Pixels are addressed as data[y * width + x].
 */
typedef struct {
    int      width;   /* number of columns */
    int      height;  /* number of rows    */
    int32_t *data;    /* width*height colour cells, owned by the Raster */
} Raster;

/*
 * Allocate a Raster of the given size with every pixel initialised to `fill`.
 * Returns NULL if the dimensions are non-positive or allocation fails. The
 * caller owns the result and must release it with raster_destroy. O(w*h).
 */
Raster *raster_create(int width, int height, int32_t fill) {
    if (width <= 0 || height <= 0)
        return NULL;
    Raster *r = (Raster *)malloc(sizeof(Raster));
    if (!r)
        return NULL;
    r->width = width;
    r->height = height;
    r->data = (int32_t *)malloc((size_t)width * height * sizeof(int32_t));
    if (!r->data) {
        free(r);          /* avoid leaking the struct if the buffer fails */
        return NULL;
    }
    for (int i = 0; i < width * height; i++)
        r->data[i] = fill;
    return r;
}

/*
 * Free a Raster and its pixel buffer. Safe to call with NULL. O(1).
 */
void raster_destroy(Raster *r) {
    if (!r)
        return;
    free(r->data);
    free(r);
}

/*
 * Return the colour at (x, y), or -1 if the coordinate lies outside the raster.
 * Centralising the bounds check keeps the fill loop readable. O(1).
 */
int32_t raster_get(const Raster *r, int x, int y) {
    if (x < 0 || y < 0 || x >= r->width || y >= r->height)
        return -1;
    return r->data[y * r->width + x];
}

/*
 * A Point is a single pending coordinate on the explicit fill stack.
 */
typedef struct {
    int x;
    int y;
} Point;

/*
 * Flood fill starting at (start_x, start_y), replacing the connected region of
 * the seed's original colour with `new_color`. Returns the number of pixels
 * recoloured, or 0 if the seed is out of bounds or already the target colour.
 *
 * The frontier is held in a growable Point stack so recursion depth is never a
 * concern. Worst case O(w*h) time and O(w*h) extra memory.
 */
int flood_fill(Raster *r, int start_x, int start_y, int32_t new_color) {
    int32_t target = raster_get(r, start_x, start_y);
    /* Out of bounds, or filling with the colour already present: nothing to do.
     * The second guard also prevents an infinite loop, since matched pixels are
     * what we re-push. */
    if (target == -1 || target == new_color)
        return 0;

    int capacity = 64;
    int top = 0; /* index one past the last valid stack entry */
    Point *stack = (Point *)malloc(capacity * sizeof(Point));
    if (!stack)
        return 0;

    stack[top++] = (Point){start_x, start_y};
    int filled = 0;

    while (top > 0) {
        Point p = stack[--top];
        /* Skip cells that are out of bounds or are not part of the original
         * region. A cell can be pushed more than once, so we must re-check
         * here rather than trusting the push site. */
        if (raster_get(r, p.x, p.y) != target)
            continue;

        r->data[p.y * r->width + p.x] = new_color;
        filled++;

        /* Grow the stack if the four neighbours we are about to add might not
         * fit. Doubling keeps the amortised push cost constant. */
        if (top + 4 > capacity) {
            capacity *= 2;
            Point *grown = (Point *)realloc(stack, capacity * sizeof(Point));
            if (!grown) {
                free(stack);
                return filled; /* return partial progress rather than crash */
            }
            stack = grown;
        }

        stack[top++] = (Point){p.x + 1, p.y};
        stack[top++] = (Point){p.x - 1, p.y};
        stack[top++] = (Point){p.x, p.y + 1};
        stack[top++] = (Point){p.x, p.y - 1};
    }

    free(stack);
    return filled;
}
