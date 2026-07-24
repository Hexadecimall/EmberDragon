#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A lexical scanner driven by an explicit state machine.
 * It classifies an input stream into numbers, identifiers, and operators.
 */

typedef enum {
    STATE_START,
    STATE_IN_NUMBER,
    STATE_IN_IDENT,
    STATE_DONE
} ScanState;

typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENT,
    TOKEN_OPERATOR,
    TOKEN_END,
    TOKEN_INVALID
} TokenKind;

typedef struct {
    TokenKind kind;
    int intValue;
    char text[24];
} Token;

typedef struct {
    const char *input;
    int position;
} Lexer;

static int isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

static int isAlphaChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int isOperatorChar(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>';
}

void lexerInit(Lexer *lexer, const char *source) {
    lexer->input = source;
    lexer->position = 0;
}

Token lexerNext(Lexer *lexer) {
    Token token;
    token.kind = TOKEN_INVALID;
    token.intValue = 0;
    token.text[0] = '\0';

    ScanState state = STATE_START;
    int writeIndex = 0;

    while (state != STATE_DONE) {
        char c = lexer->input[lexer->position];
        switch (state) {
            case STATE_START:
                if (c == '\0') {
                    token.kind = TOKEN_END;
                    state = STATE_DONE;
                } else if (c == ' ' || c == '\t' || c == '\n') {
                    lexer->position++;
                } else if (isDigitChar(c)) {
                    state = STATE_IN_NUMBER;
                } else if (isAlphaChar(c)) {
                    state = STATE_IN_IDENT;
                } else if (isOperatorChar(c)) {
                    token.kind = TOKEN_OPERATOR;
                    token.text[0] = c;
                    token.text[1] = '\0';
                    lexer->position++;
                    state = STATE_DONE;
                } else {
                    token.kind = TOKEN_INVALID;
                    lexer->position++;
                    state = STATE_DONE;
                }
                break;
            case STATE_IN_NUMBER:
                if (isDigitChar(c) && writeIndex < 23) {
                    token.intValue = token.intValue * 10 + (c - '0');
                    token.text[writeIndex++] = c;
                    lexer->position++;
                } else {
                    token.text[writeIndex] = '\0';
                    token.kind = TOKEN_NUMBER;
                    state = STATE_DONE;
                }
                break;
            case STATE_IN_IDENT:
                if ((isAlphaChar(c) || isDigitChar(c)) && writeIndex < 23) {
                    token.text[writeIndex++] = c;
                    lexer->position++;
                } else {
                    token.text[writeIndex] = '\0';
                    token.kind = TOKEN_IDENT;
                    state = STATE_DONE;
                }
                break;
            case STATE_DONE:
                break;
        }
    }
    return token;
}

int countTokens(const char *source) {
    Lexer lexer;
    lexerInit(&lexer, source);
    int total = 0;
    Token token = lexerNext(&lexer);
    while (token.kind != TOKEN_END) {
        total++;
        token = lexerNext(&lexer);
    }
    return total;
}
