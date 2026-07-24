/* Switch-based opcode dispatcher: a tiny stack VM driven by a byte program. */
#include <stdio.h>

enum { OP_PUSH, OP_ADD, OP_SUB, OP_MUL, OP_NEG, OP_DUP, OP_PRINT, OP_HALT };

int main(void) {
    /* program: push 6, push 7, mul, dup, push 2, sub, print, halt */
    int prog[] = { OP_PUSH, 6, OP_PUSH, 7, OP_MUL, OP_DUP,
                   OP_PUSH, 2, OP_SUB, OP_PRINT, OP_HALT };
    int stack[64];
    int sp = 0, pc = 0, running = 1;

    while (running) {
        int op = prog[pc++];
        switch (op) {
            case OP_PUSH: stack[sp++] = prog[pc++]; break;
            case OP_ADD:  stack[sp - 2] += stack[sp - 1]; sp--; break;
            case OP_SUB:  stack[sp - 2] -= stack[sp - 1]; sp--; break;
            case OP_MUL:  stack[sp - 2] *= stack[sp - 1]; sp--; break;
            case OP_NEG:  stack[sp - 1] = -stack[sp - 1]; break;
            case OP_DUP:  stack[sp] = stack[sp - 1]; sp++; break;
            case OP_PRINT: printf("top = %d\n", stack[sp - 1]); break;
            case OP_HALT: running = 0; break;
            default: fprintf(stderr, "bad op %d\n", op); running = 0; break;
        }
    }
    printf("final sp = %d\n", sp);
    return 0;
}
