/*
 * column_align.c -- Fixed-width column alignment for text fields.
 *
 * This module pads a string into a destination buffer of a requested width,
 * supporting left, right, and centered alignment. It is the low-level helper
 * a table renderer uses to make cells line up into clean columns. All output
 * is truncated (never overflowed) to the destination capacity.
 */

#include <stddef.h>
#include <string.h>

/* The three ways a value can sit inside a fixed-width column. */
typedef enum {
    ALIGN_LEFT,    /* text hugs the left edge, padding on the right  */
    ALIGN_RIGHT,   /* text hugs the right edge, padding on the left  */
    ALIGN_CENTER   /* text centered, extra odd pad goes to the right */
} Alignment;

/*
 * stringLength -- Compute the length of a NUL-terminated string.
 *
 * @s: pointer to a NUL-terminated string (must not be NULL).
 * Returns the number of bytes before the terminator. O(n) in the length.
 *
 * Reimplemented here so the module depends on nothing but <string.h>'s
 * memset/memcpy for the bulk copies.
 */
static size_t stringLength(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

/*
 * fillSpaces -- Write `count` space characters into a buffer.
 *
 * @dst:   destination cursor.
 * @count: number of spaces to emit (0 is valid and writes nothing).
 * Returns a pointer just past the last space written, so callers can chain.
 */
static char *fillSpaces(char *dst, size_t count) {
    if (count > 0) {
        memset(dst, ' ', count);
    }
    return dst + count;
}

/*
 * alignField -- Render `text` into `out` padded to exactly `width` columns.
 *
 * @out:   destination buffer.
 * @cap:   capacity of `out` in bytes, including room for the NUL terminator.
 * @text:  the source string to place.
 * @width: desired visible column width.
 * @mode:  one of the Alignment values.
 *
 * Returns the number of visible characters written (excluding the NUL), which
 * is min(width, cap-1). If the text is wider than `width` it is copied as-is
 * up to the column width and then truncated to fit `cap`. The result is always
 * NUL-terminated as long as cap > 0. O(width) time.
 */
size_t alignField(char *out, size_t cap, const char *text,
                  size_t width, Alignment mode) {
    if (cap == 0) {
        return 0;               /* no room even for a terminator */
    }

    size_t textLen = stringLength(text);

    /* The visible field never exceeds the column width... */
    size_t fieldWidth = width;
    /* ...nor the buffer's writable capacity (leaving one byte for NUL). */
    if (fieldWidth > cap - 1) {
        fieldWidth = cap - 1;
    }

    /* If the text alone fills or overflows the field, there is no padding. */
    size_t visibleText = textLen < fieldWidth ? textLen : fieldWidth;
    size_t totalPad = fieldWidth - visibleText;

    /* Split the padding between the two sides according to the mode. */
    size_t leftPad = 0;
    size_t rightPad = 0;
    switch (mode) {
        case ALIGN_LEFT:
            rightPad = totalPad;
            break;
        case ALIGN_RIGHT:
            leftPad = totalPad;
            break;
        case ALIGN_CENTER:
            leftPad = totalPad / 2;       /* floor on the left... */
            rightPad = totalPad - leftPad; /* ...remainder on the right */
            break;
    }

    char *cursor = out;
    cursor = fillSpaces(cursor, leftPad);
    memcpy(cursor, text, visibleText);     /* copy only what fits */
    cursor += visibleText;
    cursor = fillSpaces(cursor, rightPad);
    *cursor = '\0';

    return (size_t)(cursor - out);
}

/*
 * measureMaxWidth -- Find the widest string in an array of cells.
 *
 * @cells: array of NUL-terminated strings.
 * @count: number of entries in `cells`.
 * Returns the length of the longest string, or 0 if count is 0.
 *
 * A table builder calls this once per column to choose a column width that
 * fits every cell. O(total characters) time.
 */
size_t measureMaxWidth(const char *const *cells, size_t count) {
    size_t widest = 0;
    for (size_t i = 0; i < count; i++) {
        size_t len = stringLength(cells[i]);
        if (len > widest) {
            widest = len;       /* track the running maximum */
        }
    }
    return widest;
}
