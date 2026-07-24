// Fixed-capacity array stack (no heap). push/pop/peek with a top index.
#include <iostream>
#include <cstdint>

struct Stack {
    static const int CAP = 8;
    int data[CAP];
    int top; // index of next free slot
};

static void init(Stack &s) { s.top = 0; }

static bool push(Stack &s, int v) {
    if (s.top >= Stack::CAP) return false;
    s.data[s.top++] = v;
    return true;
}

static bool pop(Stack &s, int &out) {
    if (s.top == 0) return false;
    out = s.data[--s.top];
    return true;
}

int main() {
    Stack s;
    init(s);
    for (int i = 0; i < 12; i++) {
        if (!push(s, i * i))
            std::cout << "full at " << i << "\n";
    }
    long acc = 0;
    int v;
    while (pop(s, v))
        acc += v;
    std::cout << "drained sum=" << acc << " top=" << s.top << "\n";
    return 0;
}
