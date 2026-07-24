/*
 * csv_reader.c — An RFC-4180-style CSV field tokenizer.
 *
 * Splits a single CSV record into individual fields, correctly handling
 * double-quoted fields that contain commas, embedded newlines, and escaped
 * quotes ("" -> "). The parser is a small explicit state machine that writes
 * unescaped field text into a caller-supplied buffer, so it performs no
 * dynamic allocation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CSV_MAX_FIELDS 32

/* The two states of the field scanner. */
typedef enum CsvMode {
    CSV_UNQUOTED, /* default: a comma or end-of-record terminates the field */
    CSV_QUOTED    /* inside quotes: commas and newlines are literal text     */
} CsvMode;

/*
 * CsvRecord holds the parsed result. Field text is packed back-to-back into
 * the shared buffer, and each fields[i] points at one NUL-terminated slice of
 * it. This keeps every field in one allocation owned by the caller.
 */
typedef struct CsvRecord {
    char  buffer[512];               /* unescaped field bytes, packed tightly */
    char *fields[CSV_MAX_FIELDS];    /* pointers into buffer, one per field   */
    int   fieldCount;                /* number of valid entries in fields[]   */
} CsvRecord;

/*
 * csvParseRecord — Tokenize one CSV record into its fields.
 *
 * Runs the quote-aware state machine over the NUL-terminated line, filling
 * record->fields and record->fieldCount. Returns the field count on success
 * (an empty input still yields one empty field) or -1 if the record exceeds
 * CSV_MAX_FIELDS or overflows the internal buffer. A trailing '\n' or '\r\n'
 * on the line is ignored.
 */
int csvParseRecord(CsvRecord *record, const char *line) {
    CsvMode mode = CSV_UNQUOTED;
    int     written = 0;             /* bytes used in record->buffer */
    record->fieldCount = 0;

    /* Open the first field; even empty input produces one field. */
    record->fields[record->fieldCount++] = &record->buffer[written];

    const char *p = line;
    while (*p != '\0') {
        char c = *p;

        if (mode == CSV_QUOTED) {
            if (c == '"') {
                /* A doubled quote ("") is one literal quote; otherwise the
                 * quoted section ends here. */
                if (p[1] == '"') {
                    if (written + 1 >= (int)sizeof(record->buffer)) return -1;
                    record->buffer[written++] = '"';
                    p += 2;            /* consume both quote characters */
                    continue;
                }
                mode = CSV_UNQUOTED;
                p++;
                continue;
            }
            /* Any other byte, including ',' and '\n', is literal here. */
            if (written + 1 >= (int)sizeof(record->buffer)) return -1;
            record->buffer[written++] = c;
            p++;
            continue;
        }

        /* CSV_UNQUOTED below. */
        if (c == '"') {
            mode = CSV_QUOTED;         /* begin a quoted section */
            p++;
        } else if (c == ',') {
            /* Field separator: terminate the current field and start a new one. */
            if (written + 1 >= (int)sizeof(record->buffer)) return -1;
            record->buffer[written++] = '\0';
            if (record->fieldCount >= CSV_MAX_FIELDS) return -1;
            record->fields[record->fieldCount++] = &record->buffer[written];
            p++;
        } else if (c == '\r' || c == '\n') {
            break;                     /* end of record outside quotes */
        } else {
            if (written + 1 >= (int)sizeof(record->buffer)) return -1;
            record->buffer[written++] = c;
            p++;
        }
    }

    /* Terminate the final field. */
    if (written + 1 >= (int)sizeof(record->buffer)) return -1;
    record->buffer[written++] = '\0';
    return record->fieldCount;
}

/*
 * csvFieldAsInt — Interpret a parsed field as a base-10 integer.
 *
 * Accepts an optional leading '+'/'-' sign followed by digits. Returns 0 and
 * writes the parsed value to *out on success, or -1 if the index is out of
 * range or the field contains a non-numeric character. Leading and trailing
 * spaces around the number are tolerated.
 */
int csvFieldAsInt(const CsvRecord *record, int index, int64_t *out) {
    if (index < 0 || index >= record->fieldCount) return -1;
    const char *s = record->fields[index];
    while (*s == ' ' || *s == '\t') s++;

    int64_t sign = 1;
    if (*s == '+' || *s == '-') {
        if (*s == '-') sign = -1;
        s++;
    }
    if (*s < '0' || *s > '9') return -1; /* need at least one digit */

    int64_t value = 0;
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
    }
    while (*s == ' ' || *s == '\t') s++; /* allow trailing spaces */
    if (*s != '\0') return -1;           /* trailing junk is an error */

    *out = value * sign;
    return 0;
}
