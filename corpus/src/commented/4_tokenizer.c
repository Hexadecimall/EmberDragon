/*
 * tokenizer.c — A lexical scanner that splits source text into tokens.
 *
 * Converts a stream of characters into a sequence of typed tokens (integer
 * literals, identifiers, operators, parentheses) while skipping whitespace.
 * Each token records its type, its numeric value (for literals) and its source
 * span, so a downstream parser can consume them without rescanning characters.
 */

#include <stddef.h>

/* The kinds of token the scanner can emit. */
typedef enum TokenType {
    TOK_EOF,      /* end of input */
    TOK_NUMBER,   /* a run of decimal digits */
    TOK_IDENT,    /* identifier: letter/underscore then letters/digits/_ */
    TOK_PLUS,     /* '+' */
    TOK_MINUS,    /* '-' */
    TOK_STAR,     /* '*' */
    TOK_SLASH,    /* '/' */
    TOK_LPAREN,   /* '(' */
    TOK_RPAREN,   /* ')' */
    TOK_UNKNOWN   /* any character the scanner does not recognise */
} TokenType;

/*
 * A single lexical token. For TOK_NUMBER, value holds the decoded integer; for
 * all kinds, start/length describe the token's byte span in the source.
 */
typedef struct Token {
    TokenType type;
    int       value;   /* decoded value for TOK_NUMBER, else 0 */
    int       start;   /* offset of the first character in the source */
    int       length;  /* number of source characters spanned */
} Token;

/* The scanner cursor over an immutable source buffer. */
typedef struct Lexer {
    const char *src;   /* null-terminated source text (not owned) */
    int         pos;   /* index of the next character to scan */
} Lexer;

/*
 * Test whether a character may begin an identifier (letter or underscore).
 * Parameters: c — the character. Returns nonzero if it can start an ident. O(1).
 */
static int isIdentStart(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/*
 * Test whether a character may continue an identifier (ident-start or digit).
 * Parameters: c — the character. Returns nonzero if valid in an ident body. O(1).
 */
static int isIdentPart(char c) {
    return isIdentStart(c) || (c >= '0' && c <= '9');
}

/*
 * Initialise a lexer over a source string.
 * Parameters: lex — the lexer; source — null-terminated text. Returns nothing.
 * The lexer borrows the string; the caller must keep it alive. O(1).
 */
void lexerInit(Lexer *lex, const char *source) {
    lex->src = source;
    lex->pos = 0;
}

/*
 * Advance the cursor past spaces, tabs, carriage returns and newlines.
 * Parameters: lex — the lexer. Returns nothing. Leaves the cursor on the next
 * significant byte or the terminating null. O(k) in the whitespace run.
 */
static void skipWhitespace(Lexer *lex) {
    char c;
    while ((c = lex->src[lex->pos]) == ' ' || c == '\t' ||
           c == '\r' || c == '\n')
        lex->pos++;
}

/*
 * Scan and return the next token from the source.
 * Parameters: lex — the lexer. Returns the token; at end of input it returns a
 * TOK_EOF token with zero length. Multi-character runs (numbers, identifiers)
 * are consumed greedily. Unrecognised bytes yield a one-character TOK_UNKNOWN
 * so scanning always makes progress. O(token length).
 */
Token lexerNext(Lexer *lex) {
    skipWhitespace(lex);

    Token tok;
    tok.value = 0;
    tok.start = lex->pos;
    tok.length = 0;

    char c = lex->src[lex->pos];
    if (c == '\0') {
        tok.type = TOK_EOF;
        return tok;            /* no bytes consumed at EOF */
    }

    /* Number: greedily consume digits and decode the value as we go. */
    if (c >= '0' && c <= '9') {
        int value = 0;
        while ((c = lex->src[lex->pos]) >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
            lex->pos++;
        }
        tok.type = TOK_NUMBER;
        tok.value = value;
        tok.length = lex->pos - tok.start;
        return tok;
    }

    /* Identifier: a start character followed by ident-part characters. */
    if (isIdentStart(c)) {
        while (isIdentPart(lex->src[lex->pos]))
            lex->pos++;
        tok.type = TOK_IDENT;
        tok.length = lex->pos - tok.start;
        return tok;
    }

    /* Single-character punctuation and operators. */
    lex->pos++;               /* consume the one character below */
    switch (c) {
    case '+': tok.type = TOK_PLUS;   break;
    case '-': tok.type = TOK_MINUS;  break;
    case '*': tok.type = TOK_STAR;   break;
    case '/': tok.type = TOK_SLASH;  break;
    case '(': tok.type = TOK_LPAREN; break;
    case ')': tok.type = TOK_RPAREN; break;
    default:  tok.type = TOK_UNKNOWN; break;  /* unrecognised byte */
    }
    tok.length = 1;
    return tok;
}

/*
 * Tokenize an entire source string into a caller-provided buffer.
 * Parameters: source — the text; out — array to fill; maxTokens — capacity of
 * out. Returns the number of tokens written, always including a final TOK_EOF
 * unless the buffer fills first. Stops early when maxTokens is reached. O(n).
 */
int tokenizeAll(const char *source, Token *out, int maxTokens) {
    Lexer lex;
    lexerInit(&lex, source);

    int n = 0;
    while (n < maxTokens) {
        Token t = lexerNext(&lex);
        out[n++] = t;
        if (t.type == TOK_EOF)
            break;            /* EOF is the sentinel that ends the stream */
    }
    return n;
}
