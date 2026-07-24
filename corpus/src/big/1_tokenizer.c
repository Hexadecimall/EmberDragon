/*
 * tokenizer.c — A small lexer for a simple arithmetic expression language.
 *
 * Scans an input string into a stream of tokens (numbers, identifiers,
 * operators, and parentheses), skipping whitespace and recognizing
 * multi-character integer literals and identifiers. The tokenizer is the
 * front end any expression parser or interpreter would build upon.
 */

#include <stdlib.h>
#include <string.h>

/* The kinds of lexical token this scanner can produce. */
typedef enum TokenType {
    TOKEN_NUMBER,     /* A run of decimal digits, value stored separately. */
    TOKEN_IDENTIFIER, /* A letter followed by letters/digits/underscores. */
    TOKEN_PLUS,       /* '+' */
    TOKEN_MINUS,      /* '-' */
    TOKEN_STAR,       /* '*' */
    TOKEN_SLASH,      /* '/' */
    TOKEN_LPAREN,     /* '(' */
    TOKEN_RPAREN,     /* ')' */
    TOKEN_END,        /* Sentinel emitted at end of input. */
    TOKEN_ERROR       /* An unrecognized character was encountered. */
} TokenType;

/*
 * A single token. `start` and `length` point into the original input buffer
 * (no copy is made), and `value` carries the parsed integer for TOKEN_NUMBER.
 */
typedef struct Token {
    TokenType type;
    const char *start;
    int length;
    long value;
} Token;

/*
 * Cursor over the source text. `current` advances as tokens are consumed.
 * The lexer never looks backward, so a single forward pointer suffices.
 */
typedef struct Lexer {
    const char *current;
} Lexer;

/*
 * Initialize a lexer to scan `source` from the beginning.
 *
 * Parameters:
 *   lexer  - storage to initialize (caller-owned).
 *   source - NUL-terminated input; must outlive the lexer since tokens point
 *            into it.
 */
void lexer_init(Lexer *lexer, const char *source) {
    lexer->current = source;
}

/* Return 1 if `c` is an ASCII decimal digit, else 0. */
static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

/* Return 1 if `c` may start an identifier (a letter or underscore), else 0. */
static int is_identifier_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/* Return 1 if `c` may continue an identifier (start chars plus digits). */
static int is_identifier_part(char c) {
    return is_identifier_start(c) || is_digit(c);
}

/*
 * Advance the lexer past any spaces, tabs, and newlines.
 *
 * Leaves `current` pointing at the next significant character (possibly the
 * terminating NUL). Whitespace carries no semantic meaning in this grammar.
 */
static void skip_whitespace(Lexer *lexer) {
    while (*lexer->current == ' ' || *lexer->current == '\t' ||
           *lexer->current == '\n' || *lexer->current == '\r') {
        lexer->current++;
    }
}

/*
 * Build a single-character token of the given type and advance the cursor.
 *
 * Returns the constructed token. Used for operators and parentheses where the
 * lexeme is always exactly one character.
 */
static Token make_single(Lexer *lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->current;
    token.length = 1;
    token.value = 0;
    lexer->current++;        /* Consume the single character. */
    return token;
}

/*
 * Scan a decimal integer literal starting at the cursor.
 *
 * Returns a TOKEN_NUMBER whose `value` is the accumulated integer. Assumes the
 * caller has already verified the first character is a digit. Overflow wraps
 * silently in `long`, matching typical C integer semantics.
 */
static Token scan_number(Lexer *lexer) {
    Token token;
    token.type = TOKEN_NUMBER;
    token.start = lexer->current;
    token.value = 0;
    while (is_digit(*lexer->current)) {
        /* Horner-style accumulation: shift left one decimal place, add digit. */
        token.value = token.value * 10 + (*lexer->current - '0');
        lexer->current++;
    }
    token.length = (int)(lexer->current - token.start);
    return token;
}

/*
 * Scan an identifier starting at the cursor.
 *
 * Returns a TOKEN_IDENTIFIER spanning the full name. Assumes the first
 * character is a valid identifier start. The lexeme is delimited by `start`
 * and `length`; the lexer does not intern or copy it.
 */
static Token scan_identifier(Lexer *lexer) {
    Token token;
    token.type = TOKEN_IDENTIFIER;
    token.start = lexer->current;
    token.value = 0;
    while (is_identifier_part(*lexer->current)) {
        lexer->current++;
    }
    token.length = (int)(lexer->current - token.start);
    return token;
}

/*
 * Produce the next token from the input stream.
 *
 * Returns the next Token. At end of input it returns TOKEN_END repeatedly;
 * on an unexpected character it returns TOKEN_ERROR spanning that character
 * and advances past it so callers can recover. Complexity: O(token length).
 */
Token lexer_next(Lexer *lexer) {
    skip_whitespace(lexer);

    char c = *lexer->current;
    if (c == '\0') {
        Token end = { TOKEN_END, lexer->current, 0, 0 };
        return end;          /* Stable terminator; cursor stays put. */
    }

    if (is_digit(c)) {
        return scan_number(lexer);
    }
    if (is_identifier_start(c)) {
        return scan_identifier(lexer);
    }

    /* Single-character operators and grouping symbols. */
    switch (c) {
        case '+': return make_single(lexer, TOKEN_PLUS);
        case '-': return make_single(lexer, TOKEN_MINUS);
        case '*': return make_single(lexer, TOKEN_STAR);
        case '/': return make_single(lexer, TOKEN_SLASH);
        case '(': return make_single(lexer, TOKEN_LPAREN);
        case ')': return make_single(lexer, TOKEN_RPAREN);
        default:  return make_single(lexer, TOKEN_ERROR);
    }
}

/*
 * Count the number of tokens in `source`, excluding the terminating TOKEN_END.
 *
 * Returns the token count. Stops early and returns the count so far if a
 * TOKEN_ERROR is hit, since the remaining stream is unreliable.
 */
int tokenize_count(const char *source) {
    Lexer lexer;
    lexer_init(&lexer, source);
    int count = 0;
    for (;;) {
        Token token = lexer_next(&lexer);
        if (token.type == TOKEN_END || token.type == TOKEN_ERROR) {
            break;
        }
        count++;
    }
    return count;
}
