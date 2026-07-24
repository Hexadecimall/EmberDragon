// Stack-based bytecode VM with 18 opcodes: arithmetic, comparison,
// conditional/unconditional jumps, call/ret with a frame stack, and locals.
// Runs an embedded program that computes factorial(10) via a recursive call.
#include <cstdint>
#include <vector>
#include <array>
#include <cassert>

enum Op : std::uint8_t {
    PUSH, POP, DUP,         // stack ops
    ADD, SUB, MUL, MOD, NEG,// arithmetic
    EQ, LT, GT,             // compare top two -> 0/1
    JMP, JZ, JNZ,           // jumps (JZ/JNZ pop a condition)
    CALL, RET,              // call pushes ret-addr + frame; RET pops it
    LOAD, STORE,            // local[operand] access
    HALT
};

struct Instr { Op op; std::int64_t arg; };

struct Frame {
    std::size_t ret_addr;
    std::array<std::int64_t, 4> locals;
};

class VM {
public:
    explicit VM(std::vector<Instr> code) : code_(std::move(code)), pc_(0) {
        frames_.push_back(Frame{0, {0,0,0,0}});
    }

    std::int64_t run() {
        for (;;) {
            const Instr& in = code_[pc_++];
            switch (in.op) {
                case PUSH: push(in.arg); break;
                case POP:  pop(); break;
                case DUP:  { std::int64_t t = peek(); push(t); } break;
                case ADD:  { auto b = pop(), a = pop(); push(a + b); } break;
                case SUB:  { auto b = pop(), a = pop(); push(a - b); } break;
                case MUL:  { auto b = pop(), a = pop(); push(a * b); } break;
                case MOD:  { auto b = pop(), a = pop(); push(a % b); } break;
                case NEG:  push(-pop()); break;
                case EQ:   { auto b = pop(), a = pop(); push(a == b); } break;
                case LT:   { auto b = pop(), a = pop(); push(a < b); } break;
                case GT:   { auto b = pop(), a = pop(); push(a > b); } break;
                case JMP:  pc_ = static_cast<std::size_t>(in.arg); break;
                case JZ:   if (pop() == 0) pc_ = static_cast<std::size_t>(in.arg); break;
                case JNZ:  if (pop() != 0) pc_ = static_cast<std::size_t>(in.arg); break;
                case CALL: {
                    Frame f{};
                    f.ret_addr = pc_;
                    f.locals[0] = pop();        // pass one argument in local[0]
                    frames_.push_back(f);
                    pc_ = static_cast<std::size_t>(in.arg);
                } break;
                case RET: {
                    std::size_t ret = frames_.back().ret_addr;
                    frames_.pop_back();
                    pc_ = ret;
                } break;
                case LOAD:  push(frames_.back().locals[static_cast<std::size_t>(in.arg)]); break;
                case STORE: frames_.back().locals[static_cast<std::size_t>(in.arg)] = pop(); break;
                case HALT: return stack_.empty() ? 0 : peek();
            }
        }
    }

private:
    void push(std::int64_t v) { stack_.push_back(v); }
    std::int64_t pop() { std::int64_t v = stack_.back(); stack_.pop_back(); return v; }
    std::int64_t peek() const { return stack_.back(); }

    std::vector<Instr> code_;
    std::vector<std::int64_t> stack_;
    std::vector<Frame> frames_;
    std::size_t pc_;
};

int main() {
    // fact(n): if n < 2 return 1 else return n * fact(n-1)
    // Layout: main at 0, function fact starts at index FACT.
    const std::int64_t FACT = 6;
    std::vector<Instr> code = {
        /* 0 */ {PUSH, 10},     // arg
        /* 1 */ {CALL, FACT},   // fact(10)
        /* 2 */ {STORE, 0},     // main local[0] = result
        /* 3 */ {LOAD, 0},
        /* 4 */ {DUP, 0},
        /* 5 */ {HALT, 0},
        // ---- fact: arg in local[0] ----
        /* 6 */ {LOAD, 0},      // n
        /* 7 */ {PUSH, 2},
        /* 8 */ {LT, 0},        // n < 2 ?
        /* 9 */ {JZ, 13},       // if not, go recurse
        /*10 */ {PUSH, 1},      // base case
        /*11 */ {STORE, 1},
        /*12 */ {JMP, 20},
        /*13 */ {LOAD, 0},      // n
        /*14 */ {LOAD, 0},      // n
        /*15 */ {PUSH, 1},
        /*16 */ {SUB, 0},       // n-1
        /*17 */ {CALL, FACT},   // fact(n-1)
        /*18 */ {MUL, 0},       // n * fact(n-1)
        /*19 */ {STORE, 1},
        /*20 */ {LOAD, 1},      // push result
        /*21 */ {RET, 0},
    };

    VM vm(std::move(code));
    std::int64_t result = vm.run();
    // 10! = 3628800
    assert(result == 3628800);
    return result == 3628800 ? 0 : 1;
}
