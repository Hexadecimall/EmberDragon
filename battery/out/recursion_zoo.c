#include <cstdio>

long fact(long a);
long gcd(long a, long b);
long fib(long a);
long ackermann(long a, long b);
long hanoi_moves(long a);


int main(int argc, char** argv) {
    int result;
    result = 0;
    printf("fact(10)      = %llu\n", fact(10));
    printf("gcd(1071,462) = %llu\n", gcd(1071, 462));
    printf("fib(20)       = %llu\n", fib(20));
    printf("ack(3,4)      = %llu\n", ackermann(3, 4));
    printf("hanoi(16)     = %llu\n", hanoi_moves(16));
    return result;
}

long fact(long a) {
    long result;
    if (a < 2) {
        return 1;
    } else {
        return (a * fact(a - 1));
    }
    return result;
}

long gcd(long a, long b) {
    long result;
    if (b == 0) {
        return a;
    } else {
        return (gcd(b, a - a / b * b));
    }
    return result;
}

long fib(long a) {
    long result;
    long v8;
    if (a < 2) {
        return a;
    } else {
        v8 = fib(a - 1);
        return (v8 + fib(a - 2));
    }
    return result;
}

long ackermann(long a, long b) {
    long result;
    long v0;
    if (a == 0) {
        return (b + 1);
    } else {
        if (b == 0) {
            return (ackermann(a - 1, 1));
        } else {
            v0 = a - 1;
            return (ackermann(v0, ackermann(a, b - 1)));
        }
    }
    return result;
}

long hanoi_moves(long a) {
    long result;
    if (a == 0) {
        return 0;
    } else {
        return (2 * hanoi_moves(a - 1) + 1);
    }
    return result;
}

