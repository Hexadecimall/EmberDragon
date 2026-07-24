/*
 * stack_vm.c — A tiny stack-based bytecode virtual machine.
 *
 * Executes a flat array of integer instructions against an operand stack,
 * supporting push/pop, integer arithmetic, comparisons, conditional and
 * unconditional jumps, and a halt that yields the top-of-stack result. The VM
 * is deliberately register-free: every operation reads and writes the operand
 * stack, which makes the interpreter loop short and easy to reason about.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define VM_STACK_SIZE 256

/* The opcode set. Each instruction is an OpCode plus an optional operand. */
typedef enum OpCode {
    OP_PUSH,   /* push the operand onto the stack                         */
    OP_POP,    /* discard the top of stack                                */
    OP_ADD,    /* pop b, pop a, push (a + b)                               */
    OP_SUB,    /* pop b, pop a, push (a - b)                               */
    OP_MUL,    /* pop b, pop a, push (a * b)                               */
    OP_DIV,    /* pop b, pop a, push (a / b); traps on b == 0             */
    OP_CMPLT,  /* pop b, pop a, push 1 if a < b else 0                    */
    OP_JMP,    /* set the program counter to operand                      */
    OP_JZ,     /* pop a; if a == 0 jump to operand                        */
    OP_DUP,    /* duplicate the top of stack                              */
    OP_HALT    /* stop execution; result is the current top of stack      */
} OpCode;

/* A single decoded instruction: an opcode and its immediate operand. */
typedef struct Instruction {
    OpCode  op;
    int64_t operand;                 /* literal value or jump target */
} Instruction;

/* Distinct termination outcomes the interpreter can report. */
typedef enum VmStatus {
    VM_OK,             /* halted normally with a result            */
    VM_STACK_OVERFLOW, /* push would exceed VM_STACK_SIZE          */
    VM_STACK_UNDERFLOW,/* an op needed more operands than present  */
    VM_DIV_ZERO,       /* division by zero                         */
    VM_BAD_JUMP,       /* jump target outside the program          */
    VM_BAD_OPCODE      /* unrecognized opcode                      */
} VmStatus;

/*
 * Vm holds the entire execution context: the operand stack and its height plus
 * the program counter. Bundling them lets the helpers stay tiny.
 */
typedef struct Vm {
    int64_t stack[VM_STACK_SIZE];
    int     sp;                      /* stack pointer: index of next free slot */
    int     pc;                      /* program counter: next instruction      */
} Vm;

/*
 * vmPush — Push a value, guarding against overflow.
 *
 * Returns VM_OK on success or VM_STACK_OVERFLOW if the stack is full, leaving
 * the stack unchanged on failure.
 */
static VmStatus vmPush(Vm *vm, int64_t value) {
    if (vm->sp >= VM_STACK_SIZE) return VM_STACK_OVERFLOW;
    vm->stack[vm->sp++] = value;
    return VM_OK;
}

/*
 * vmPop — Pop a value into *out, guarding against underflow.
 *
 * Returns VM_OK and writes the popped value to *out, or VM_STACK_UNDERFLOW if
 * the stack is empty (in which case *out is left unchanged).
 */
static VmStatus vmPop(Vm *vm, int64_t *out) {
    if (vm->sp <= 0) return VM_STACK_UNDERFLOW;
    *out = vm->stack[--vm->sp];
    return VM_OK;
}

/*
 * vmRunBinary — Apply a binary arithmetic/comparison op to the top two values.
 *
 * Pops b then a, computes the result for the given opcode, and pushes it back.
 * Returns VM_DIV_ZERO for a divide by zero and propagates any underflow or
 * overflow from the stack operations. Assumes op is one of the binary opcodes.
 */
static VmStatus vmRunBinary(Vm *vm, OpCode op) {
    int64_t b, a;
    VmStatus status = vmPop(vm, &b);
    if (status != VM_OK) return status;
    status = vmPop(vm, &a);
    if (status != VM_OK) return status;

    int64_t result = 0;
    switch (op) {
        case OP_ADD:   result = a + b; break;
        case OP_SUB:   result = a - b; break;
        case OP_MUL:   result = a * b; break;
        case OP_DIV:
            if (b == 0) return VM_DIV_ZERO;
            result = a / b;
            break;
        case OP_CMPLT: result = (a < b) ? 1 : 0; break;
        default:       return VM_BAD_OPCODE; /* not reached for binary ops */
    }
    return vmPush(vm, result);
}

/*
 * vmExecute — Run a bytecode program to completion.
 *
 * Interprets up to instructionCount instructions starting at pc 0, dispatching
 * each opcode through the helpers above. On OP_HALT it writes the top-of-stack
 * (or 0 if the stack is empty) to *result and returns VM_OK. Any fault aborts
 * immediately with the matching VmStatus and leaves *result untouched.
 * Running off the end of the program without a HALT also returns VM_OK with
 * the current top of stack.
 */
VmStatus vmExecute(const Instruction *program, int instructionCount, int64_t *result) {
    Vm vm;
    vm.sp = 0;
    vm.pc = 0;

    while (vm.pc < instructionCount) {
        const Instruction *instr = &program[vm.pc];
        VmStatus status = VM_OK;

        switch (instr->op) {
            case OP_PUSH:
                status = vmPush(&vm, instr->operand);
                vm.pc++;
                break;
            case OP_POP: {
                int64_t discarded;
                status = vmPop(&vm, &discarded);
                vm.pc++;
                break;
            }
            case OP_ADD: case OP_SUB: case OP_MUL:
            case OP_DIV: case OP_CMPLT:
                status = vmRunBinary(&vm, instr->op);
                vm.pc++;
                break;
            case OP_DUP: {
                if (vm.sp <= 0) { status = VM_STACK_UNDERFLOW; break; }
                status = vmPush(&vm, vm.stack[vm.sp - 1]);
                vm.pc++;
                break;
            }
            case OP_JMP:
                /* Validate the target before transferring control. */
                if (instr->operand < 0 || instr->operand >= instructionCount)
                    return VM_BAD_JUMP;
                vm.pc = (int)instr->operand;
                break;
            case OP_JZ: {
                int64_t condition;
                status = vmPop(&vm, &condition);
                if (status != VM_OK) break;
                if (condition == 0) {
                    if (instr->operand < 0 || instr->operand >= instructionCount)
                        return VM_BAD_JUMP;
                    vm.pc = (int)instr->operand;
                } else {
                    vm.pc++;           /* fall through when nonzero */
                }
                break;
            }
            case OP_HALT:
                /* Yield the result; an empty stack halts with 0 by convention. */
                *result = (vm.sp > 0) ? vm.stack[vm.sp - 1] : 0;
                return VM_OK;
            default:
                return VM_BAD_OPCODE;
        }
        if (status != VM_OK) return status;
    }

    /* Fell off the end without an explicit HALT. */
    *result = (vm.sp > 0) ? vm.stack[vm.sp - 1] : 0;
    return VM_OK;
}
