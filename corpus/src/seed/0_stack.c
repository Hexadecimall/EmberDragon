#include <stdio.h>
#include <stdlib.h>

/* Fixed-capacity array stack used as an RPN calculator engine. */
#define STACK_CAPACITY 64

struct Stack {
    int items[STACK_CAPACITY];
    int top;
};

void stack_init(struct Stack *stack) {
    stack->top = 0;
}

int stack_is_empty(struct Stack *stack) {
    return stack->top == 0;
}

int stack_is_full(struct Stack *stack) {
    return stack->top >= STACK_CAPACITY;
}

int stack_push(struct Stack *stack, int value) {
    if (stack_is_full(stack)) {
        return 0;
    }
    stack->items[stack->top] = value;
    stack->top = stack->top + 1;
    return 1;
}

int stack_pop(struct Stack *stack, int *out) {
    if (stack_is_empty(stack)) {
        return 0;
    }
    stack->top = stack->top - 1;
    *out = stack->items[stack->top];
    return 1;
}

int stack_peek(struct Stack *stack) {
    if (stack_is_empty(stack)) {
        return 0;
    }
    return stack->items[stack->top - 1];
}

int evaluate_rpn(int *tokens, int length) {
    struct Stack stack;
    int index;
    stack_init(&stack);
    for (index = 0; index < length; index = index + 1) {
        int token = tokens[index];
        if (token >= 0) {
            stack_push(&stack, token);
        } else {
            int right;
            int left;
            int result;
            stack_pop(&stack, &right);
            stack_pop(&stack, &left);
            if (token == -1) {
                result = left + right;
            } else if (token == -2) {
                result = left - right;
            } else if (token == -3) {
                result = left * right;
            } else {
                if (right == 0) {
                    result = 0;
                } else {
                    result = left / right;
                }
            }
            stack_push(&stack, result);
        }
    }
    return stack_peek(&stack);
}

int is_balanced(int *brackets, int length) {
    struct Stack stack;
    int index;
    stack_init(&stack);
    for (index = 0; index < length; index = index + 1) {
        int symbol = brackets[index];
        if (symbol == 1) {
            stack_push(&stack, symbol);
        } else if (symbol == 2) {
            int popped;
            if (!stack_pop(&stack, &popped)) {
                return 0;
            }
        }
    }
    return stack_is_empty(&stack);
}
