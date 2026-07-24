// Enum-driven switch returning string literals (jump-table lowering),
// plus a fallthrough group. Decompiler-favorite shape.
#include <cstdio>

enum Op { ADD, SUB, MUL, DIV, MOD, NEG, NOP };

static const char* name(Op o) {
    switch (o) {
        case ADD: return "add";
        case SUB: return "sub";
        case MUL: return "mul";
        case DIV:
        case MOD: return "divlike";   // intentional fallthrough group
        case NEG: return "neg";
        default:  return "nop";
    }
}

static int apply(Op o, int a, int b) {
    switch (o) {
        case ADD: return a + b;
        case SUB: return a - b;
        case MUL: return a * b;
        case DIV: return b ? a / b : 0;
        case MOD: return b ? a % b : 0;
        case NEG: return -a;
        default:  return a;
    }
}

int main() {
    Op ops[] = { ADD, SUB, MUL, DIV, MOD, NEG, NOP };
    for (Op o : ops)
        std::printf("%-7s(12,5) = %d\n", name(o), apply(o, 12, 5));
    return 0;
}
