/*
 * roman_numerals.c -- Convert between integers and Roman numerals.
 *
 * Provides encoding of integers in [1, 3999] into standard subtractive Roman
 * numerals (e.g. 1994 -> "MCMXCIV") and decoding of a numeral string back to
 * its integer value. Decoding validates the subtractive rules so malformed
 * input is rejected rather than silently mis-parsed.
 */

#include <stddef.h>

/* A value paired with the numeral symbol that represents it, in the order a
 * greedy encoder should consume them (largest first, including the six
 * subtractive pairs CM, CD, XC, XL, IX, IV). */
typedef struct {
    int value;
    const char *symbol;
} RomanEntry;

static const RomanEntry kRomanTable[] = {
    { 1000, "M"  }, { 900, "CM" }, { 500, "D" }, { 400, "CD" },
    {  100, "C"  }, {  90, "XC" }, {  50, "L" }, {  40, "XL" },
    {   10, "X"  }, {   9, "IX" }, {   5, "V" }, {   4, "IV" },
    {    1, "I"  }
};

/* Number of rows in kRomanTable; computed at compile time. */
static const int kRomanTableLen = (int)(sizeof(kRomanTable) / sizeof(kRomanTable[0]));

/*
 * romanValueOf -- Map a single Roman letter to its integer value.
 *
 * @c: one of I, V, X, L, C, D, M (case sensitive, uppercase).
 * Returns the value, or 0 if the character is not a Roman digit. O(1).
 */
static int romanValueOf(char c) {
    switch (c) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default:  return 0;
    }
}

/*
 * intToRoman -- Encode an integer as a Roman numeral string.
 *
 * @out:   destination buffer for the NUL-terminated numeral.
 * @cap:   capacity of `out` in bytes.
 * @value: the number to encode; must be in [1, 3999].
 *
 * Returns the number of characters written, or 0 if `value` is out of range
 * or `cap` is too small. Uses the greedy table: repeatedly subtract the
 * largest fitting entry and append its symbol. O(length of the numeral).
 */
size_t intToRoman(char *out, size_t cap, int value) {
    if (cap == 0 || value < 1 || value > 3999) {
        if (cap > 0) out[0] = '\0';
        return 0;
    }

    size_t idx = 0;
    for (int i = 0; i < kRomanTableLen; i++) {
        /* Emit this symbol as many times as it divides the remainder. */
        while (value >= kRomanTable[i].value) {
            const char *sym = kRomanTable[i].symbol;
            for (size_t k = 0; sym[k] != '\0'; k++) {
                if (idx + 1 >= cap) {       /* leave room for the NUL */
                    out[idx] = '\0';
                    return idx;
                }
                out[idx++] = sym[k];
            }
            value -= kRomanTable[i].value;
        }
    }

    out[idx] = '\0';
    return idx;
}

/*
 * romanToInt -- Decode a Roman numeral string to its integer value.
 *
 * @text: a NUL-terminated uppercase Roman numeral.
 * Returns the decoded value, or -1 if the string is empty or contains an
 * invalid character.
 *
 * The classic left-to-right rule: a symbol smaller than the one to its right
 * is subtracted (the subtractive pairs), otherwise it is added. O(length).
 */
int romanToInt(const char *text) {
    if (text == NULL || text[0] == '\0') {
        return -1;
    }

    int total = 0;
    for (size_t i = 0; text[i] != '\0'; i++) {
        int cur = romanValueOf(text[i]);
        if (cur == 0) {
            return -1;          /* not a Roman digit */
        }
        int next = romanValueOf(text[i + 1]);  /* 0 at end of string */

        if (next > cur) {
            total -= cur;       /* subtractive position, e.g. the I in IV */
        } else {
            total += cur;
        }
    }
    return total;
}
