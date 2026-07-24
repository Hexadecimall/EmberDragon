#include <iostream>

using namespace std;

long collatz_steps(long a);
long reverse_digits(long a);

const char* s_str = ") = ";

int main(int argc, char** argv) {
    long* obj;
    char v64;
    long v32;
    long v24;
    long v8;
    long v0;
    // neon:  ldr    value0, [v105, #0x0]  // = 0x06000000000000000700000000000000
    // neon:  ldr    value0, [v105, #0x10]  // = 0x1b000000000000006100000000000000
    obj = &v64;
    v32 = &v64 + 32;
    while (obj != v32) {
        v24 = *obj;
        cout << "collatz(";
        operator_lshv24;
        v8 = operator_lsh&s_str;
        v8 << collatz_steps(v24);
        operator_lsh" steps\n";
        obj += 8;
    }
    cout << "reverse(";
    operator_lsh0x12d644;
    v0 = operator_lsh&s_str;
    v0 << reverse_digits(0x12d644);
    operator_lsh10;
    return 0;
}

long collatz_steps(unsigned long long a) {
    long ptr;
    int result;
    ptr = a;
    result = 0;
    while (ptr) {
        ptr = 3 * ptr + 1;
        result++;
        if (ptr != 1) continue;
    }
    return result;
}

long reverse_digits(unsigned long long a) {
    long v8;
    long result;
    v8 = a;
    result = 0;
    do {
        result = result * 10 + (v8 - v8 / 10 * 10);
        v8 /= 10;
    } while (v8 != 0);
    return result;
}

