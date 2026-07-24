#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* A recursive-descent integer expression evaluator.
 * Grammar: expr = term (('+'|'-') term)*
 *          term = factor (('*'|'/'|'%') factor)*
 *          factor = number | '(' expr ')' | '-' factor
 */

typedef struct {
    const char *cursor;
    int hadError;
} Parser;

static void skipSpaces(Parser *parser) {
    while (*parser->cursor == ' ' || *parser->cursor == '\t')
        parser->cursor++;
}

static int parseExpr(Parser *parser);

static int parseNumber(Parser *parser) {
    int value = 0;
    int digits = 0;
    while (*parser->cursor >= '0' && *parser->cursor <= '9') {
        value = value * 10 + (*parser->cursor - '0');
        parser->cursor++;
        digits++;
    }
    if (digits == 0)
        parser->hadError = 1;
    return value;
}

static int parseFactor(Parser *parser) {
    skipSpaces(parser);
    if (*parser->cursor == '-') {
        parser->cursor++;
        return -parseFactor(parser);
    }
    if (*parser->cursor == '(') {
        parser->cursor++;
        int inner = parseExpr(parser);
        skipSpaces(parser);
        if (*parser->cursor == ')')
            parser->cursor++;
        else
            parser->hadError = 1;
        return inner;
    }
    return parseNumber(parser);
}

static int parseTerm(Parser *parser) {
    int accumulator = parseFactor(parser);
    skipSpaces(parser);
    while (*parser->cursor == '*' || *parser->cursor == '/' || *parser->cursor == '%') {
        char op = *parser->cursor;
        parser->cursor++;
        int right = parseFactor(parser);
        if (op == '*') {
            accumulator *= right;
        } else if (right == 0) {
            parser->hadError = 1;
        } else if (op == '/') {
            accumulator /= right;
        } else {
            accumulator %= right;
        }
        skipSpaces(parser);
    }
    return accumulator;
}

static int parseExpr(Parser *parser) {
    int accumulator = parseTerm(parser);
    skipSpaces(parser);
    while (*parser->cursor == '+' || *parser->cursor == '-') {
        char op = *parser->cursor;
        parser->cursor++;
        int right = parseTerm(parser);
        if (op == '+')
            accumulator += right;
        else
            accumulator -= right;
        skipSpaces(parser);
    }
    return accumulator;
}

int evaluateExpression(const char *text, int *outError) {
    Parser parser;
    parser.cursor = text;
    parser.hadError = 0;
    int result = parseExpr(&parser);
    skipSpaces(&parser);
    if (*parser.cursor != '\0')
        parser.hadError = 1;
    *outError = parser.hadError;
    return result;
}
