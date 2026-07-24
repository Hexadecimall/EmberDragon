/*
 * stackvm.cpp — A tiny stack-based bytecode virtual machine for integers.
 *
 * Executes a flat array of opcodes against an operand stack of 32-bit signed
 * integers. Supports immediate pushes, arithmetic, comparison, and conditional
 * and unconditional jumps, making it Turing-complete enough for small loops.
 */

#include <cstdint>

/* Instruction set. Each opcode is one byte; OP_PUSH consumes the next cell. */
enum OpCode : int32_t {
    OP_PUSH,   /* push the following immediate operand */
    OP_ADD,    /* pop b, pop a, push a + b */
    OP_SUB,    /* pop b, pop a, push a - b */
    OP_MUL,    /* pop b, pop a, push a * b */
    OP_DIV,    /* pop b, pop a, push a / b (halts on b == 0) */
    OP_DUP,    /* duplicate the top of stack */
    OP_CMPLT,  /* pop b, pop a, push 1 if a < b else 0 */
    OP_JMP,    /* unconditional jump to the following immediate address */
    OP_JZ,     /* pop c; if c == 0 jump to the following immediate address */
    OP_HALT    /* stop execution and return the top of stack */
};

#define STACK_MAX 256   /* operand stack depth */

/*
 * Execution state: the operand stack and its top pointer. The program counter
 * is local to run(); only the data stack needs to persist as a structure.
 */
struct VM {
    int32_t stack[STACK_MAX];
    int32_t sp;   /* number of values on the stack; stack[sp-1] is the top */
};

/*
 * Initialise a VM to an empty stack.
 * Parameters: vm — the machine to reset. Returns nothing. O(1).
 */
void vmInit(VM *vm) {
    vm->sp = 0;
}

/*
 * Push a value onto the operand stack.
 * Parameters: vm — the machine; value — the integer to push. Returns 1 on
 * success, 0 if the stack is full (overflow is refused, not wrapped). O(1).
 */
static int vmPush(VM *vm, int32_t value) {
    if (vm->sp >= STACK_MAX)
        return 0;
    vm->stack[vm->sp++] = value;
    return 1;
}

/*
 * Pop a value from the operand stack.
 * Parameters: vm — the machine; out — receives the popped value. Returns 1 on
 * success, 0 on underflow (out is left untouched). O(1).
 */
static int vmPop(VM *vm, int32_t *out) {
    if (vm->sp <= 0)
        return 0;
    *out = vm->stack[--vm->sp];
    return 1;
}

/*
 * Run a bytecode program to completion.
 * Parameters: vm — a fresh machine; code — the instruction/operand array;
 * length — number of cells in code; outResult — receives the top of stack at
 * HALT. Returns 1 on clean halt, 0 on any fault (underflow, overflow, div by
 * zero, bad jump, or running off the end). O(steps executed), which may exceed
 * length because of backward jumps in loops.
 */
int vmRun(VM *vm, const int32_t *code, int32_t length, int32_t *outResult) {
    int32_t pc = 0;   /* program counter: index of the next opcode */

    while (pc < length) {
        int32_t op = code[pc++];
        int32_t a, b;

        switch (op) {
        case OP_PUSH:
            if (pc >= length)              /* immediate must follow the opcode */
                return 0;
            if (!vmPush(vm, code[pc++]))
                return 0;
            break;

        case OP_ADD:
            if (!vmPop(vm, &b) || !vmPop(vm, &a)) return 0;
            if (!vmPush(vm, a + b)) return 0;
            break;

        case OP_SUB:
            if (!vmPop(vm, &b) || !vmPop(vm, &a)) return 0;
            if (!vmPush(vm, a - b)) return 0;
            break;

        case OP_MUL:
            if (!vmPop(vm, &b) || !vmPop(vm, &a)) return 0;
            if (!vmPush(vm, a * b)) return 0;
            break;

        case OP_DIV:
            if (!vmPop(vm, &b) || !vmPop(vm, &a)) return 0;
            if (b == 0) return 0;          /* division by zero is a hard fault */
            if (!vmPush(vm, a / b)) return 0;
            break;

        case OP_DUP:
            if (!vmPop(vm, &a)) return 0;
            /* Push twice to leave two copies; both pushes must fit. */
            if (!vmPush(vm, a) || !vmPush(vm, a)) return 0;
            break;

        case OP_CMPLT:
            if (!vmPop(vm, &b) || !vmPop(vm, &a)) return 0;
            if (!vmPush(vm, a < b ? 1 : 0)) return 0;
            break;

        case OP_JMP:
            if (pc >= length) return 0;
            pc = code[pc];                 /* target replaces the PC outright */
            if (pc < 0 || pc >= length) return 0;
            break;

        case OP_JZ:
            if (pc >= length) return 0;
            if (!vmPop(vm, &a)) return 0;
            if (a == 0) {
                pc = code[pc];             /* branch taken */
                if (pc < 0 || pc >= length) return 0;
            } else {
                pc++;                      /* skip the unused jump target */
            }
            break;

        case OP_HALT:
            /* Report the current top of stack, or 0 if the stack is empty. */
            *outResult = (vm->sp > 0) ? vm->stack[vm->sp - 1] : 0;
            return 1;

        default:
            return 0;                      /* unknown opcode: fault */
        }
    }
    return 0;   /* fell off the end without an explicit HALT */
}
