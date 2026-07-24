/*
 * number_to_words.c -- Convert non-negative integers to English words.
 *
 * Spells out a number such as 1234 as "one thousand two hundred thirty four".
 * Handles values up to the billions using grouped three-digit scale names.
 * The conversion is purely integer arithmetic (no floating point) and writes
 * into a caller-supplied buffer.
 */

#include <stddef.h>

/* Word forms for the digits 0..19, which English names irregularly. */
static const char *const kOnes[] = {
    "zero", "one", "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen",
    "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"
};

/* Word forms for the multiples of ten from 20..90; index 0/1 are unused. */
static const char *const kTens[] = {
    "", "", "twenty", "thirty", "forty", "fifty",
    "sixty", "seventy", "eighty", "ninety"
};

/* Scale names attached to each three-digit group, smallest group first. */
static const char *const kScales[] = { "", "thousand", "million", "billion" };

/*
 * appendString -- Copy a NUL-terminated word into a buffer at an offset.
 *
 * @out:  destination buffer.
 * @cap:  total capacity of `out`.
 * @idx:  current write position (updated in place).
 * @word: the text to append.
 *
 * Stops at the buffer limit without overflowing. The terminator is not
 * written here; the top-level function finalizes it. O(length of word).
 */
static void appendString(char *out, size_t cap, size_t *idx, const char *word) {
    size_t i = 0;
    while (word[i] != '\0' && *idx + 1 < cap) {
        out[(*idx)++] = word[i++];
    }
}

/*
 * appendSpaceIfNeeded -- Insert a separating space between words.
 *
 * @out: destination buffer.
 * @cap: capacity of `out`.
 * @idx: current write position (updated in place).
 *
 * Only writes when there is already content, so the result never has a leading
 * space. Keeps the spelled-out phrase tidy.
 */
static void appendSpaceIfNeeded(char *out, size_t cap, size_t *idx) {
    if (*idx > 0 && *idx + 1 < cap) {
        out[(*idx)++] = ' ';
    }
}

/*
 * spellGroup -- Spell a value in [0, 999] into the buffer.
 *
 * @out:   destination buffer.
 * @cap:   capacity of `out`.
 * @idx:   current write position (updated in place).
 * @group: the three-digit value to spell; nothing is written if it is 0.
 *
 * Emits the hundreds, then the tens/ones, handling the 10..19 teens as a
 * single special case. O(1) work (bounded by three digits).
 */
static void spellGroup(char *out, size_t cap, size_t *idx, int group) {
    int hundreds = group / 100;
    int rest = group % 100;

    if (hundreds > 0) {
        appendSpaceIfNeeded(out, cap, idx);
        appendString(out, cap, idx, kOnes[hundreds]);
        appendSpaceIfNeeded(out, cap, idx);
        appendString(out, cap, idx, "hundred");
    }

    if (rest >= 20) {
        appendSpaceIfNeeded(out, cap, idx);
        appendString(out, cap, idx, kTens[rest / 10]);
        if (rest % 10 != 0) {
            appendSpaceIfNeeded(out, cap, idx);
            appendString(out, cap, idx, kOnes[rest % 10]);
        }
    } else if (rest > 0) {
        /* 1..19 are named directly from the ones table (covers the teens). */
        appendSpaceIfNeeded(out, cap, idx);
        appendString(out, cap, idx, kOnes[rest]);
    }
}

/*
 * numberToWords -- Spell a non-negative integer into English words.
 *
 * @out:   destination buffer for the NUL-terminated phrase.
 * @cap:   capacity of `out` in bytes.
 * @value: the number to spell, in [0, 999,999,999,999].
 *
 * Returns the number of characters written (excluding the terminator). The
 * special case zero yields "zero". The integer is split into three-digit
 * groups from least to most significant, each tagged with its scale word.
 * O(number of digits).
 */
size_t numberToWords(char *out, size_t cap, unsigned long long value) {
    size_t idx = 0;
    if (cap == 0) {
        return 0;
    }

    if (value == 0) {
        appendString(out, cap, &idx, kOnes[0]);   /* "zero" */
        out[idx] = '\0';
        return idx;
    }

    /* Decompose into up to four groups of three digits. */
    int groups[4];
    int groupCount = 0;
    while (value > 0 && groupCount < 4) {
        groups[groupCount++] = (int)(value % 1000);
        value /= 1000;
    }

    /* Walk groups from most significant to least, appending scale names. */
    for (int g = groupCount - 1; g >= 0; g--) {
        if (groups[g] == 0) {
            continue;           /* skip empty groups, e.g. the 0 in 1,000,001 */
        }
        spellGroup(out, cap, &idx, groups[g]);
        if (g > 0) {
            appendSpaceIfNeeded(out, cap, &idx);
            appendString(out, cap, &idx, kScales[g]);
        }
    }

    out[idx] = '\0';
    return idx;
}
