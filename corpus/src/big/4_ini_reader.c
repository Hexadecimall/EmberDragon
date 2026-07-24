/*
 * ini_reader.c — A minimal INI configuration parser.
 *
 * Parses classic INI text consisting of "[section]" headers and
 * "key = value" lines into a flat, fixed-capacity table of entries keyed by
 * "section.key". Comments beginning with ';' or '#' and surrounding
 * whitespace are stripped. The parser allocates nothing on the heap, so the
 * caller owns the lifetime of the IniDocument it passes in.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INI_MAX_ENTRIES 64
#define INI_MAX_NAME    64
#define INI_MAX_VALUE   128

/* One resolved "section.key = value" triple. */
typedef struct IniEntry {
    char section[INI_MAX_NAME];
    char key[INI_MAX_NAME];
    char value[INI_MAX_VALUE];
} IniEntry;

/* The full parsed document: a bounded array of entries. */
typedef struct IniDocument {
    IniEntry entries[INI_MAX_ENTRIES];
    int      count;
} IniDocument;

/*
 * trimInPlace — Strip leading and trailing ASCII whitespace from a string.
 *
 * Returns a pointer to the first non-space character and writes a NUL over the
 * trailing whitespace, mutating the buffer in place. The returned pointer
 * stays valid as long as the original buffer does.
 */
static char *trimInPlace(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;          /* string was all whitespace */
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' ||
                       *end == '\r' || *end == '\n')) {
        *end-- = '\0';
    }
    return s;
}

/*
 * copyBounded — Copy a NUL-terminated string into a fixed buffer safely.
 *
 * Copies at most capacity-1 bytes from src into dst and always NUL-terminates.
 * Longer sources are silently truncated, which matches INI's lenient nature.
 */
static void copyBounded(char *dst, const char *src, size_t capacity) {
    size_t i = 0;
    while (i + 1 < capacity && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/*
 * iniInit — Reset a document to the empty state before parsing.
 *
 * Sets the entry count to zero. Always call this once before iniParse so a
 * reused IniDocument does not retain stale entries.
 */
void iniInit(IniDocument *doc) {
    doc->count = 0;
}

/*
 * iniParse — Parse a full INI text buffer into doc.
 *
 * Walks text line by line, tracking the current "[section]" and appending one
 * entry per "key = value" line. Returns the number of entries stored, or -1 if
 * the document overflows INI_MAX_ENTRIES. Lines that are blank, comment-only,
 * or lack an '=' outside of a header are skipped without error.
 */
int iniParse(IniDocument *doc, const char *text) {
    char section[INI_MAX_NAME] = "";   /* "" is the implicit global section */
    char line[INI_MAX_VALUE + INI_MAX_NAME];
    const char *p = text;

    while (*p != '\0') {
        /* Copy one physical line (up to '\n') into the scratch buffer. */
        size_t len = 0;
        while (*p != '\0' && *p != '\n' && len + 1 < sizeof(line)) {
            line[len++] = *p++;
        }
        line[len] = '\0';
        while (*p != '\0' && *p != '\n') p++; /* skip an overlong tail */
        if (*p == '\n') p++;

        char *content = trimInPlace(line);
        if (content[0] == '\0' || content[0] == ';' || content[0] == '#') {
            continue;                  /* blank or comment line */
        }

        if (content[0] == '[') {
            /* Section header: capture the name between the brackets. */
            char *close = strchr(content, ']');
            if (close == NULL) continue; /* malformed header, ignore */
            *close = '\0';
            copyBounded(section, trimInPlace(content + 1), INI_MAX_NAME);
            continue;
        }

        /* Otherwise expect a key/value pair split on the first '='. */
        char *equals = strchr(content, '=');
        if (equals == NULL) continue;  /* not a pair; tolerate and skip */
        *equals = '\0';
        char *rawKey = trimInPlace(content);
        char *rawValue = trimInPlace(equals + 1);
        if (rawKey[0] == '\0') continue; /* empty key is meaningless */

        if (doc->count >= INI_MAX_ENTRIES) return -1;
        IniEntry *entry = &doc->entries[doc->count++];
        copyBounded(entry->section, section, INI_MAX_NAME);
        copyBounded(entry->key, rawKey, INI_MAX_NAME);
        copyBounded(entry->value, rawValue, INI_MAX_VALUE);
    }
    return doc->count;
}

/*
 * iniLookup — Find the value for a given section and key.
 *
 * Performs a linear O(n) scan over the parsed entries. Returns a pointer to
 * the stored value string on a match, or NULL if no such section/key exists.
 * The returned pointer is owned by doc and must not be freed.
 */
const char *iniLookup(const IniDocument *doc, const char *section, const char *key) {
    for (int i = 0; i < doc->count; i++) {
        const IniEntry *e = &doc->entries[i];
        if (strcmp(e->section, section) == 0 && strcmp(e->key, key) == 0) {
            return e->value;
        }
    }
    return NULL;
}
