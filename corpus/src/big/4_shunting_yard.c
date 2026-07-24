/*
 * shunting_yard.c — A small integer expression evaluator.
 *
 * Implements Dijkstra's shunting-yard algorithm to convert an infix
 * arithmetic expression (e.g. "3 + 4 * 2 - 1") into a result using two
 * explicit stacks: one for pending operators and one for operand values.
 * Only non-negative integer literals and the binary operators + - * / %
 * with parentheses are supported, which keeps the whole evaluator free of
 * floating point and recursion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Maximum depth of either stack. Expressions deeper than this are rejected. */
#define STACK_CAPACITY 128

/*
 * EvalState bundles the two parallel stacks the algorithm needs plus their
 * current heights. Keeping them in one struct lets every helper take a single
 * pointer instead of juggling four loose variables.
 */
typedef struct EvalState {
    int64_t values[STACK_CAPACITY];   /* operand stack (numbers)            */
    char    operators[STACK_CAPACITY]; /* operator stack (+ - * / % and '(') */
    int     valueTop;                  /* index one past the last value     */
    int     operatorTop;               /* index one past the last operator  */
    int     error;                     /* sticky flag set on any failure    */
} EvalState;

/*
 * precedence — Report the binding strength of a binary operator.
 *
 * Returns 2 for '*', '/', '%'; 1 for '+', '-'; and 0 for anything else
 * (notably '(', which must never pop a real operator). Higher means tighter.
 */
static int precedence(char op) {
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

/*
 * applyTop — Pop one operator and its two operands, then push the result.
 *
 * Consumes the top operator and the top two values from state. On a divide
 * or modulo by zero it sets state->error and pushes 0 so the stacks stay
 * balanced. Does nothing meaningful once error is already set.
 */
static void applyTop(EvalState *state) {
    if (state->operatorTop < 1 || state->valueTop < 2) {
        state->error = 1;  /* malformed expression: not enough to apply */
        return;
    }
    char op = state->operators[--state->operatorTop];
    int64_t rhs = state->values[--state->valueTop];
    int64_t lhs = state->values[--state->valueTop];
    int64_t result = 0;
    switch (op) {
        case '+': result = lhs + rhs; break;
        case '-': result = lhs - rhs; break;
        case '*': result = lhs * rhs; break;
        case '/':
            if (rhs == 0) { state->error = 1; break; }
            result = lhs / rhs;
            break;
        case '%':
            if (rhs == 0) { state->error = 1; break; }
            result = lhs % rhs;
            break;
        default: state->error = 1; break;
    }
    state->values[state->valueTop++] = result;
}

/*
 * evaluateExpression — Parse and evaluate an infix integer expression.
 *
 * Scans text left to right, pushing operands directly and using the operator
 * stack to defer lower- or equal-precedence operators per shunting-yard.
 * Writes the final value to *out and returns 0 on success, or -1 on any
 * syntax error, division by zero, or stack overflow (in which case *out is
 * left unchanged).
 */
int evaluateExpression(const char *text, int64_t *out) {
    EvalState state;
    state.valueTop = 0;
    state.operatorTop = 0;
    state.error = 0;

    const char *cursor = text;
    while (*cursor != '\0') {
        char c = *cursor;
        if (c == ' ' || c == '\t') {
            cursor++;                 /* whitespace is insignificant */
            continue;
        }
        if (c >= '0' && c <= '9') {
            /* Accumulate a multi-digit literal in place. */
            int64_t number = 0;
            while (*cursor >= '0' && *cursor <= '9') {
                number = number * 10 + (*cursor - '0');
                cursor++;
            }
            if (state.valueTop >= STACK_CAPACITY) return -1;
            state.values[state.valueTop++] = number;
            continue;                 /* cursor already advanced */
        }
        if (c == '(') {
            if (state.operatorTop >= STACK_CAPACITY) return -1;
            state.operators[state.operatorTop++] = '(';
        } else if (c == ')') {
            /* Collapse everything back to the matching '('. */
            while (state.operatorTop > 0 &&
                   state.operators[state.operatorTop - 1] != '(') {
                applyTop(&state);
            }
            if (state.operatorTop == 0) return -1;  /* unbalanced ')' */
            state.operatorTop--;       /* discard the '(' itself */
        } else if (precedence(c) > 0) {
            /* Pop operators of greater-or-equal precedence first (left assoc). */
            while (state.operatorTop > 0 &&
                   precedence(state.operators[state.operatorTop - 1]) >= precedence(c)) {
                applyTop(&state);
            }
            if (state.operatorTop >= STACK_CAPACITY) return -1;
            state.operators[state.operatorTop++] = c;
        } else {
            return -1;                 /* unrecognized character */
        }
        if (state.error) return -1;
        cursor++;
    }

    /* Drain any operators still pending after the last token. */
    while (state.operatorTop > 0) {
        applyTop(&state);
        if (state.error) return -1;
    }
    if (state.valueTop != 1) return -1; /* leftover operands => malformed */
    *out = state.values[0];
    return 0;
}
