/*
 * leapyear.c — Gregorian leap-year rules and month-length queries.
 *
 * Encapsulates the three nested rules of the Gregorian calendar (divisible by
 * 4, except centuries, except every 400 years) and builds month-length and
 * year-length helpers on top of that single source of truth.
 */

#include <stdint.h>

/*
 * isLeapYear — Decide whether a Gregorian year has a February 29.
 *
 * Applies the standard rule cascade: a year is a leap year if it is divisible
 * by 4, unless it is divisible by 100, unless it is also divisible by 400.
 *
 * Parameters:
 *   year — the proleptic Gregorian year (negative years allowed).
 * Returns:
 *   1 if a leap year, 0 otherwise.
 * Complexity: O(1).
 */
int isLeapYear(int year) {
    if (year % 400 == 0)
        return 1;            /* every 400 years is always a leap year */
    if (year % 100 == 0)
        return 0;            /* but plain centuries are not */
    return (year % 4 == 0) ? 1 : 0;  /* otherwise, every 4th year qualifies */
}

/*
 * daysInMonth — Length of a given month, accounting for leap years.
 *
 * Parameters:
 *   year  — the year the month belongs to (affects February only).
 *   month — 1 = January ... 12 = December.
 * Returns:
 *   Number of days in the month, or 0 if the month index is out of range.
 */
int daysInMonth(int year, int month) {
    /* Lengths for a non-leap year, indexed 1..12 (index 0 is a filler). */
    static const int lengths[13] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (month < 1 || month > 12)
        return 0;                    /* invalid month: report no days */
    if (month == 2 && isLeapYear(year))
        return 29;                   /* February gains a day in leap years */
    return lengths[month];
}

/*
 * daysInYear — Total number of days in a year.
 *
 * Parameters:
 *   year — the year to measure.
 * Returns:
 *   366 for a leap year, 365 otherwise.
 */
int daysInYear(int year) {
    return isLeapYear(year) ? 366 : 365;
}

/*
 * leapYearsInRange — Count leap years in the half-open interval [start, end).
 *
 * Computed in closed form using the inclusion-exclusion of the /4, /100, /400
 * rules, so the cost does not grow with the size of the range. The helper
 * countUpTo() counts leap years in [1, n] for non-negative n.
 *
 * Parameters:
 *   start — first year of the range (inclusive).
 *   end   — one past the last year of the range (exclusive).
 * Returns:
 *   The number of leap years in [start, end); 0 if the range is empty or
 *   inverted, or if either endpoint is negative (unsupported here).
 * Complexity: O(1).
 */
int leapYearsInRange(int start, int end) {
    if (start >= end || start < 0)
        return 0;  /* empty/inverted range, or out-of-domain input */

    /* Leap years in [1, n]: multiples of 4, minus 100s, plus 400s. */
    int endCount = (end - 1) / 4 - (end - 1) / 100 + (end - 1) / 400;
    int startCount = (start - 1) / 4 - (start - 1) / 100 + (start - 1) / 400;

    /* Difference gives the count strictly inside the half-open window. */
    return endCount - startCount;
}
