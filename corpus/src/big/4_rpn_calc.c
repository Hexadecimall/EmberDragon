/*
 * rpn_calc.c — A reverse-Polish-notation integer calculator.
 *
 * Evaluates a postfix expression such as "3 4 + 5 *" by scanning tokens left
 * to right and maintaining a single operand stack: operands are pushed, and
 * each operator pops its arguments and pushes the result. Postfix needs no
 * precedence rules or parentheses, so the evaluator is a single tight loop.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define RPN_STACK_SIZE 64

/* Outcome codes returned by evalRpn. */
typedef enum RpnResult {
    RPN_OK,           /* evaluated to exactly one value          */
    RPN_UNDERFLOW,    /* an operator lacked enough operands       */
    RPN_OVERFLOW,     /* pushed past RPN_STACK_SIZE               */
    RPN_DIV_ZERO,     /* division or modulo by zero               */
    RPN_BAD_TOKEN,    /* token was neither a number nor operator  */
    RPN_LEFTOVER      /* more than one value remained at the end   */
} RpnResult;

/* A bounded operand stack with its current height. */
typedef struct RpnStack {
    int64_t data[RPN_STACK_SIZE];
    int     top;                     /* index one past the last value */
} RpnStack;

/*
 * isOperator — Report whether a one-character token is a supported operator.
 *
 * Returns 1 for + - * / %, 0 otherwise. Used to distinguish operators from
 * operands during the scan.
 */
static int isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

/*
 * parseInteger — Convert a token to a signed 64-bit integer.
 *
 * Accepts an optional leading sign and one or more digits. Returns 1 and
 * writes the value to *out on success, or 0 if the token is not a valid
 * integer (for example, a lone "-" or trailing letters).
 */
static int parseInteger(const char *token, int64_t *out) {
    const char *s = token;
    int64_t sign = 1;
    if (*s == '+' || *s == '-') {
        if (*s == '-') sign = -1;
        s++;
    }
    if (*s == '\0') return 0;         /* a sign with no digits is invalid */

    int64_t value = 0;
    while (*s != '\0') {
        if (*s < '0' || *s > '9') return 0;
        value = value * 10 + (*s - '0');
        s++;
    }
    *out = value * sign;
    return 1;
}

/*
 * applyOperator — Pop two operands, apply op, and push the result.
 *
 * Pops b then a from the stack and pushes the result of (a op b). Returns
 * RPN_UNDERFLOW if fewer than two operands are present, RPN_DIV_ZERO on a
 * zero divisor/modulus, and RPN_OK otherwise (the push cannot overflow because
 * it nets the stack one slot smaller).
 */
static RpnResult applyOperator(RpnStack *stack, char op) {
    if (stack->top < 2) return RPN_UNDERFLOW;
    int64_t b = stack->data[--stack->top];
    int64_t a = stack->data[--stack->top];
    int64_t result;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) return RPN_DIV_ZERO;
            result = a / b;
            break;
        case '%':
            if (b == 0) return RPN_DIV_ZERO;
            result = a % b;
            break;
        default: return RPN_BAD_TOKEN; /* defensive; isOperator gates callers */
    }
    stack->data[stack->top++] = result;
    return RPN_OK;
}

/*
 * evalRpn — Evaluate a whitespace-separated postfix expression.
 *
 * Tokenizes text on spaces and tabs, pushing integers and applying operators
 * as they appear. On success writes the single remaining value to *out and
 * returns RPN_OK. Returns a specific RpnResult error for an empty expression,
 * a bad token, a stack fault, division by zero, or leftover operands. The
 * input string is not modified.
 */
RpnResult evalRpn(const char *text, int64_t *out) {
    RpnStack stack;
    stack.top = 0;

    const char *p = text;
    while (*p != '\0') {
        /* Skip any run of whitespace between tokens. */
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            p++;
            continue;
        }

        /* Copy the next token into a small fixed buffer. */
        char token[32];
        int len = 0;
        while (*p != '\0' && *p != ' ' && *p != '\t' &&
               *p != '\n' && *p != '\r') {
            if (len + 1 >= (int)sizeof(token)) return RPN_BAD_TOKEN;
            token[len++] = *p++;
        }
        token[len] = '\0';

        /* A single-character operator token, but only when it stands alone:
         * "-5" must parse as a number, not the subtraction operator. */
        if (len == 1 && isOperator(token[0])) {
            RpnResult status = applyOperator(&stack, token[0]);
            if (status != RPN_OK) return status;
            continue;
        }

        int64_t value;
        if (!parseInteger(token, &value)) return RPN_BAD_TOKEN;
        if (stack.top >= RPN_STACK_SIZE) return RPN_OVERFLOW;
        stack.data[stack.top++] = value;
    }

    if (stack.top != 1) return RPN_LEFTOVER; /* also catches the empty input */
    *out = stack.data[0];
    return RPN_OK;
}
