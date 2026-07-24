// Small C++ class: a Counter with increment/decrement/reset, static instance tally.
#include <iostream>

class Counter {
private:
    int value_;
    int step_;
    static int instances_;

public:
    explicit Counter(int step) : value_(0), step_(step) {
        ++instances_;
    }

    void tick()  { value_ += step_; }
    void untick() { value_ -= step_; }
    void reset() { value_ = 0; }
    int get() const { return value_; }

    static int live() { return instances_; }
};

int Counter::instances_ = 0;

int main() {
    Counter a(1);
    Counter b(5);

    for (int i = 0; i < 6; ++i)
        a.tick();
    a.untick();

    b.tick();
    b.tick();
    b.reset();
    b.tick();

    std::cout << "a = " << a.get() << "\n";
    std::cout << "b = " << b.get() << "\n";
    std::cout << "counters alive = " << Counter::live() << "\n";
    return 0;
}
