#include <stdint.h>

/* Recursive-descent parser for signed integer expressions: + - * with parens. */

typedef struct Parser {
    const char *input;
    int position;
    int error;
} Parser;

static char peek_char(Parser *parser) {
    return parser->input[parser->position];
}

static void skip_spaces(Parser *parser) {
    while (peek_char(parser) == ' ') {
        parser->position++;
    }
}

static int parse_expression(Parser *parser);

static int parse_number(Parser *parser) {
    skip_spaces(parser);
    int sign = 1;
    if (peek_char(parser) == '-') {
        sign = -1;
        parser->position++;
    }
    int value = 0;
    int digit_count = 0;
    char ch = peek_char(parser);
    while (ch >= '0' && ch <= '9') {
        value = value * 10 + (ch - '0');
        parser->position++;
        digit_count++;
        ch = peek_char(parser);
    }
    if (digit_count == 0) {
        parser->error = 1;
    }
    return sign * value;
}

static int parse_factor(Parser *parser) {
    skip_spaces(parser);
    if (peek_char(parser) == '(') {
        parser->position++;
        int inner = parse_expression(parser);
        skip_spaces(parser);
        if (peek_char(parser) == ')') {
            parser->position++;
        } else {
            parser->error = 1;
        }
        return inner;
    }
    return parse_number(parser);
}

static int parse_term(Parser *parser) {
    int result = parse_factor(parser);
    skip_spaces(parser);
    while (peek_char(parser) == '*') {
        parser->position++;
        int rhs = parse_factor(parser);
        result = result * rhs;
        skip_spaces(parser);
    }
    return result;
}

static int parse_expression(Parser *parser) {
    int result = parse_term(parser);
    skip_spaces(parser);
    char op = peek_char(parser);
    while (op == '+' || op == '-') {
        parser->position++;
        int rhs = parse_term(parser);
        if (op == '+') {
            result = result + rhs;
        } else {
            result = result - rhs;
        }
        skip_spaces(parser);
        op = peek_char(parser);
    }
    return result;
}

int evaluate_expression(const char *text, int *out_error) {
    Parser parser;
    parser.input = text;
    parser.position = 0;
    parser.error = 0;
    int value = parse_expression(&parser);
    *out_error = parser.error;
    return value;
}
