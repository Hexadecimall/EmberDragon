#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// A Reverse Polish Notation calculator over whitespace-separated tokens.
// Numbers are pushed; operators consume the top two operands.

class OperandStack {
public:
    OperandStack() : size(0), overflowed(false), underflowed(false) {}

    void push(long value) {
        if (size < kCapacity)
            data[size++] = value;
        else
            overflowed = true;
    }

    long pop() {
        if (size > 0)
            return data[--size];
        underflowed = true;
        return 0;
    }

    int depth() const { return size; }
    bool isValid() const { return !overflowed && !underflowed; }

private:
    static const int kCapacity = 64;
    long data[kCapacity];
    int size;
    bool overflowed;
    bool underflowed;
};

static bool parseInteger(const char *token, long *outValue) {
    int index = 0;
    int sign = 1;
    if (token[0] == '-' && token[1] != '\0') {
        sign = -1;
        index = 1;
    }
    long accumulator = 0;
    bool sawDigit = false;
    while (token[index] != '\0') {
        char c = token[index];
        if (c < '0' || c > '9')
            return false;
        accumulator = accumulator * 10 + (c - '0');
        sawDigit = true;
        index++;
    }
    if (!sawDigit)
        return false;
    *outValue = accumulator * sign;
    return true;
}

static void applyOperator(OperandStack &stack, char op, int *errorFlag) {
    long right = stack.pop();
    long left = stack.pop();
    switch (op) {
        case '+': stack.push(left + right); break;
        case '-': stack.push(left - right); break;
        case '*': stack.push(left * right); break;
        case '/':
            if (right == 0)
                *errorFlag = 1;
            else
                stack.push(left / right);
            break;
        default:
            *errorFlag = 1;
            break;
    }
}

long evaluateRpn(const char *expression, int *errorFlag) {
    OperandStack stack;
    *errorFlag = 0;
    char buffer[32];
    const char *cursor = expression;

    while (*cursor != '\0') {
        while (*cursor == ' ' || *cursor == '\t')
            cursor++;
        if (*cursor == '\0')
            break;

        int length = 0;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && length < 31) {
            buffer[length++] = *cursor++;
        }
        buffer[length] = '\0';

        long number;
        if (parseInteger(buffer, &number)) {
            stack.push(number);
        } else if (length == 1) {
            applyOperator(stack, buffer[0], errorFlag);
        } else {
            *errorFlag = 1;
        }
    }

    if (!stack.isValid() || stack.depth() != 1)
        *errorFlag = 1;
    return stack.pop();
}
