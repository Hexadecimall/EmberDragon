/*
 * digit_sequences.c — Digit-driven number sequences.
 *
 * Routines built around the decimal digits of a number: digital root, the
 * look-and-say sequence step, the digit-factorial chain leading to its
 * cycle (the "factorion" world of 145 and 169), and a happy-number test.
 * Pure integer and array manipulation, no floating point.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Sum the decimal digits of n.
 *
 * n:      input value.
 * return: the sum of n's base-10 digits. O(number of digits).
 */
uint32_t digit_sum(uint64_t n) {
    uint32_t sum = 0;
    while (n > 0) {
        sum += (uint32_t)(n % 10); /* peel off the least significant digit */
        n /= 10;
    }
    return sum;
}

/*
 * Compute the digital root: repeatedly sum digits until one remains.
 *
 * Equivalent to 1 + (n-1) % 9 for n > 0, but computed iteratively here to
 * mirror the definition. The digital root of 0 is 0.
 *
 * n:      input value.
 * return: the single-digit digital root (0..9). O(log n) total work.
 */
uint32_t digital_root(uint64_t n) {
    while (n >= 10) {
        n = digit_sum(n); /* collapse multi-digit sums repeatedly */
    }
    return (uint32_t)n;
}

/*
 * Sum the factorials of n's digits.
 *
 * Uses a tiny precomputed factorial table for digits 0..9. Numbers equal to
 * this sum are "factorions" (1, 2, 145, 40585).
 *
 * n:      input value.
 * return: the sum of the factorials of n's digits. O(number of digits).
 */
uint64_t digit_factorial_sum(uint64_t n) {
    /* Factorials of single digits, indexed directly by the digit. */
    static const uint64_t fact[10] = {
        1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880
    };
    if (n == 0) {
        return fact[0]; /* the lone digit 0 contributes 0! = 1 */
    }
    uint64_t sum = 0;
    while (n > 0) {
        sum += fact[n % 10];
        n /= 10;
    }
    return sum;
}

/*
 * Sum the squares of n's digits.
 *
 * Helper for the happy-number test; each digit contributes its square.
 *
 * n:      input value.
 * return: the sum of squared digits. O(number of digits).
 */
uint32_t digit_square_sum(uint64_t n) {
    uint32_t sum = 0;
    while (n > 0) {
        uint32_t d = (uint32_t)(n % 10);
        sum += d * d;
        n /= 10;
    }
    return sum;
}

/*
 * Test whether n is a "happy number".
 *
 * Repeatedly replace n with the sum of the squares of its digits; n is
 * happy if this reaches 1. Unhappy numbers fall into the cycle that passes
 * through 4, so detecting 4 is a sufficient and simple stopping condition.
 *
 * n:      input value, assumed >= 1.
 * return: 1 if n is happy, 0 otherwise. Terminates quickly for all inputs.
 */
int is_happy(uint64_t n) {
    while (n != 1 && n != 4) {
        n = digit_square_sum(n);
    }
    return n == 1;
}

/*
 * Apply one step of the look-and-say sequence to a digit string.
 *
 * Reads `in` as runs of identical digits and writes their "say" encoding:
 * "1" -> "11" (one 1), "1211" -> "111221". Run counts are assumed to stay
 * below 10 (true for the canonical sequence), so each run emits two chars.
 *
 * in:       NUL-terminated input digit string.
 * out:      destination buffer for the NUL-terminated result.
 * capacity: size of out in bytes, including room for the terminator.
 * return:   the length of the produced string, or 0 if out of room.
 */
uint32_t look_and_say(const char *in, char *out, uint32_t capacity) {
    uint32_t out_len = 0;
    uint32_t i = 0;
    uint32_t in_len = (uint32_t)strlen(in);
    while (i < in_len) {
        char digit = in[i];
        uint32_t run = 0;
        /* Count the length of the run of this digit. */
        while (i < in_len && in[i] == digit) {
            run++;
            i++;
        }
        /* Need two chars for the count+digit pair plus the terminator. */
        if (out_len + 2 >= capacity) {
            out[out_len] = '\0';
            return 0; /* signal overflow without writing past the buffer */
        }
        out[out_len++] = (char)('0' + run); /* the count digit */
        out[out_len++] = digit;             /* the digit being counted */
    }
    out[out_len] = '\0';
    return out_len;
}
