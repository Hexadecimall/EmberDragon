// Function-pointer dispatch table: opcodes select an int->int->int operation.
#include <iostream>
#include <cstdint>

static int op_add(int a, int b) { return a + b; }
static int op_sub(int a, int b) { return a - b; }
static int op_mul(int a, int b) { return a * b; }
static int op_max(int a, int b) { return a > b ? a : b; }

typedef int (*BinOp)(int, int);

struct Entry {
    const char *name;
    BinOp fn;
};

int main() {
    Entry table[] = {
        {"add", op_add},
        {"sub", op_sub},
        {"mul", op_mul},
        {"max", op_max},
    };
    int n = (int)(sizeof(table) / sizeof(table[0]));

    // a tiny program: opcode, lhs, rhs triples
    uint8_t prog[] = {0, 7, 3, 1, 7, 3, 2, 7, 3, 3, 7, 3};
    int steps = (int)(sizeof(prog) / sizeof(prog[0])) / 3;
    long acc = 0;
    for (int i = 0; i < steps; i++) {
        int op = prog[i * 3 + 0] % n;
        int a = prog[i * 3 + 1];
        int b = prog[i * 3 + 2];
        int r = table[op].fn(a, b);
        std::cout << table[op].name << "(" << a << "," << b << ")=" << r << "\n";
        acc += r;
    }
    std::cout << "acc=" << acc << "\n";
    return 0;
}
