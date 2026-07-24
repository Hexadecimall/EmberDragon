/*
 * csvreader.c — A single-row CSV field splitter with RFC-4180-style quoting.
 *
 * Splits one CSV record into comma-separated fields, honouring double-quoted
 * fields that may contain commas and escaped quotes ("" -> "). The split is
 * driven by a small explicit state machine so quoting rules stay readable.
 */

#include <stddef.h>

#define MAX_FIELDS 32    /* maximum fields parsed from a single record */
#define FIELD_LEN  64    /* max bytes (incl. null) captured per field */

/*
 * The output of a parse: a fixed array of field buffers plus how many were
 * actually filled. Owning the storage inline avoids any allocation.
 */
typedef struct CsvRow {
    char fields[MAX_FIELDS][FIELD_LEN];
    int  count;
} CsvRow;

/* Scanner states for the quoting state machine. */
typedef enum CsvState {
    FIELD_START,   /* at the first byte of a field; decide quoted vs. plain */
    IN_PLAIN,      /* inside an unquoted field; comma ends it */
    IN_QUOTED,     /* inside a quoted field; only a closing quote ends it */
    AFTER_QUOTE    /* just saw a quote inside a quoted field; "" or close? */
} CsvState;

/*
 * Append one character to the current field, respecting its capacity.
 * Parameters: row — target row; fieldIndex — which field; ch — byte to add;
 * len — pointer to the field's current length (updated). Returns nothing.
 * Silently drops characters once the field buffer is full (keeps the null).
 */
static void pushChar(CsvRow *row, int fieldIndex, char ch, int *len) {
    if (*len < FIELD_LEN - 1) {
        row->fields[fieldIndex][*len] = ch;
        (*len)++;
        row->fields[fieldIndex][*len] = '\0';  /* keep field null-terminated */
    }
}

/*
 * Parse a single CSV record into separate fields.
 * Parameters: row — output (reset by the call); line — null-terminated record
 * without its trailing newline. Returns the number of fields produced. Empty
 * input yields one empty field. Stops accepting new fields at MAX_FIELDS.
 * O(n) over the line length.
 */
int csvParseRow(CsvRow *row, const char *line) {
    row->count = 0;
    int field = 0;             /* index of the field being built */
    int len = 0;               /* length of the current field */
    CsvState state = FIELD_START;
    row->fields[0][0] = '\0';  /* ensure field 0 is a valid empty string */

    const char *p = line;
    for (; *p != '\0'; p++) {
        char c = *p;
        switch (state) {
        case FIELD_START:
            if (c == '"') {
                state = IN_QUOTED;          /* opening quote: enter quoted mode */
            } else if (c == ',') {
                /* Empty field, immediately terminated by a delimiter. */
                if (field < MAX_FIELDS - 1) {
                    field++;
                    len = 0;
                    row->fields[field][0] = '\0';
                }
            } else {
                pushChar(row, field, c, &len);
                state = IN_PLAIN;
            }
            break;

        case IN_PLAIN:
            if (c == ',') {
                /* Delimiter ends the field; start the next one. */
                if (field < MAX_FIELDS - 1) {
                    field++;
                    len = 0;
                    row->fields[field][0] = '\0';
                }
                state = FIELD_START;
            } else {
                pushChar(row, field, c, &len);
            }
            break;

        case IN_QUOTED:
            if (c == '"') {
                /* Could be the close quote or the first of an escaped pair. */
                state = AFTER_QUOTE;
            } else {
                pushChar(row, field, c, &len);  /* commas here are literal */
            }
            break;

        case AFTER_QUOTE:
            if (c == '"') {
                /* "" inside a quoted field encodes a single literal quote. */
                pushChar(row, field, '"', &len);
                state = IN_QUOTED;
            } else if (c == ',') {
                if (field < MAX_FIELDS - 1) {
                    field++;
                    len = 0;
                    row->fields[field][0] = '\0';
                }
                state = FIELD_START;
            } else {
                /* Stray char after a closing quote: treat as plain text. */
                pushChar(row, field, c, &len);
                state = IN_PLAIN;
            }
            break;
        }
    }

    row->count = field + 1;   /* one more field than the number of delimiters */
    return row->count;
}

/*
 * Fetch a parsed field by index.
 * Parameters: row — parsed row; index — field number. Returns a pointer to the
 * field text, or NULL if the index is out of range. The pointer is owned by the
 * CsvRow. O(1).
 */
const char *csvField(const CsvRow *row, int index) {
    if (index < 0 || index >= row->count)
        return NULL;
    return row->fields[index];
}
