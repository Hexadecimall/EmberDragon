/*
 * nthweekday.cpp — "Nth weekday of the month" scheduling.
 *
 * Resolves rules such as "the third Monday of November" or "the last Friday of
 * the month" into concrete dates, the integer logic behind floating holidays
 * and recurring meeting schedules. All computation is table-driven and O(1).
 */

#include <stdint.h>

/* A Gregorian date; day == 0 is used to signal "no such date". */
struct Date {
    int year;
    int month;   /* 1..12 */
    int day;     /* 1..31, or 0 to mean invalid/absent */
};

/*
 * isLeap — Gregorian leap-year predicate.
 * Parameters: year. Returns: true if leap, false otherwise.
 */
static bool isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * monthLength — Days in a month, leap-aware.
 * Parameters: year, month (1..12). Returns: day count, 0 if month invalid.
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
 * weekdayOf — Day of week for a date: 0 = Sunday ... 6 = Saturday.
 *
 * Uses Sakamoto's algorithm, an integer method that needs no date library.
 *
 * Parameters: d — the date to evaluate.
 * Returns: weekday index in 0..6.
 */
static int weekdayOf(Date d) {
    static const int t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = d.year;
    if (d.month < 3)
        y -= 1;  /* shift Jan/Feb to prior year so leap years align */
    return (y + y / 4 - y / 100 + y / 400 + t[d.month - 1] + d.day) % 7;
}

/*
 * nthWeekdayOfMonth — Find the Nth occurrence of a weekday in a month.
 *
 * Example: nthWeekdayOfMonth(2026, 11, 1, 3) — with weekday 1 meaning Monday —
 * gives the 3rd Monday of November 2026. Computes the first matching weekday,
 * then strides forward by
 * whole weeks; returns an invalid date (day == 0) if the Nth occurrence would
 * fall outside the month.
 *
 * Parameters:
 *   year     — the year.
 *   month    — the month, 1..12.
 *   weekday  — target weekday, 0 = Sunday ... 6 = Saturday.
 *   n        — which occurrence to pick, 1-based (1 = first, 2 = second, ...).
 * Returns:
 *   The resolved Date, or a Date with day == 0 if no such occurrence exists.
 * Complexity: O(1).
 */
Date nthWeekdayOfMonth(int year, int month, int weekday, int n) {
    Date invalid = {year, month, 0};
    if (month < 1 || month > 12 || weekday < 0 || weekday > 6 || n < 1)
        return invalid;

    /* Weekday of the 1st of the month tells us how far the first match is. */
    Date first = {year, month, 1};
    int firstWd = weekdayOf(first);

    /* Days to advance from the 1st to reach the first target weekday (0..6). */
    int offset = (weekday - firstWd + 7) % 7;

    /* The Nth occurrence is the first one plus (n-1) full weeks. */
    int day = 1 + offset + (n - 1) * 7;

    if (day > monthLength(year, month))
        return invalid;  /* the requested occurrence overflows the month */

    Date result = {year, month, day};
    return result;
}

/*
 * lastWeekdayOfMonth — Find the last occurrence of a weekday in a month.
 *
 * Works backward from the final day of the month to the nearest matching
 * weekday, which is the natural way to express rules like "last Friday".
 *
 * Parameters:
 *   year    — the year.
 *   month   — the month, 1..12.
 *   weekday — target weekday, 0 = Sunday ... 6 = Saturday.
 * Returns:
 *   The resolved Date, or a Date with day == 0 on invalid input.
 * Complexity: O(1).
 */
Date lastWeekdayOfMonth(int year, int month, int weekday) {
    Date invalid = {year, month, 0};
    if (month < 1 || month > 12 || weekday < 0 || weekday > 6)
        return invalid;

    int last = monthLength(year, month);
    Date lastDay = {year, month, last};
    int lastWd = weekdayOf(lastDay);

    /* Step back from the last day to the most recent matching weekday. */
    int back = (lastWd - weekday + 7) % 7;
    Date result = {year, month, last - back};
    return result;
}
