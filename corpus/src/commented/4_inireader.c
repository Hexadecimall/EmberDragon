/*
 * inireader.c — A minimal INI configuration parser.
 *
 * Reads "[section]" headers and "key = value" lines from an in-memory INI
 * buffer into a fixed-capacity table of entries, then offers case-sensitive
 * lookup by section and key. Comment lines (';' or '#') and blank lines are
 * ignored. No dynamic allocation: all storage is inline and bounded.
 */

#include <string.h>

#define MAX_ENTRIES 64   /* hard cap on stored key/value pairs */
#define NAME_LEN    32    /* max length (incl. null) of section and key names */
#define VALUE_LEN   64    /* max length (incl. null) of a value string */

/*
 * One parsed key/value pair, tagged with the section it belongs to. Storing the
 * section on every entry keeps lookup a flat linear scan with no nesting.
 */
typedef struct IniEntry {
    char section[NAME_LEN];
    char key[NAME_LEN];
    char value[VALUE_LEN];
} IniEntry;

/* The whole parsed document: a flat array plus a live count. */
typedef struct IniFile {
    IniEntry entries[MAX_ENTRIES];
    int      count;
} IniFile;

/*
 * Trim leading and trailing ASCII whitespace from a string in place.
 * Parameters: s — null-terminated, mutable. Returns nothing; shifts content to
 * the front and truncates the tail with a null. O(n) in the string length.
 */
static void trim(char *s) {
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t')
        start++;

    int len = (int)strlen(s + start);
    /* Move the trimmed-left content to the buffer head (memmove: ranges overlap). */
    memmove(s, s + start, (size_t)len + 1);

    /* Walk back over trailing whitespace and CR/LF, capping with a null. */
    while (len > 0) {
        char c = s[len - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            len--;
        else
            break;
    }
    s[len] = '\0';
}

/*
 * Copy at most cap-1 bytes of src into dst and always null-terminate.
 * Parameters: dst — destination buffer of size cap; src — source; cap — dst
 * capacity. Returns nothing. Safe against overrun; truncates if src is longer.
 */
static void copyBounded(char *dst, const char *src, int cap) {
    int i = 0;
    while (i < cap - 1 && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/*
 * Parse an entire INI text buffer into the table.
 * Parameters: ini — output table (will be reset); text — null-terminated INI
 * source. Returns the number of entries stored. Stops early if MAX_ENTRIES is
 * reached. O(n) over the input. Lines longer than the line buffer are clipped.
 */
int iniParse(IniFile *ini, const char *text) {
    ini->count = 0;
    char current[NAME_LEN] = "";   /* section in effect for following keys */
    char line[VALUE_LEN + NAME_LEN + 8];

    const char *p = text;
    while (*p != '\0' && ini->count < MAX_ENTRIES) {
        /* Copy one physical line (up to the newline) into the work buffer. */
        int n = 0;
        while (*p != '\0' && *p != '\n' && n < (int)sizeof(line) - 1)
            line[n++] = *p++;
        line[n] = '\0';
        if (*p == '\n')
            p++;                   /* consume the newline separator */

        trim(line);
        if (line[0] == '\0' || line[0] == ';' || line[0] == '#')
            continue;              /* skip blanks and comments */

        if (line[0] == '[') {
            /* Section header: copy the text between the brackets. */
            char *close = strchr(line, ']');
            if (close != NULL) {
                *close = '\0';
                copyBounded(current, line + 1, NAME_LEN);
                trim(current);
            }
            continue;
        }

        /* Key/value: split on the first '='. Lines without one are ignored. */
        char *eq = strchr(line, '=');
        if (eq == NULL)
            continue;
        *eq = '\0';
        char *keyPart = line;
        char *valPart = eq + 1;
        trim(keyPart);
        trim(valPart);

        IniEntry *e = &ini->entries[ini->count++];
        copyBounded(e->section, current, NAME_LEN);
        copyBounded(e->key, keyPart, NAME_LEN);
        copyBounded(e->value, valPart, VALUE_LEN);
    }
    return ini->count;
}

/*
 * Look up a value by section and key.
 * Parameters: ini — parsed table; section — section name; key — key name.
 * Returns a pointer to the stored value string, or NULL if no such pair exists.
 * Matching is exact and case-sensitive. O(n) linear scan. Pointer is owned by
 * the IniFile and stays valid as long as it does.
 */
const char *iniGet(const IniFile *ini, const char *section, const char *key) {
    for (int i = 0; i < ini->count; i++) {
        const IniEntry *e = &ini->entries[i];
        if (strcmp(e->section, section) == 0 && strcmp(e->key, key) == 0)
            return e->value;
    }
    return NULL;   /* not found */
}
