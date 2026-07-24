#include <stdint.h>
#include <string.h>

#define MAX_DIGITS 64

class BigInt {
public:
    uint8_t digits[MAX_DIGITS];
    int32_t length;

    BigInt() {
        memset(digits, 0, sizeof(digits));
        length = 1;
    }

    void setFromInt(uint64_t value) {
        memset(digits, 0, sizeof(digits));
        length = 0;
        if (value == 0) {
            digits[0] = 0;
            length = 1;
            return;
        }
        while (value > 0 && length < MAX_DIGITS) {
            digits[length] = (uint8_t)(value % 10);
            value /= 10;
            length++;
        }
    }

    void normalize() {
        while (length > 1 && digits[length - 1] == 0) {
            length--;
        }
    }

    bool isZero() const {
        return length == 1 && digits[0] == 0;
    }
};

void bigint_add(const BigInt *a, const BigInt *b, BigInt *result) {
    memset(result->digits, 0, sizeof(result->digits));
    int32_t maxLength = a->length > b->length ? a->length : b->length;
    int32_t carry = 0;
    int32_t pos = 0;
    for (pos = 0; pos < maxLength; pos++) {
        int32_t left = pos < a->length ? a->digits[pos] : 0;
        int32_t right = pos < b->length ? b->digits[pos] : 0;
        int32_t sum = left + right + carry;
        result->digits[pos] = (uint8_t)(sum % 10);
        carry = sum / 10;
    }
    if (carry > 0 && pos < MAX_DIGITS) {
        result->digits[pos] = (uint8_t)carry;
        pos++;
    }
    result->length = pos;
    result->normalize();
}

void bigint_multiply_small(const BigInt *a, int32_t factor, BigInt *result) {
    memset(result->digits, 0, sizeof(result->digits));
    int32_t carry = 0;
    int32_t pos = 0;
    for (pos = 0; pos < a->length; pos++) {
        int32_t product = a->digits[pos] * factor + carry;
        result->digits[pos] = (uint8_t)(product % 10);
        carry = product / 10;
    }
    while (carry > 0 && pos < MAX_DIGITS) {
        result->digits[pos] = (uint8_t)(carry % 10);
        carry /= 10;
        pos++;
    }
    result->length = pos > 0 ? pos : 1;
    result->normalize();
}

int bigint_compare(const BigInt *a, const BigInt *b) {
    if (a->length != b->length) {
        return a->length > b->length ? 1 : -1;
    }
    for (int32_t i = a->length - 1; i >= 0; i--) {
        if (a->digits[i] != b->digits[i]) {
            return a->digits[i] > b->digits[i] ? 1 : -1;
        }
    }
    return 0;
}

void bigint_factorial(int32_t n, BigInt *result) {
    result->setFromInt(1);
    for (int32_t multiplier = 2; multiplier <= n; multiplier++) {
        BigInt scratch;
        bigint_multiply_small(result, multiplier, &scratch);
        memcpy(result->digits, scratch.digits, sizeof(result->digits));
        result->length = scratch.length;
    }
}
