/* Mini stack-machine interpreter running a fixed bytecode program.
   Computes (2 + 3) * 4 - 1, prints intermediate ops, returns top of stack. */
#include <stdio.h>
#include <stdint.h>

enum { OP_PUSH, OP_ADD, OP_SUB, OP_MUL, OP_PRINT, OP_HALT };

int main(void) {
    /* program: PUSH 2, PUSH 3, ADD, PUSH 4, MUL, PUSH 1, SUB, PRINT, HALT */
    int32_t code[] = {
        OP_PUSH, 2,
        OP_PUSH, 3,
        OP_ADD,
        OP_PUSH, 4,
        OP_MUL,
        OP_PUSH, 1,
        OP_SUB,
        OP_PRINT,
        OP_HALT
    };

    int32_t stack[32];
    int sp = 0;
    int ip = 0;
    int running = 1;

    while (running) {
        int32_t op = code[ip++];
        switch (op) {
            case OP_PUSH:
                stack[sp++] = code[ip++];
                break;
            case OP_ADD: {
                int32_t b = stack[--sp], a = stack[--sp];
                stack[sp++] = a + b;
                printf("ADD -> %d\n", stack[sp - 1]);
                break;
            }
            case OP_SUB: {
                int32_t b = stack[--sp], a = stack[--sp];
                stack[sp++] = a - b;
                printf("SUB -> %d\n", stack[sp - 1]);
                break;
            }
            case OP_MUL: {
                int32_t b = stack[--sp], a = stack[--sp];
                stack[sp++] = a * b;
                printf("MUL -> %d\n", stack[sp - 1]);
                break;
            }
            case OP_PRINT:
                printf("TOP = %d\n", stack[sp - 1]);
                break;
            case OP_HALT:
                running = 0;
                break;
            default:
                fprintf(stderr, "bad opcode %d\n", op);
                return 1;
        }
    }
    return stack[sp - 1] & 0xFF;
}
