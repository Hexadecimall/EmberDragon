/*
 * glob_match.c
 *
 * Shell-style glob matching with the classic three wildcards: '*' (any run of
 * characters, including none), '?' (exactly one character), and '[...]'
 * character classes with ranges and negation. Implemented iteratively with a
 * single backtracking checkpoint for '*', which keeps it O(n*m) worst case
 * without recursion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Match one '[...]' character class against the byte 'c'.
 * On entry 'class_start' points at the byte just after '['. On a successful
 * parse, '*end' is set to the byte just past the closing ']'. Supports a
 * leading '!' or '^' for negation and 'a-z' style ranges.
 * Returns 1 if 'c' is in the class, 0 if not. If the class is malformed
 * (no closing ']'), returns 0 and leaves *end at class_start.
 */
static int match_class(const char *class_start, unsigned char c, const char **end) {
    const char *p = class_start;
    int negate = 0;
    int matched = 0;

    /* A leading '!' or '^' inverts the membership test. */
    if (*p == '!' || *p == '^') { negate = 1; p++; }

    /* A ']' immediately after the (optional) negation is a literal ']'. */
    if (*p == ']') {
        if (c == ']') matched = 1;
        p++;
    }

    while (*p && *p != ']') {
        if (p[1] == '-' && p[2] && p[2] != ']') {
            /* Range form 'lo-hi'. Compare inclusively as unsigned bytes. */
            unsigned char lo = (unsigned char)p[0];
            unsigned char hi = (unsigned char)p[2];
            if (lo <= c && c <= hi) matched = 1;
            p += 3;
        } else {
            /* Plain single-character class member. */
            if ((unsigned char)*p == c) matched = 1;
            p++;
        }
    }

    if (*p != ']') {           /* unterminated class: treat as no match */
        *end = class_start;
        return 0;
    }
    *end = p + 1;              /* skip past the closing ']' */
    return negate ? !matched : matched;
}

/*
 * Test whether 'pattern' matches the whole string 'text'.
 * Returns 1 on a full match, 0 otherwise. The algorithm walks both strings
 * once, and whenever it meets a '*' it records a backtrack point; if a later
 * mismatch occurs it rewinds to that '*' and lets it swallow one more byte.
 * This bounds the work at O(len(text) * len(pattern)).
 */
static int glob_match(const char *pattern, const char *text) {
    const char *p = pattern;
    const char *t = text;
    const char *star = NULL;    /* last '*' seen in the pattern, or NULL */
    const char *star_text = NULL; /* text position when that '*' was taken */

    while (*t) {
        if (*p == '*') {
            /* Remember this star and the current text spot; tentatively let
             * the star match zero characters and move past it. */
            star = p++;
            star_text = t;
        } else if (*p == '?') {
            /* '?' consumes exactly one byte unconditionally. */
            p++; t++;
        } else if (*p == '[') {
            const char *end;
            if (match_class(p + 1, (unsigned char)*t, &end)) {
                p = end; t++;          /* class matched: advance both */
            } else if (star) {
                p = star + 1; t = ++star_text;  /* backtrack: star eats one more */
            } else {
                return 0;
            }
        } else if ((unsigned char)*p == (unsigned char)*t) {
            p++; t++;                  /* literal byte matched */
        } else if (star) {
            /* Mismatch but we have a star to fall back on: have it consume the
             * next text byte and retry the pattern after the star. */
            p = star + 1;
            t = ++star_text;
        } else {
            return 0;                  /* hard mismatch, nothing to backtrack to */
        }
    }

    /* Text exhausted. Any trailing '*'s in the pattern can match empty, so
     * skip over them; a full match requires the pattern to be fully consumed. */
    while (*p == '*') p++;
    return *p == '\0';
}

/*
 * Convenience predicate: does 'name' match any of 'count' glob patterns?
 * Returns the 0-based index of the first matching pattern, or -1 if none
 * match. Useful for .gitignore-style "match against a list" checks.
 */
static int glob_match_any(const char *const *patterns, int count, const char *name) {
    for (int i = 0; i < count; i++)
        if (glob_match(patterns[i], name))
            return i;
    return -1;
}
