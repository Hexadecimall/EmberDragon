/*
 * ordinal_suffix.cpp -- Format integers as English ordinals (1st, 2nd, 3rd...).
 *
 * Produces the ordinal form of a non-negative integer by choosing the correct
 * two-letter suffix ("st", "nd", "rd", "th") according to English rules,
 * including the irregular 11th/12th/13th cases. Everything is integer-based
 * and writes into a caller-provided buffer.
 */

#include <cstddef>

namespace ordinal {

/* The four possible ordinal suffixes, indexed by a small enum for clarity. */
enum class Suffix { St, Nd, Rd, Th };

/*
 * suffixText -- Map a Suffix enum to its two-character string.
 *
 * @s: the suffix selector.
 * Returns a static NUL-terminated string ("st", "nd", "rd", or "th"). O(1).
 */
static const char *suffixText(Suffix s) {
    switch (s) {
        case Suffix::St: return "st";
        case Suffix::Nd: return "nd";
        case Suffix::Rd: return "rd";
        default:         return "th";
    }
}

/*
 * chooseSuffix -- Decide which ordinal suffix a number takes.
 *
 * @value: a non-negative integer.
 * Returns the matching Suffix.
 *
 * The rule: numbers whose last two digits are 11, 12, or 13 always take "th"
 * (eleventh, twelfth, thirteenth); otherwise the last digit decides -- 1->st,
 * 2->nd, 3->rd, everything else->th. O(1).
 */
static Suffix chooseSuffix(unsigned long value) {
    unsigned lastTwo = static_cast<unsigned>(value % 100);
    unsigned lastOne = static_cast<unsigned>(value % 10);

    /* The teens are the famous exception and must be checked first. */
    if (lastTwo >= 11 && lastTwo <= 13) {
        return Suffix::Th;
    }
    switch (lastOne) {
        case 1:  return Suffix::St;
        case 2:  return Suffix::Nd;
        case 3:  return Suffix::Rd;
        default: return Suffix::Th;
    }
}

/*
 * writeDigits -- Write the decimal digits of `value` into `out`.
 *
 * @out: destination buffer.
 * @cap: capacity of `out`.
 * @idx: write position (updated in place).
 * @value: the number whose digits to emit.
 *
 * Produces digits in correct most-significant-first order by recursing on the
 * higher-order part before printing the final digit. Stops if the buffer fills.
 * O(number of digits), with recursion depth equal to the digit count.
 */
static void writeDigits(char *out, std::size_t cap, std::size_t &idx,
                        unsigned long value) {
    if (value >= 10) {
        writeDigits(out, cap, idx, value / 10);  /* higher digits first */
    }
    if (idx + 1 < cap) {
        out[idx++] = static_cast<char>('0' + static_cast<int>(value % 10));
    }
}

/*
 * formatOrdinal -- Render `value` as an ordinal string, e.g. 23 -> "23rd".
 *
 * @out:   destination buffer for the NUL-terminated ordinal.
 * @cap:   capacity of `out` in bytes.
 * @value: the non-negative number to format.
 *
 * Returns the number of characters written (excluding the terminator). The
 * numeric part is written first, then the two-letter suffix chosen by the
 * English rules. The result is always terminated when cap > 0. O(digits).
 */
std::size_t formatOrdinal(char *out, std::size_t cap, unsigned long value) {
    if (cap == 0) {
        return 0;
    }

    std::size_t idx = 0;
    writeDigits(out, cap, idx, value);

    const char *suffix = suffixText(chooseSuffix(value));
    for (std::size_t i = 0; suffix[i] != '\0'; i++) {
        if (idx + 1 < cap) {
            out[idx++] = suffix[i];
        }
    }

    out[idx] = '\0';
    return idx;
}

} // namespace ordinal
