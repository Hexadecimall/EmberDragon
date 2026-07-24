/*
 * validate.c — Calendar date validation and normalization.
 *
 * Provides strict validation of Gregorian dates and a normalizer that carries
 * overflowing day/month values into higher fields, so that loosely-constructed
 * dates (e.g. month 13, day 40) can be folded into a canonical valid date.
 */

#include <stdint.h>

/* A date plus a validity flag, returned by normalization routines. */
typedef struct {
    int year;
    int month;
    int day;
} CalDate;

/*
 * isLeap — Gregorian leap-year predicate.
 * Parameters: year. Returns: 1 if leap, 0 otherwise.
 */
static int isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * monthLength — Number of days in a month, leap-aware.
 *
 * Parameters:
 *   year  — the owning year (affects February).
 *   month — 1..12.
 * Returns:
 *   Day count for the month, or 0 if the month index is out of range.
 */
static int monthLength(int year, int month) {
    static const int len[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (month < 1 || month > 12)
        return 0;
    if (month == 2 && isLeap(year))
        return 29;
    return len[month];
}

/*
 * isValidDate — Strictly check whether (year, month, day) is a real date.
 *
 * Parameters:
 *   year  — any integer year.
 *   month — must be 1..12.
 *   day   — must be 1..monthLength(year, month).
 * Returns:
 *   1 if the triple names an existing calendar day, 0 otherwise.
 */
int isValidDate(int year, int month, int day) {
    if (month < 1 || month > 12)
        return 0;
    int maxDay = monthLength(year, month);
    if (day < 1 || day > maxDay)
        return 0;
    return 1;
}

/*
 * normalizeMonths — Fold an out-of-range month into the year.
 *
 * Helper that carries a month value outside 1..12 into the year field, leaving
 * the month in canonical 1..12 range. Handles both overflow (month > 12) and
 * underflow (month < 1) using floor division so negative months work too.
 *
 * Parameters:
 *   date — pointer to the date to adjust in place (must be non-null).
 */
static void normalizeMonths(CalDate *date) {
    /* Convert to a zero-based month index for clean floor arithmetic. */
    int monthIndex = date->month - 1;
    int yearCarry = monthIndex / 12;
    monthIndex = monthIndex % 12;
    if (monthIndex < 0) {
        /* C truncates toward zero; correct the sign to emulate floor division. */
        monthIndex += 12;
        yearCarry -= 1;
    }
    date->year += yearCarry;
    date->month = monthIndex + 1;
}

/*
 * normalizeDate — Carry overflowing fields to produce a canonical valid date.
 *
 * First normalizes the month so it is in 1..12, then repeatedly carries excess
 * days forward into later months (and deficit days back into earlier months)
 * until the day falls within the current month's length. The loop runs once per
 * month spanned, which is bounded by the magnitude of the day overflow.
 *
 * Parameters:
 *   input — the loosely-specified date to canonicalize.
 * Returns:
 *   An equivalent CalDate with month in 1..12 and day in 1..monthLength.
 * Complexity: O(d) in the number of months the day field spans.
 */
CalDate normalizeDate(CalDate input) {
    CalDate d = input;
    normalizeMonths(&d);

    /* Carry days forward while the day exceeds the current month's length. */
    while (d.day > monthLength(d.year, d.month)) {
        d.day -= monthLength(d.year, d.month);
        d.month += 1;
        if (d.month > 12) {     /* rolled past December: bump the year */
            d.month = 1;
            d.year += 1;
        }
    }

    /* Carry days backward while the day is below 1 (borrow from prior month). */
    while (d.day < 1) {
        d.month -= 1;
        if (d.month < 1) {      /* rolled before January: drop a year */
            d.month = 12;
            d.year -= 1;
        }
        d.day += monthLength(d.year, d.month);
    }

    return d;
}
