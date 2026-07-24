/*
 * json_int.cpp — A recursive-descent parser for an integer-only JSON subset.
 *
 * Parses JSON values restricted to objects, arrays, integer numbers, the
 * literals true/false/null, and double-quoted strings without escape
 * sequences. It builds a small tagged value tree on the heap; callers free it
 * with freeJsonValue. The grammar is intentionally tiny so the whole parser
 * fits in one self-contained translation unit with no external dependencies.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Discriminator for the tagged JsonValue union. */
enum JsonType {
    JSON_NULL,
    JSON_BOOL,
    JSON_INT,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

/*
 * JsonValue is a node in the parsed tree. Arrays own a contiguous block of
 * child pointers; objects own parallel arrays of keys and child values. The
 * unused members for a given type are simply left zeroed.
 */
struct JsonValue {
    JsonType    type;
    int64_t     intValue;            /* valid when type == JSON_INT          */
    char       *stringValue;         /* owned string for JSON_STRING / keys  */
    JsonValue **children;            /* owned child array (array/object)     */
    char      **keys;                /* owned key array, parallel to children */
    int         childCount;          /* number of valid children             */
};

/*
 * Parser carries the input cursor and a sticky failure flag through the mutual
 * recursion so individual helpers don't each need to return error codes.
 */
struct Parser {
    const char *cursor;
    bool        failed;
};

/* Forward declaration: values nest, so the parsers are mutually recursive. */
static JsonValue *parseValue(Parser *parser);

/*
 * skipWhitespace — Advance the cursor past spaces, tabs, and line breaks.
 *
 * Mutates parser->cursor in place; has no effect at end of input.
 */
static void skipWhitespace(Parser *parser) {
    const char *p = parser->cursor;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    parser->cursor = p;
}

/*
 * allocValue — Allocate and zero-initialize a JsonValue of the given type.
 *
 * Returns a heap node with all pointer/count fields cleared, or NULL if the
 * allocation fails (in which case the caller should mark the parser failed).
 */
static JsonValue *allocValue(JsonType type) {
    JsonValue *value = (JsonValue *)calloc(1, sizeof(JsonValue));
    if (value != NULL) value->type = type;
    return value;
}

/*
 * parseString — Parse a double-quoted string with no escape handling.
 *
 * Assumes the cursor sits on the opening quote. Allocates and returns a fresh
 * NUL-terminated copy of the contents, advancing past the closing quote. On a
 * missing closing quote it sets parser->failed and returns NULL. Caller owns
 * the returned buffer.
 */
static char *parseString(Parser *parser) {
    if (*parser->cursor != '"') { parser->failed = true; return NULL; }
    const char *start = ++parser->cursor;   /* first char after the quote */
    while (*parser->cursor != '"' && *parser->cursor != '\0') {
        parser->cursor++;
    }
    if (*parser->cursor != '"') { parser->failed = true; return NULL; }

    size_t length = (size_t)(parser->cursor - start);
    char *text = (char *)malloc(length + 1);
    if (text == NULL) { parser->failed = true; return NULL; }
    memcpy(text, start, length);
    text[length] = '\0';
    parser->cursor++;                        /* step over closing quote */
    return text;
}

/*
 * parseNumber — Parse an optionally signed base-10 integer.
 *
 * Reads a leading '-' plus one or more digits into a JSON_INT node. Sets
 * parser->failed if no digit is present. Returns the new node or NULL on
 * allocation/parse failure.
 */
static JsonValue *parseNumber(Parser *parser) {
    int64_t sign = 1;
    if (*parser->cursor == '-') { sign = -1; parser->cursor++; }
    if (*parser->cursor < '0' || *parser->cursor > '9') {
        parser->failed = true;
        return NULL;
    }
    int64_t magnitude = 0;
    while (*parser->cursor >= '0' && *parser->cursor <= '9') {
        magnitude = magnitude * 10 + (*parser->cursor - '0');
        parser->cursor++;
    }
    JsonValue *node = allocValue(JSON_INT);
    if (node == NULL) { parser->failed = true; return NULL; }
    node->intValue = sign * magnitude;
    return node;
}

/*
 * matchLiteral — Consume an exact keyword if it appears at the cursor.
 *
 * Returns true and advances past the word on a match; otherwise leaves the
 * cursor untouched and returns false. Used for true/false/null.
 */
static bool matchLiteral(Parser *parser, const char *word) {
    size_t len = strlen(word);
    if (strncmp(parser->cursor, word, len) == 0) {
        parser->cursor += len;
        return true;
    }
    return false;
}

/*
 * appendChild — Grow an array/object node by one child (and optional key).
 *
 * Reallocates the children (and keys, when key != NULL) arrays to hold one
 * more element and stores the pair. Sets parser->failed on allocation failure.
 * Takes ownership of child and key.
 */
static void appendChild(Parser *parser, JsonValue *parent, char *key, JsonValue *child) {
    int newCount = parent->childCount + 1;
    JsonValue **grown =
        (JsonValue **)realloc(parent->children, (size_t)newCount * sizeof(JsonValue *));
    if (grown == NULL) { parser->failed = true; return; }
    parent->children = grown;
    parent->children[parent->childCount] = child;

    if (key != NULL) {
        char **grownKeys =
            (char **)realloc(parent->keys, (size_t)newCount * sizeof(char *));
        if (grownKeys == NULL) { parser->failed = true; return; }
        parent->keys = grownKeys;
        parent->keys[parent->childCount] = key;
    }
    parent->childCount = newCount;
}

/*
 * parseArray — Parse a bracketed, comma-separated list of values.
 *
 * Assumes the cursor sits on '['. Returns a JSON_ARRAY node owning its
 * elements, or NULL on malformed input (parser->failed is set). Handles the
 * empty array "[]" as a special case.
 */
static JsonValue *parseArray(Parser *parser) {
    parser->cursor++;                        /* consume '[' */
    JsonValue *array = allocValue(JSON_ARRAY);
    if (array == NULL) { parser->failed = true; return NULL; }

    skipWhitespace(parser);
    if (*parser->cursor == ']') { parser->cursor++; return array; }

    for (;;) {
        JsonValue *element = parseValue(parser);
        if (parser->failed) return array;     /* caller will free partial tree */
        appendChild(parser, array, NULL, element);
        if (parser->failed) return array;

        skipWhitespace(parser);
        if (*parser->cursor == ',') { parser->cursor++; continue; }
        if (*parser->cursor == ']') { parser->cursor++; break; }
        parser->failed = true;                /* expected ',' or ']' */
        break;
    }
    return array;
}

/*
 * parseObject — Parse a brace-delimited set of "string": value members.
 *
 * Assumes the cursor sits on '{'. Returns a JSON_OBJECT node owning parallel
 * keys and values, or NULL/failed on malformed input. Handles the empty
 * object "{}".
 */
static JsonValue *parseObject(Parser *parser) {
    parser->cursor++;                        /* consume '{' */
    JsonValue *object = allocValue(JSON_OBJECT);
    if (object == NULL) { parser->failed = true; return NULL; }

    skipWhitespace(parser);
    if (*parser->cursor == '}') { parser->cursor++; return object; }

    for (;;) {
        skipWhitespace(parser);
        char *key = parseString(parser);
        if (parser->failed) return object;

        skipWhitespace(parser);
        if (*parser->cursor != ':') { parser->failed = true; free(key); return object; }
        parser->cursor++;                    /* consume ':' */

        JsonValue *member = parseValue(parser);
        if (parser->failed) { free(key); return object; }
        appendChild(parser, object, key, member);
        if (parser->failed) return object;

        skipWhitespace(parser);
        if (*parser->cursor == ',') { parser->cursor++; continue; }
        if (*parser->cursor == '}') { parser->cursor++; break; }
        parser->failed = true;
        break;
    }
    return object;
}

/*
 * parseValue — Dispatch on the next token to parse any single JSON value.
 *
 * Skips leading whitespace, then routes to the object/array/string/number
 * sub-parser or matches a bare literal. Returns the parsed node, or NULL with
 * parser->failed set on an unexpected character.
 */
static JsonValue *parseValue(Parser *parser) {
    skipWhitespace(parser);
    char c = *parser->cursor;
    if (c == '{') return parseObject(parser);
    if (c == '[') return parseArray(parser);
    if (c == '"') {
        JsonValue *node = allocValue(JSON_STRING);
        if (node == NULL) { parser->failed = true; return NULL; }
        node->stringValue = parseString(parser);
        return node;
    }
    if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(parser);
    if (matchLiteral(parser, "true"))  { JsonValue *n = allocValue(JSON_BOOL); if (n) n->intValue = 1; return n; }
    if (matchLiteral(parser, "false")) { JsonValue *n = allocValue(JSON_BOOL); if (n) n->intValue = 0; return n; }
    if (matchLiteral(parser, "null"))  return allocValue(JSON_NULL);

    parser->failed = true;
    return NULL;
}

/*
 * freeJsonValue — Recursively release a value tree built by the parser.
 *
 * Frees all owned strings, keys, child arrays, and the node itself. Safe to
 * call on NULL. After this returns the pointer is dangling and must not be
 * reused.
 */
void freeJsonValue(JsonValue *value) {
    if (value == NULL) return;
    for (int i = 0; i < value->childCount; i++) {
        freeJsonValue(value->children[i]);
        if (value->keys != NULL) free(value->keys[i]);
    }
    free(value->children);
    free(value->keys);
    free(value->stringValue);
    free(value);
}

/*
 * parseJson — Top-level entry point: parse a whole document string.
 *
 * Parses exactly one value and verifies only trailing whitespace follows.
 * Returns the root node on success, or NULL on any syntax error (freeing any
 * partial tree first). Caller owns the returned tree and must freeJsonValue it.
 */
JsonValue *parseJson(const char *text) {
    Parser parser;
    parser.cursor = text;
    parser.failed = false;

    JsonValue *root = parseValue(&parser);
    if (parser.failed) { freeJsonValue(root); return NULL; }

    skipWhitespace(&parser);
    if (*parser.cursor != '\0') {     /* extra non-space content after value */
        freeJsonValue(root);
        return NULL;
    }
    return root;
}
