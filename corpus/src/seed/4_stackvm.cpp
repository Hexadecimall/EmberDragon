#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// A tiny stack-based virtual machine executing a fixed bytecode program.
// Opcodes manipulate a value stack of 32-bit integers.

enum OpCode {
    OP_PUSH = 1,
    OP_ADD  = 2,
    OP_SUB  = 3,
    OP_MUL  = 4,
    OP_DUP  = 5,
    OP_SWAP = 6,
    OP_JMPZ = 7,
    OP_JMP  = 8,
    OP_HALT = 9
};

struct Instruction {
    int32_t opcode;
    int32_t operand;
};

class StackMachine {
public:
    StackMachine() : stackTop(0), programCounter(0), faulted(0) {}

    void push(int32_t value) {
        if (stackTop < kStackSize)
            stack[stackTop++] = value;
        else
            faulted = 1;
    }

    int32_t pop() {
        if (stackTop > 0)
            return stack[--stackTop];
        faulted = 1;
        return 0;
    }

    int32_t run(const Instruction *program, int32_t length) {
        programCounter = 0;
        while (programCounter < length && !faulted) {
            const Instruction *current = &program[programCounter];
            switch (current->opcode) {
                case OP_PUSH:
                    push(current->operand);
                    programCounter++;
                    break;
                case OP_ADD: {
                    int32_t b = pop();
                    int32_t a = pop();
                    push(a + b);
                    programCounter++;
                    break;
                }
                case OP_SUB: {
                    int32_t b = pop();
                    int32_t a = pop();
                    push(a - b);
                    programCounter++;
                    break;
                }
                case OP_MUL: {
                    int32_t b = pop();
                    int32_t a = pop();
                    push(a * b);
                    programCounter++;
                    break;
                }
                case OP_DUP: {
                    int32_t top = pop();
                    push(top);
                    push(top);
                    programCounter++;
                    break;
                }
                case OP_SWAP: {
                    int32_t b = pop();
                    int32_t a = pop();
                    push(b);
                    push(a);
                    programCounter++;
                    break;
                }
                case OP_JMPZ: {
                    int32_t condition = pop();
                    if (condition == 0)
                        programCounter = current->operand;
                    else
                        programCounter++;
                    break;
                }
                case OP_JMP:
                    programCounter = current->operand;
                    break;
                case OP_HALT:
                    return stackTop > 0 ? stack[stackTop - 1] : 0;
                default:
                    faulted = 1;
                    break;
            }
        }
        if (faulted)
            return -1;
        return stackTop > 0 ? stack[stackTop - 1] : 0;
    }

    int hasFaulted() const { return faulted; }

private:
    static const int kStackSize = 128;
    int32_t stack[kStackSize];
    int32_t stackTop;
    int32_t programCounter;
    int faulted;
};

int32_t executeProgram(const Instruction *program, int32_t length) {
    StackMachine machine;
    return machine.run(program, length);
}
