/*
 * ascii_table.c -- Render a grid of strings as a bordered ASCII table.
 *
 * Given a rectangular array of cell strings, this module computes per-column
 * widths and draws a box-drawing-style table using +, -, and | characters,
 * with a separator rule under the header row. It is the kind of helper a CLI
 * tool uses to print results in aligned columns.
 */

#include <stddef.h>
#include <string.h>

/* Upper bounds so the renderer can use fixed-size scratch arrays. */
#define MAX_COLS 16

/* A table is a flat row-major array of cell pointers plus its dimensions. */
typedef struct {
    const char *const *cells; /* rows*cols entries, row-major            */
    size_t rows;              /* total rows including the header row      */
    size_t cols;             /* number of columns (<= MAX_COLS)          */
} Table;

/*
 * cellAt -- Fetch the cell string at (row, col) from a Table.
 *
 * @t:   the table.
 * @row: zero-based row index.
 * @col: zero-based column index.
 * Returns the cell pointer; the caller guarantees indices are in range. O(1).
 */
static const char *cellAt(const Table *t, size_t row, size_t col) {
    return t->cells[row * t->cols + col];
}

/*
 * computeColumnWidths -- Find the display width of each column.
 *
 * @t:      the table to measure.
 * @widths: out array of at least t->cols entries, filled with the max string
 *          length seen in each column.
 *
 * A column's width is the longest cell (header or body) in that column. This
 * lets every cell in the column be padded to a common size. O(rows*cols).
 */
static void computeColumnWidths(const Table *t, size_t *widths) {
    for (size_t c = 0; c < t->cols; c++) {
        widths[c] = 0;
    }
    for (size_t r = 0; r < t->rows; r++) {
        for (size_t c = 0; c < t->cols; c++) {
            size_t len = strlen(cellAt(t, r, c));
            if (len > widths[c]) {
                widths[c] = len;   /* track the widest cell per column */
            }
        }
    }
}

/*
 * putChar -- Append one character to the output buffer if room remains.
 *
 * @out: destination buffer.
 * @cap: capacity of `out`.
 * @idx: write position (updated in place).
 * @c:   character to write.
 *
 * Silently drops the write when the buffer is full, guaranteeing no overflow.
 */
static void putChar(char *out, size_t cap, size_t *idx, char c) {
    if (*idx + 1 < cap) {
        out[(*idx)++] = c;
    }
}

/*
 * drawRule -- Emit a horizontal border line like "+----+------+".
 *
 * @out:    destination buffer.
 * @cap:    capacity of `out`.
 * @idx:    write position (updated in place).
 * @widths: per-column widths.
 * @cols:   number of columns.
 *
 * Each column contributes its width plus two spaces of cell padding, drawn as
 * dashes between '+' junctions. Ends with a newline. O(total table width).
 */
static void drawRule(char *out, size_t cap, size_t *idx,
                     const size_t *widths, size_t cols) {
    putChar(out, cap, idx, '+');
    for (size_t c = 0; c < cols; c++) {
        /* +2 accounts for the single space of padding on each side of a cell. */
        for (size_t d = 0; d < widths[c] + 2; d++) {
            putChar(out, cap, idx, '-');
        }
        putChar(out, cap, idx, '+');
    }
    putChar(out, cap, idx, '\n');
}

/*
 * drawRow -- Emit one data row like "| Name  | Age |".
 *
 * @out:    destination buffer.
 * @cap:    capacity of `out`.
 * @idx:    write position (updated in place).
 * @t:      the table (for cell access).
 * @row:    which row to render.
 * @widths: per-column widths.
 *
 * Each cell is left-aligned and space-padded to its column width, wrapped in
 * "| " ... " " delimiters. Ends with a newline. O(row width).
 */
static void drawRow(char *out, size_t cap, size_t *idx, const Table *t,
                    size_t row, const size_t *widths) {
    putChar(out, cap, idx, '|');
    for (size_t c = 0; c < t->cols; c++) {
        const char *cell = cellAt(t, row, c);
        size_t len = strlen(cell);

        putChar(out, cap, idx, ' ');
        for (size_t k = 0; k < len; k++) {
            putChar(out, cap, idx, cell[k]);
        }
        /* Pad the remaining width so the right border lines up. */
        for (size_t p = len; p < widths[c]; p++) {
            putChar(out, cap, idx, ' ');
        }
        putChar(out, cap, idx, ' ');
        putChar(out, cap, idx, '|');
    }
    putChar(out, cap, idx, '\n');
}

/*
 * renderTable -- Render an entire table into a text buffer.
 *
 * @out: destination buffer for the NUL-terminated rendering.
 * @cap: capacity of `out` in bytes.
 * @t:   the table to render; t->cols must be <= MAX_COLS.
 *
 * Returns the number of characters written. Layout is: top rule, header row,
 * header separator rule, each body row, and a bottom rule. Returns 0 if the
 * table is empty or has too many columns. O(rows * total width).
 */
size_t renderTable(char *out, size_t cap, const Table *t) {
    if (cap == 0) return 0;
    if (t->rows == 0 || t->cols == 0 || t->cols > MAX_COLS) {
        out[0] = '\0';
        return 0;
    }

    size_t widths[MAX_COLS];
    computeColumnWidths(t, widths);

    size_t idx = 0;
    drawRule(out, cap, &idx, widths, t->cols);          /* top border      */
    drawRow(out, cap, &idx, t, 0, widths);              /* header row       */
    drawRule(out, cap, &idx, widths, t->cols);          /* header separator */
    for (size_t r = 1; r < t->rows; r++) {
        drawRow(out, cap, &idx, t, r, widths);          /* body rows        */
    }
    drawRule(out, cap, &idx, widths, t->cols);          /* bottom border    */

    out[idx] = '\0';
    return idx;
}
