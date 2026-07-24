/*
 * Scanline flood fill over an 8-bit integer raster.
 *
 * Given a grid of color values, this module replaces a connected region of
 * one color with another, starting from a seed pixel and spreading through
 * 4-connected neighbors. The implementation uses an explicit stack of
 * horizontal spans instead of naive per-pixel recursion, which keeps memory
 * bounded and avoids deep call stacks on large fills.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * A raster image: a width x height buffer of single-byte color indices stored
 * row-major. The caller owns 'pixels'; this module only reads and writes it.
 */
typedef struct {
    int      width;
    int      height;
    uint8_t *pixels;  /* width * height bytes, row-major, not owned here */
} Raster;

/*
 * One pending horizontal segment to be processed by the scanline fill.
 * 'x1' and 'x2' are inclusive column bounds on row 'y'.
 */
typedef struct {
    int x1;
    int x2;
    int y;
} Span;

/*
 * Read the color index at (x, y).
 * Precondition: the coordinate is in bounds (callers guard before calling).
 * Returns the stored byte. O(1).
 */
static uint8_t raster_at(const Raster *img, int x, int y) {
    return img->pixels[y * img->width + x];
}

/*
 * Write 'color' to the pixel at (x, y).
 * Precondition: the coordinate is in bounds. O(1).
 */
static void raster_put(Raster *img, int x, int y, uint8_t color) {
    img->pixels[y * img->width + x] = color;
}

/*
 * Flood fill the 4-connected region of 'target' color reachable from the seed
 * pixel (seed_x, seed_y), recoloring it to 'replacement'.
 *
 * Parameters:
 *   img         - the raster to modify in place.
 *   seed_x/y    - starting pixel; ignored (no-op) if out of bounds.
 *   replacement - the new color to paint.
 *
 * Returns the number of pixels recolored, or -1 if a scratch stack could not
 * be allocated. If the seed already holds 'replacement', returns 0 immediately
 * to avoid an infinite loop. Runs in O(width * height) time; the span stack
 * needs at most O(height) entries at once.
 */
int flood_fill(Raster *img, int seed_x, int seed_y, uint8_t replacement) {
    if (seed_x < 0 || seed_x >= img->width ||
        seed_y < 0 || seed_y >= img->height) {
        return 0;  /* seed outside the canvas: nothing to do */
    }

    uint8_t target = raster_at(img, seed_x, seed_y);
    if (target == replacement) {
        /* Filling a color with itself would never terminate; bail early. */
        return 0;
    }

    /* In the worst case every pixel could become its own pending span before
       being filled, so size the stack to the full pixel count plus one. This
       is conservative but guarantees no overflow on pathological inputs. */
    int capacity = img->width * img->height + 1;
    Span *stack = (Span *)malloc((size_t)capacity * sizeof(Span));
    if (!stack) {
        return -1;  /* allocation failure: signal the caller */
    }

    int top = 0;
    int filled = 0;

    /* Seed the stack with the single-pixel span at the start point. */
    stack[top].x1 = seed_x;
    stack[top].x2 = seed_x;
    stack[top].y  = seed_y;
    top++;

    while (top > 0) {
        Span span = stack[--top];
        int y = span.y;

        /* Expand the span leftward to the start of the matching run. */
        int left = span.x1;
        while (left > 0 && raster_at(img, left - 1, y) == target) {
            left--;
        }
        /* Expand rightward to the end of the matching run. */
        int right = span.x2;
        while (right < img->width - 1 && raster_at(img, right + 1, y) == target) {
            right++;
        }

        /* Paint the full run and account for it. */
        for (int x = left; x <= right; ++x) {
            raster_put(img, x, y, replacement);
            filled++;
        }

        /* Queue matching runs on the rows directly above and below. We push
           the whole [left, right] band; the next pop re-scans it for runs,
           which naturally skips already-filled (now 'replacement') pixels. */
        if (y > 0) {
            stack[top].x1 = left;
            stack[top].x2 = right;
            stack[top].y  = y - 1;
            top++;
        }
        if (y < img->height - 1) {
            stack[top].x1 = left;
            stack[top].x2 = right;
            stack[top].y  = y + 1;
            top++;
        }
    }

    free(stack);
    return filled;
}
