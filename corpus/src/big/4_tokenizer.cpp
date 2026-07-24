/*
 * tokenizer.cpp — A lexer for a small C-like expression language.
 *
 * Scans source text into a stream of tokens (identifiers, integer literals,
 * arithmetic/comparison operators, and parentheses) using a single-pass
 * character state machine. Whitespace separates tokens and is otherwise
 * discarded. Token text is copied into each Token, so the produced array does
 * not alias the original source buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TOKEN_TEXT_MAX 32
#define TOKEN_LIST_MAX 128

/* The lexical category assigned to each token. */
enum TokenKind {
    TOKEN_IDENTIFIER, /* a name: letter or '_' followed by letters/digits/'_' */
    TOKEN_NUMBER,     /* a run of decimal digits                              */
    TOKEN_OPERATOR,   /* one of + - * / and the relational ops < > <= >= == != */
    TOKEN_LPAREN,     /* '('                                                  */
    TOKEN_RPAREN,     /* ')'                                                  */
    TOKEN_ERROR       /* an unexpected character                             */
};

/*
 * A single token: its category, a copied text spelling, and the byte offset in
 * the source where it began (useful for error reporting).
 */
struct Token {
    TokenKind kind;
    char      text[TOKEN_TEXT_MAX];
    int       position;
};

/* A bounded list of tokens plus how many were produced. */
struct TokenList {
    Token tokens[TOKEN_LIST_MAX];
    int   count;
};

/*
 * isIdentStart — Test whether a character may begin an identifier.
 *
 * Returns true for ASCII letters and the underscore, false otherwise.
 */
static bool isIdentStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/*
 * isIdentPart — Test whether a character may continue an identifier.
 *
 * Returns true for anything valid as a start character plus the decimal
 * digits, false otherwise.
 */
static bool isIdentPart(char c) {
    return isIdentStart(c) || (c >= '0' && c <= '9');
}

/*
 * isDigit — Test whether a character is a decimal digit.
 *
 * Returns true for '0' through '9', false otherwise.
 */
static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

/*
 * emitToken — Append a token spanning source[start, end) to the list.
 *
 * Copies up to TOKEN_TEXT_MAX-1 characters of the slice into a new token of
 * the given kind, records its starting position, and bumps the count. Returns
 * true on success or false if the list is full (the token is dropped).
 */
static bool emitToken(TokenList *list, TokenKind kind,
                      const char *source, int start, int end) {
    if (list->count >= TOKEN_LIST_MAX) return false;
    Token *token = &list->tokens[list->count];
    token->kind = kind;
    token->position = start;

    int length = end - start;
    if (length >= TOKEN_TEXT_MAX) length = TOKEN_TEXT_MAX - 1; /* truncate */
    memcpy(token->text, source + start, (size_t)length);
    token->text[length] = '\0';

    list->count++;
    return true;
}

/*
 * tokenize — Convert a source string into a TokenList.
 *
 * Walks source once, dispatching on the leading character of each token:
 * identifiers, numbers, parentheses, and one- or two-character operators. On
 * an unexpected character it emits a single TOKEN_ERROR token and stops.
 * Initializes *list and returns its final token count.
 */
int tokenize(TokenList *list, const char *source) {
    list->count = 0;
    int i = 0;

    while (source[i] != '\0') {
        char c = source[i];

        /* Whitespace is a token separator and carries no token of its own. */
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            i++;
            continue;
        }

        if (isIdentStart(c)) {
            int start = i;
            while (isIdentPart(source[i])) i++;     /* greedy identifier run */
            if (!emitToken(list, TOKEN_IDENTIFIER, source, start, i)) break;
            continue;
        }

        if (isDigit(c)) {
            int start = i;
            while (isDigit(source[i])) i++;         /* greedy digit run */
            if (!emitToken(list, TOKEN_NUMBER, source, start, i)) break;
            continue;
        }

        if (c == '(') {
            if (!emitToken(list, TOKEN_LPAREN, source, i, i + 1)) break;
            i++;
            continue;
        }
        if (c == ')') {
            if (!emitToken(list, TOKEN_RPAREN, source, i, i + 1)) break;
            i++;
            continue;
        }

        /* Two-character relational operators: <=, >=, ==, != take priority
         * over their single-character forms when the second char matches. */
        if ((c == '<' || c == '>' || c == '=' || c == '!') && source[i + 1] == '=') {
            if (!emitToken(list, TOKEN_OPERATOR, source, i, i + 2)) break;
            i += 2;
            continue;
        }

        /* Single-character operators. '=' alone is intentionally not valid
         * here (this language has no assignment), so it falls through. */
        if (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '<' || c == '>') {
            if (!emitToken(list, TOKEN_OPERATOR, source, i, i + 1)) break;
            i++;
            continue;
        }

        /* Anything else is a lexical error; record it and stop scanning. */
        emitToken(list, TOKEN_ERROR, source, i, i + 1);
        break;
    }

    return list->count;
}

/*
 * countTokensOfKind — Count how many tokens in a list have a given kind.
 *
 * Returns the number of matching tokens in O(n). Handy for quick assertions
 * such as verifying parenthesis balance against LPAREN/RPAREN counts.
 */
int countTokensOfKind(const TokenList *list, TokenKind kind) {
    int matches = 0;
    for (int i = 0; i < list->count; i++) {
        if (list->tokens[i].kind == kind) matches++;
    }
    return matches;
}
