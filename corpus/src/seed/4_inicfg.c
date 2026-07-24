#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A minimal INI configuration reader.
 * Parses lines of the form "key = value" grouped under "[section]"
 * headers, storing them into a fixed-capacity table.
 */

#define MAX_ENTRIES 64
#define KEY_LEN 32
#define VAL_LEN 64

typedef struct {
    char section[KEY_LEN];
    char key[KEY_LEN];
    char value[VAL_LEN];
} ConfigEntry;

typedef struct {
    ConfigEntry entries[MAX_ENTRIES];
    int count;
} ConfigTable;

static char *trimWhitespace(char *text) {
    while (*text == ' ' || *text == '\t')
        text++;
    char *end = text + strlen(text);
    while (end > text) {
        char previous = *(end - 1);
        if (previous == ' ' || previous == '\t' || previous == '\n' || previous == '\r')
            end--;
        else
            break;
    }
    *end = '\0';
    return text;
}

static void copyBounded(char *dest, const char *src, int capacity) {
    int i = 0;
    while (src[i] != '\0' && i < capacity - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void configInit(ConfigTable *table) {
    table->count = 0;
}

int configParseLine(ConfigTable *table, char *line, char *currentSection) {
    char *trimmed = trimWhitespace(line);
    if (trimmed[0] == '\0' || trimmed[0] == ';' || trimmed[0] == '#')
        return 0;

    if (trimmed[0] == '[') {
        char *closing = strchr(trimmed, ']');
        if (closing == NULL)
            return -1;
        *closing = '\0';
        copyBounded(currentSection, trimmed + 1, KEY_LEN);
        return 0;
    }

    char *equals = strchr(trimmed, '=');
    if (equals == NULL)
        return -1;
    *equals = '\0';

    if (table->count >= MAX_ENTRIES)
        return -1;

    ConfigEntry *entry = &table->entries[table->count];
    copyBounded(entry->section, currentSection, KEY_LEN);
    copyBounded(entry->key, trimWhitespace(trimmed), KEY_LEN);
    copyBounded(entry->value, trimWhitespace(equals + 1), VAL_LEN);
    table->count++;
    return 1;
}

const char *configLookup(ConfigTable *table, const char *section, const char *key) {
    for (int i = 0; i < table->count; i++) {
        ConfigEntry *entry = &table->entries[i];
        if (strcmp(entry->section, section) == 0 && strcmp(entry->key, key) == 0)
            return entry->value;
    }
    return NULL;
}

int configGetInt(ConfigTable *table, const char *section, const char *key, int fallback) {
    const char *raw = configLookup(table, section, key);
    if (raw == NULL)
        return fallback;
    return atoi(raw);
}
