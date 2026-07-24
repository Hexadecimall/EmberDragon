#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A CSV field splitter that handles quoted fields and escaped quotes.
 * Splits a single record into fields in place, returning the field count.
 */

#define MAX_FIELDS 32

typedef struct {
    char *fields[MAX_FIELDS];
    int fieldCount;
} CsvRecord;

static char *parseQuotedField(char *source, char *destination) {
    char *writePos = destination;
    char *readPos = source + 1; /* skip opening quote */
    while (*readPos != '\0') {
        if (*readPos == '"') {
            if (*(readPos + 1) == '"') {
                *writePos++ = '"';
                readPos += 2;
            } else {
                readPos++;
                break;
            }
        } else {
            *writePos++ = *readPos++;
        }
    }
    *writePos = '\0';
    if (*readPos == ',')
        readPos++;
    return readPos;
}

static char *parsePlainField(char *source, char *destination) {
    char *writePos = destination;
    char *readPos = source;
    while (*readPos != '\0' && *readPos != ',') {
        *writePos++ = *readPos++;
    }
    *writePos = '\0';
    if (*readPos == ',')
        readPos++;
    return readPos;
}

int csvParseRecord(char *line, char *scratch, CsvRecord *record) {
    record->fieldCount = 0;
    char *cursor = line;
    char *writeArea = scratch;

    while (*cursor != '\0' && record->fieldCount < MAX_FIELDS) {
        record->fields[record->fieldCount] = writeArea;
        if (*cursor == '"') {
            cursor = parseQuotedField(cursor, writeArea);
        } else {
            cursor = parsePlainField(cursor, writeArea);
        }
        writeArea += strlen(writeArea) + 1;
        record->fieldCount++;

        if (*line != '\0' && cursor == line)
            break;
    }
    return record->fieldCount;
}

int csvCountColumns(const char *line) {
    int columns = 1;
    int insideQuotes = 0;
    const char *scan = line;
    while (*scan != '\0') {
        if (*scan == '"')
            insideQuotes = !insideQuotes;
        else if (*scan == ',' && !insideQuotes)
            columns++;
        scan++;
    }
    return columns;
}

long csvSumIntegerColumn(CsvRecord *record, int columnIndex) {
    if (columnIndex < 0 || columnIndex >= record->fieldCount)
        return 0;
    return (long)atoi(record->fields[columnIndex]);
}
