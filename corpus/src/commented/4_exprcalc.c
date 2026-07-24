/*
 * exprcalc.c — A recursive-descent calculator for integer arithmetic expressions.
 *
 * Parses and evaluates expressions over 32-bit signed integers supporting
 * +, -, *, / and parenthesised grouping with the usual precedence rules.
 * The parser is a hand-written recursive-descent engine driven by a single
 * read cursor; it reports the first error it encounters via a status flag.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * Parser holds the immutable input string plus a moving read position and an
 * error flag. The flag is "sticky": once an error is seen it stays set so the
 * caller can check a single field after the whole parse completes.
 */
typedef struct Parser {
    const char *input;  /* null-terminated expression text (not owned) */
    int         pos;    /* index of the next character to read */
    int         error;  /* 0 = OK, 1 = syntax/divide-by-zero error */
} Parser;

/* Forward declaration: the grammar is mutually recursive (term -> factor ->
 * expression via parentheses), so the lowest level must see this name. */
static int parseExpression(Parser *p);

/*
 * Return the current character without consuming it, or '\0' at end of input.
 * Parameters: p — the parser. Returns the byte at the cursor. O(1).
 */
static char peek(Parser *p) {
    return p->input[p->pos];
}

/*
 * Advance the cursor past any run of ASCII spaces and tabs.
 * Parameters: p — the parser. Returns nothing. Leaves the cursor on the next
 * significant character (or the terminating null). O(k) in the whitespace run.
 */
static void skipSpaces(Parser *p) {
    while (peek(p) == ' ' || peek(p) == '\t')
        p->pos++;
}

/*
 * Parse a factor: either a parenthesised sub-expression or a non-negative
 * integer literal. Parameters: p — the parser. Returns the factor's value, or
 * 0 with p->error set on a malformed factor or missing close paren.
 */
static int parseFactor(Parser *p) {
    skipSpaces(p);
    if (peek(p) == '(') {
        p->pos++;                       /* consume '(' */
        int value = parseExpression(p);
        skipSpaces(p);
        if (peek(p) == ')')
            p->pos++;                   /* consume matching ')' */
        else
            p->error = 1;               /* unbalanced parentheses */
        return value;
    }

    /* Otherwise we must be looking at a digit; absence of one is an error. */
    if (peek(p) < '0' || peek(p) > '9') {
        p->error = 1;
        return 0;
    }

    int value = 0;
    while (peek(p) >= '0' && peek(p) <= '9') {
        value = value * 10 + (peek(p) - '0');  /* accumulate decimal digits */
        p->pos++;
    }
    return value;
}

/*
 * Parse a term: a sequence of factors joined by '*' or '/' (higher precedence
 * than +/-). Parameters: p — the parser. Returns the term's value. Sets
 * p->error on division by zero so the result is never undefined.
 */
static int parseTerm(Parser *p) {
    int value = parseFactor(p);
    skipSpaces(p);
    while (peek(p) == '*' || peek(p) == '/') {
        char op = peek(p);
        p->pos++;                       /* consume the operator */
        int rhs = parseFactor(p);
        if (op == '*') {
            value = value * rhs;
        } else {
            if (rhs == 0) {             /* guard against UB on /0 */
                p->error = 1;
                return 0;
            }
            value = value / rhs;
        }
        skipSpaces(p);
    }
    return value;
}

/*
 * Parse an expression: terms joined by '+' or '-' (lowest precedence).
 * Parameters: p — the parser. Returns the expression's value. This is the
 * grammar's entry rule and the target of the recursive parentheses call.
 */
static int parseExpression(Parser *p) {
    int value = parseTerm(p);
    skipSpaces(p);
    while (peek(p) == '+' || peek(p) == '-') {
        char op = peek(p);
        p->pos++;                       /* consume the operator */
        int rhs = parseTerm(p);
        value = (op == '+') ? value + rhs : value - rhs;
        skipSpaces(p);
    }
    return value;
}

/*
 * Public entry point: evaluate a full expression string.
 * Parameters: text — the expression; outError — set to 1 on any failure,
 * 0 on success (may be NULL). Returns the computed value (0 on error).
 * Requires the entire string to be consumed; trailing junk is an error.
 */
int evaluate(const char *text, int *outError) {
    Parser p = { text, 0, 0 };
    int result = parseExpression(&p);
    skipSpaces(&p);
    /* Any leftover characters mean the grammar did not cover the whole input. */
    if (peek(&p) != '\0')
        p.error = 1;
    if (outError)
        *outError = p.error;
    return p.error ? 0 : result;
}
