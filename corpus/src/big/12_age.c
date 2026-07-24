/*
 * age.c — Calendar age and elapsed-interval computation.
 *
 * Computes a person's age (or any elapsed duration) between two dates in whole
 * years, months, and days, using borrow-based subtraction that respects real
 * month lengths and leap years. Also reports total whole months elapsed.
 */

#include <stdint.h>

/* A Gregorian date. */
typedef struct {
    int year;
    int month;   /* 1..12 */
    int day;     /* 1..31 */
} Date;

/* A calendar interval broken into whole years, months, and days. */
typedef struct {
    int years;
    int months;
    int days;
} Duration;

/*
 * isLeap — Gregorian leap-year predicate.
 * Parameters: year. Returns: 1 if leap, 0 otherwise.
 */
static int isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * monthLength — Days in a given month, leap-aware.
 *
 * Parameters: year, month (1..12).
 * Returns: day count, or 0 if month is out of range.
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
 * compareDates — Order two dates chronologically.
 *
 * Parameters:
 *   a, b — the dates to compare.
 * Returns:
 *   Negative if a < b, 0 if equal, positive if a > b.
 */
int compareDates(Date a, Date b) {
    if (a.year != b.year)   return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

/*
 * dateDifference — Elapsed time from `start` to `end` in years/months/days.
 *
 * Subtracts field by field from the least significant upward, borrowing from
 * the next field whenever a component goes negative. The day borrow pulls the
 * length of the month *preceding* `end`'s month, which is what makes the result
 * match how people naturally count calendar age.
 *
 * Parameters:
 *   start — the earlier date (e.g. a birth date).
 *   end   — the later date (e.g. today).
 * Returns:
 *   A Duration with non-negative years, months, and days. If `end` precedes
 *   `start`, all fields are returned as 0.
 * Complexity: O(1).
 */
Duration dateDifference(Date start, Date end) {
    Duration d = {0, 0, 0};
    if (compareDates(end, start) < 0)
        return d;  /* negative intervals are not represented here */

    int years = end.year - start.year;
    int months = end.month - start.month;
    int days = end.day - start.day;

    /* Borrow days from the month before `end` if the day component is negative. */
    if (days < 0) {
        months -= 1;
        int borrowMonth = end.month - 1;
        int borrowYear = end.year;
        if (borrowMonth < 1) {     /* underflow before January -> December prior */
            borrowMonth = 12;
            borrowYear -= 1;
        }
        days += monthLength(borrowYear, borrowMonth);
    }

    /* Borrow a year's worth of months if the month component is negative. */
    if (months < 0) {
        years -= 1;
        months += 12;
    }

    d.years = years;
    d.months = months;
    d.days = days;
    return d;
}

/*
 * totalMonthsBetween — Whole calendar months from `start` to `end`.
 *
 * Counts complete months only: a partial final month (where end's day-of-month
 * has not yet reached start's) is not counted.
 *
 * Parameters:
 *   start — the earlier date.
 *   end   — the later date.
 * Returns:
 *   The number of complete months elapsed, or 0 if end precedes start.
 */
int totalMonthsBetween(Date start, Date end) {
    if (compareDates(end, start) < 0)
        return 0;
    int months = (end.year - start.year) * 12 + (end.month - start.month);
    /* If we haven't reached the anniversary day this month, drop one month. */
    if (end.day < start.day)
        months -= 1;
    return months < 0 ? 0 : months;
}
