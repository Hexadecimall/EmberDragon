/*
 * query_string.c — A URL query-string parser with percent-decoding.
 *
 * Splits a "key=value&key=value" query string into decoded key/value pairs,
 * translating '+' to a space and "%XX" hex escapes back to their byte values.
 * Decoding happens into a caller-supplied arena buffer so the parser performs
 * no heap allocation and the pairs stay valid for the lifetime of that arena.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define QS_MAX_PAIRS 32

/* One decoded key/value pair; both point into the QueryString arena. */
typedef struct QueryPair {
    const char *key;
    const char *value;               /* empty string when "key" has no '='  */
} QueryPair;

/*
 * QueryString owns the decoded text arena and the table of pairs that point
 * into it. The pairs are only valid while this struct is alive.
 */
typedef struct QueryString {
    char      arena[1024];           /* packed, NUL-separated decoded strings */
    int       used;                  /* bytes consumed in arena               */
    QueryPair pairs[QS_MAX_PAIRS];
    int       pairCount;
} QueryString;

/*
 * hexDigit — Convert a single hex character to its 0-15 value.
 *
 * Accepts '0'-'9', 'a'-'f', and 'A'-'F'. Returns the numeric value, or -1 if
 * the character is not a hex digit.
 */
static int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/*
 * decodeComponent — Percent-decode one key or value into the arena.
 *
 * Copies bytes from [begin, end) into qs->arena, converting '+' to ' ' and a
 * well-formed "%XX" sequence to the corresponding byte; a malformed escape is
 * copied through verbatim. Returns a pointer to the NUL-terminated decoded
 * string within the arena, or NULL if the arena is exhausted.
 */
static const char *decodeComponent(QueryString *qs, const char *begin, const char *end) {
    char *out = &qs->arena[qs->used];
    char *write = out;
    char *limit = &qs->arena[sizeof(qs->arena)];

    const char *p = begin;
    while (p < end) {
        if (write + 1 >= limit) return NULL; /* leave room for the NUL */
        if (*p == '+') {
            *write++ = ' ';
            p++;
        } else if (*p == '%') {
            /* Decode "%XX" only when two hex digits actually follow. */
            int hi = (p + 1 < end) ? hexDigit(p[1]) : -1;
            int lo = (p + 2 < end) ? hexDigit(p[2]) : -1;
            if (hi >= 0 && lo >= 0) {
                *write++ = (char)((hi << 4) | lo);
                p += 3;
            } else {
                *write++ = *p++;     /* malformed escape: keep the '%' literal */
            }
        } else {
            *write++ = *p++;
        }
    }
    if (write + 1 >= limit) return NULL;
    *write = '\0';
    qs->used = (int)(write - qs->arena) + 1; /* advance past the NUL */
    return out;
}

/*
 * qsParse — Parse and decode a full query string.
 *
 * Splits text on '&' into components, each split once on its first '=' into a
 * decoded key and value (a missing '=' yields an empty value). Initializes qs
 * and fills its pairs. Returns the number of pairs parsed, or -1 if the input
 * exceeds QS_MAX_PAIRS or the decode arena overflows. A leading '?' is skipped.
 */
int qsParse(QueryString *qs, const char *text) {
    qs->used = 0;
    qs->pairCount = 0;
    if (*text == '?') text++;        /* tolerate a leading '?' */

    const char *p = text;
    while (*p != '\0') {
        /* Find the end of this component (up to '&' or end of string). */
        const char *segmentEnd = p;
        while (*segmentEnd != '\0' && *segmentEnd != '&') segmentEnd++;

        /* An empty segment (e.g. "a=1&&b=2") contributes no pair. */
        if (segmentEnd != p) {
            const char *equals = p;
            while (equals < segmentEnd && *equals != '=') equals++;

            if (qs->pairCount >= QS_MAX_PAIRS) return -1;
            QueryPair *pair = &qs->pairs[qs->pairCount];

            /* Key runs from p up to '=' (or the whole segment if no '='). */
            pair->key = decodeComponent(qs, p, equals);
            if (pair->key == NULL) return -1;

            if (equals < segmentEnd) {
                /* There is a '=': value is everything after it. */
                pair->value = decodeComponent(qs, equals + 1, segmentEnd);
            } else {
                /* No '=': value is the empty string (decode an empty range). */
                pair->value = decodeComponent(qs, segmentEnd, segmentEnd);
            }
            if (pair->value == NULL) return -1;
            qs->pairCount++;
        }

        if (*segmentEnd == '\0') break;
        p = segmentEnd + 1;          /* step past the '&' */
    }
    return qs->pairCount;
}

/*
 * qsGet — Look up the decoded value for a key.
 *
 * Linearly scans the parsed pairs and returns the value of the first matching
 * key, or NULL if the key is absent. The returned pointer is owned by qs.
 */
const char *qsGet(const QueryString *qs, const char *key) {
    for (int i = 0; i < qs->pairCount; i++) {
        if (strcmp(qs->pairs[i].key, key) == 0) {
            return qs->pairs[i].value;
        }
    }
    return NULL;
}
