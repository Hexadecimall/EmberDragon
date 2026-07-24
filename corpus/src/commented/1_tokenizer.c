/*
 * tokenizer.c
 *
 * A small word tokenizer that splits an input string into tokens separated by
 * a caller-supplied set of delimiter characters. Tokens are copied into a
 * dynamically grown array of heap-allocated strings; the caller takes
 * ownership of the result and frees it with freeTokenList().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A growable list of NUL-terminated token strings. */
typedef struct {
    char   **items;     /* array of owned C-strings                       */
    size_t   count;     /* number of tokens currently stored              */
    size_t   capacity;  /* number of slots allocated in `items`           */
} TokenList;

/*
 * Initialize an empty token list. Must be called before any push.
 * Sets every field to a zero/NULL starting state; allocates nothing.
 */
static void tokenListInit(TokenList *list) {
    list->items    = NULL;
    list->count    = 0;
    list->capacity = 0;
}

/*
 * Append one token to the list, growing the backing array if it is full.
 * `text` points at the start of the token and `length` is its byte count.
 * The token is copied (with an added NUL terminator) so the source buffer
 * may be modified or freed afterward. Returns 1 on success, 0 on allocation
 * failure. Amortized O(1) per push thanks to capacity doubling.
 */
static int tokenListPush(TokenList *list, const char *text, size_t length) {
    if (list->count == list->capacity) {
        /* Grow geometrically: start at 8, then double to keep pushes cheap. */
        size_t newCapacity = (list->capacity == 0) ? 8 : list->capacity * 2;
        char **grown = realloc(list->items, newCapacity * sizeof(char *));
        if (grown == NULL) {
            return 0;  /* leave the list intact so the caller can clean up */
        }
        list->items    = grown;
        list->capacity = newCapacity;
    }

    char *copy = malloc(length + 1);
    if (copy == NULL) {
        return 0;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';

    list->items[list->count] = copy;
    list->count += 1;
    return 1;
}

/*
 * Test whether `c` appears in the NUL-terminated `delimiters` string.
 * Returns 1 if `c` is a delimiter, 0 otherwise. O(d) in the delimiter count.
 */
static int isDelimiter(char c, const char *delimiters) {
    for (const char *d = delimiters; *d != '\0'; d++) {
        if (*d == c) {
            return 1;
        }
    }
    return 0;
}

/*
 * Split `input` into tokens, treating any run of delimiter characters as a
 * single separator (so empty tokens are never produced). Writes the result
 * into `out`, which must already be initialized. Returns the number of tokens
 * on success, or -1 if any allocation failed mid-way. O(n) over the input.
 */
int tokenize(const char *input, const char *delimiters, TokenList *out) {
    const char *start = NULL;  /* start of the token currently being scanned */

    for (const char *p = input; ; p++) {
        /* End the token at a delimiter or at the terminating NUL. */
        if (*p == '\0' || isDelimiter(*p, delimiters)) {
            if (start != NULL) {
                size_t length = (size_t)(p - start);
                if (!tokenListPush(out, start, length)) {
                    return -1;
                }
                start = NULL;  /* reset: we are back in "between tokens" state */
            }
            if (*p == '\0') {
                break;  /* the NUL is the only exit from the loop */
            }
        } else if (start == NULL) {
            start = p;  /* first non-delimiter byte begins a new token */
        }
    }

    return (int)out->count;
}

/*
 * Release every token string and the backing array, then reset the list to
 * the empty state. Safe to call on a zero-initialized or already-freed list.
 */
void freeTokenList(TokenList *list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    tokenListInit(list);
}
