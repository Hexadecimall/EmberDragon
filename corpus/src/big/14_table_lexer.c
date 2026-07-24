/*
 * table_lexer.c
 *
 * A table-driven lexer that recognizes integers, identifiers, operators, and
 * whitespace using an explicit state-transition table. Driving a DFA from a
 * table (rather than hand-written if/else chains) keeps the scanner compact
 * and makes the recognized language easy to audit at a glance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Character classes: every input byte is folded into one of these buckets so
 * the transition table can stay small (states x classes) instead of
 * states x 256. */
enum CharClass {
    CC_DIGIT = 0,   /* '0'..'9' */
    CC_ALPHA,       /* 'A'..'Z', 'a'..'z', '_' */
    CC_SPACE,       /* space, tab, newline, carriage return */
    CC_OP,          /* + - * / = < > etc. */
    CC_OTHER,       /* anything else (punctuation, control bytes) */
    CC_COUNT
};

/* Lexer states. ST_START is the entry; the others accumulate a token until a
 * byte that no longer extends it is seen. */
enum LexState {
    ST_START = 0,
    ST_IN_NUM,
    ST_IN_IDENT,
    ST_IN_SPACE,
    ST_IN_OP,
    ST_COUNT
};

/* The kinds of token the lexer can emit. */
enum TokenKind {
    TOK_NUMBER,
    TOK_IDENT,
    TOK_SPACE,
    TOK_OPERATOR,
    TOK_UNKNOWN,
    TOK_EOF
};

/* One lexical token: a slice of the source plus its classification. The
 * 'start' pointer aliases into the original buffer (no copy is made). */
typedef struct Token {
    enum TokenKind kind;
    const char    *start;   /* first byte of the lexeme in the source */
    int            length;  /* number of bytes in the lexeme */
} Token;

/*
 * Map a raw byte to its character class.
 * Returns one of the CC_* values; never fails (unknown bytes fold to CC_OTHER).
 */
static enum CharClass classify(unsigned char ch) {
    if (ch >= '0' && ch <= '9') return CC_DIGIT;
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_')
        return CC_ALPHA;
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') return CC_SPACE;
    if (strchr("+-*/=<>!&|%^", (char)ch) != NULL) return CC_OP;
    return CC_OTHER;
}

/* Transition table: transition[state][class] -> next state, or ST_COUNT to
 * signal "this byte does not belong to the current token; stop here". The
 * table encodes the full lexer DFA in one readable block. */
static const int transition[ST_COUNT][CC_COUNT] = {
    /*               DIGIT      ALPHA      SPACE      OP        OTHER   */
    /* START  */  { ST_IN_NUM,  ST_IN_IDENT, ST_IN_SPACE, ST_IN_OP, ST_COUNT },
    /* IN_NUM */  { ST_IN_NUM,  ST_COUNT,    ST_COUNT,    ST_COUNT, ST_COUNT },
    /* IN_IDENT*/ { ST_IN_IDENT,ST_IN_IDENT, ST_COUNT,    ST_COUNT, ST_COUNT },
    /* IN_SPACE*/ { ST_COUNT,   ST_COUNT,    ST_IN_SPACE, ST_COUNT, ST_COUNT },
    /* IN_OP  */  { ST_COUNT,   ST_COUNT,    ST_COUNT,    ST_IN_OP, ST_COUNT },
};

/*
 * Translate a terminal lexer state into the token kind it represents.
 * Returns the matching TOK_* value; ST_START maps to TOK_UNKNOWN because a
 * token that never left the start state consumed an unclassifiable byte.
 */
static enum TokenKind state_to_kind(enum LexState st) {
    switch (st) {
        case ST_IN_NUM:   return TOK_NUMBER;
        case ST_IN_IDENT: return TOK_IDENT;
        case ST_IN_SPACE: return TOK_SPACE;
        case ST_IN_OP:    return TOK_OPERATOR;
        default:          return TOK_UNKNOWN;
    }
}

/*
 * Scan a single token starting at '*cursor' and advance the cursor past it.
 * Returns the token; sets kind to TOK_EOF (with length 0) at end of input.
 * A lone unclassifiable byte is returned as a one-byte TOK_UNKNOWN so the
 * caller can decide how to report errors. Each call is O(token length).
 */
static Token lex_next(const char **cursor) {
    const char *p = *cursor;
    Token tok;
    tok.start = p;
    tok.length = 0;

    if (*p == '\0') {                 /* nothing left to scan */
        tok.kind = TOK_EOF;
        return tok;
    }

    enum LexState st = ST_START;
    enum LexState last = ST_START;    /* last non-terminal state we were in */

    while (*p != '\0') {
        enum CharClass cc = classify((unsigned char)*p);
        int nxt = transition[st][cc];
        if (nxt == ST_COUNT) break;   /* byte does not extend the token */
        last = (enum LexState)nxt;
        st = (enum LexState)nxt;
        p++;
    }

    /* If the very first byte was unclassifiable we never advanced; emit it as
     * a single-byte unknown token so the scanner always makes progress. */
    if (p == *cursor) {
        tok.kind = TOK_UNKNOWN;
        tok.length = 1;
        *cursor = p + 1;
        return tok;
    }

    tok.kind = state_to_kind(last);
    tok.length = (int)(p - *cursor);
    *cursor = p;
    return tok;
}

/*
 * Tokenize an entire NUL-terminated source string into 'out', skipping
 * whitespace tokens. Stops at end of input or when 'max' tokens have been
 * produced. Returns the number of tokens written. O(length of source).
 */
static int lex_all(const char *source, Token *out, int max) {
    const char *cursor = source;
    int n = 0;
    while (n < max) {
        Token t = lex_next(&cursor);
        if (t.kind == TOK_EOF) break;
        if (t.kind == TOK_SPACE) continue;   /* whitespace is not significant */
        out[n++] = t;
    }
    return n;
}
