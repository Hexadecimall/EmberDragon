#include <stdint.h>

typedef struct Fraction {
    int64_t numerator;
    int64_t denominator;
} Fraction;

int64_t gcd(int64_t a, int64_t b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int64_t remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int64_t lcm(int64_t a, int64_t b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    int64_t divisor = gcd(a, b);
    int64_t product = (a / divisor) * b;
    if (product < 0) {
        product = -product;
    }
    return product;
}

int64_t gcd_extended(int64_t a, int64_t b, int64_t *coeffX, int64_t *coeffY) {
    if (b == 0) {
        *coeffX = 1;
        *coeffY = 0;
        return a;
    }
    int64_t innerX;
    int64_t innerY;
    int64_t result = gcd_extended(b, a % b, &innerX, &innerY);
    *coeffX = innerY;
    *coeffY = innerX - (a / b) * innerY;
    return result;
}

void fraction_reduce(Fraction *fraction) {
    int64_t divisor = gcd(fraction->numerator, fraction->denominator);
    if (divisor == 0) {
        return;
    }
    fraction->numerator /= divisor;
    fraction->denominator /= divisor;
    if (fraction->denominator < 0) {
        fraction->denominator = -fraction->denominator;
        fraction->numerator = -fraction->numerator;
    }
}

Fraction fraction_add(Fraction left, Fraction right) {
    Fraction sum;
    sum.numerator = left.numerator * right.denominator + right.numerator * left.denominator;
    sum.denominator = left.denominator * right.denominator;
    fraction_reduce(&sum);
    return sum;
}

int are_coprime(int64_t a, int64_t b) {
    return gcd(a, b) == 1;
}

int64_t gcd_of_array(const int64_t *values, int32_t length) {
    if (length == 0) {
        return 0;
    }
    int64_t accumulator = values[0];
    for (int32_t i = 1; i < length; i++) {
        accumulator = gcd(accumulator, values[i]);
        if (accumulator == 1) {
            break;
        }
    }
    return accumulator;
}
