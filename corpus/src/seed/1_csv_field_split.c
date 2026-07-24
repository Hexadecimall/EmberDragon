#include <stdint.h>

/* CSV-style record splitter with quoted-field and escaped-quote handling. */

#define MAX_FIELDS 32
#define FIELD_CAPACITY 64

typedef struct FieldBuffer {
    char data[FIELD_CAPACITY];
    int length;
} FieldBuffer;

typedef struct Record {
    FieldBuffer fields[MAX_FIELDS];
    int field_count;
} Record;

static void append_char(FieldBuffer *field, char ch) {
    if (field->length < FIELD_CAPACITY - 1) {
        field->data[field->length] = ch;
        field->length++;
        field->data[field->length] = '\0';
    }
}

static void reset_field(FieldBuffer *field) {
    field->length = 0;
    field->data[0] = '\0';
}

int split_record(const char *line, char delimiter, Record *record) {
    record->field_count = 0;
    FieldBuffer *current = &record->fields[0];
    reset_field(current);

    int in_quotes = 0;
    int index = 0;
    while (line[index] != '\0') {
        char ch = line[index];
        if (in_quotes) {
            if (ch == '"') {
                if (line[index + 1] == '"') {
                    append_char(current, '"');
                    index += 2;
                    continue;
                }
                in_quotes = 0;
                index++;
                continue;
            }
            append_char(current, ch);
            index++;
            continue;
        }
        if (ch == '"') {
            in_quotes = 1;
            index++;
            continue;
        }
        if (ch == delimiter) {
            record->field_count++;
            if (record->field_count >= MAX_FIELDS) {
                return -1;
            }
            current = &record->fields[record->field_count];
            reset_field(current);
            index++;
            continue;
        }
        append_char(current, ch);
        index++;
    }
    record->field_count++;
    return record->field_count;
}

int find_field(const Record *record, const char *needle) {
    for (int i = 0; i < record->field_count; i++) {
        const char *field = record->fields[i].data;
        int j = 0;
        while (field[j] != '\0' && needle[j] != '\0' && field[j] == needle[j]) {
            j++;
        }
        if (field[j] == '\0' && needle[j] == '\0') {
            return i;
        }
    }
    return -1;
}

int total_field_bytes(const Record *record) {
    int total = 0;
    for (int i = 0; i < record->field_count; i++) {
        total += record->fields[i].length;
    }
    return total;
}
