#include <iostream>

using namespace std;

void Counter(long a, int b);
long live();

static long g_100008000[8192];   // unresolved data global — real bytes pending (lifter)


static const char str[] = "\n";

class Counter {
    int total;
    int value;
    void tick() {
        this->total += this->value;
        return;
    }
    void untick() {
        this->total -= this->value;
        return;
    }
    void reset() {
        this->total = 0;
        return;
    }
    long get() const {
        return this->total;
    }
    Counter(int a) {
        this->total = 0;
        this->value = a;
        *g_100008000 = *g_100008000 + 1;
        return;
    }
};

int main(int argc, char** argv) {
    long v68;
    long v60;
    int i;
    long t8;
    long v8;
    long t12;
    long v24;
    long v40;
    Counter(&v68, 1);
    Counter(&v60, 5);
    i = 0;
    while (i < 6) {
        v68.tick();
        i++;
    }
    v68.untick();
    v60.tick();
    v60.tick();
    v60.reset();
    v60.tick();
    t8 = cout << "a = ";
    v8 = t8;
    v8 << v68.get(t8);
    operator_lshstr;
    t12 = cout << "b = ";
    v24 = t12;
    v24 << v60.get(t12);
    operator_lshstr;
    v40 = cout << "counters alive = ";
    v40 << live();
    operator_lshstr;
    return 0;
}

Counter::Counter(int b) {
    long result;
    result = a;
    Counter(a, b);
    return;
}

long Counter::live() {
    return *g_100008000;
}

