/*
 * zeller.c — Day-of-week computation via Zeller's congruence.
 *
 * Given a proleptic Gregorian calendar date, this module determines the
 * weekday using Zeller's congruence, an integer-only formula that needs no
 * lookup tables or iteration. It also exposes a small helper for printing
 * weekday names so callers can render results directly.
 */

#include <stdio.h>

/* A plain calendar date in the Gregorian system. */
typedef struct {
    int year;   /* full year, e.g. 2026 (proleptic Gregorian) */
    int month;  /* 1 = January ... 12 = December */
    int day;    /* day of month, 1..31 */
} GregorianDate;

/*
 * dayOfWeek — Compute the weekday index for a Gregorian date.
 *
 * Uses Zeller's congruence. Internally January and February are treated as
 * months 13 and 14 of the *previous* year, which keeps the leap-day handling
 * at the end of the shifted year where the formula expects it.
 *
 * Parameters:
 *   date — the date to evaluate (fields assumed already valid).
 * Returns:
 *   Weekday index in 0..6 where 0 = Sunday, 1 = Monday, ... 6 = Saturday.
 * Complexity: O(1).
 */
int dayOfWeek(GregorianDate date) {
    int m = date.month;
    int y = date.year;

    /* Shift Jan/Feb into the tail of the prior year so leap days line up. */
    if (m < 3) {
        m += 12;
        y -= 1;
    }

    int k = y % 100;   /* year within the century */
    int j = y / 100;   /* the zero-based century */

    /*
     * Classic Zeller term. (13*(m+1))/5 advances the weekday by the right
     * amount for each month's length; the century terms account for the
     * Gregorian leap-year corrections at 100- and 400-year boundaries.
     */
    int h = (date.day
             + (13 * (m + 1)) / 5
             + k
             + k / 4
             + j / 4
             + 5 * j) % 7;

    /*
     * Zeller's h yields 0 = Saturday. Re-base so that 0 = Sunday, which is the
     * convention most callers expect. Adding 6 before the modulo keeps the
     * intermediate value non-negative for all inputs.
     */
    return (h + 6) % 7;
}

/*
 * weekdayName — Map a weekday index to its English name.
 *
 * Parameters:
 *   index — weekday in 0..6 (0 = Sunday), as returned by dayOfWeek().
 * Returns:
 *   A pointer to a static string literal; "?" if the index is out of range.
 *   The caller must not free or modify the returned string.
 */
const char *weekdayName(int index) {
    static const char *names[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };
    if (index < 0 || index > 6)
        return "?";  /* defensive: guard against malformed indices */
    return names[index];
}

/*
 * isWeekend — Report whether a date falls on Saturday or Sunday.
 *
 * Parameters:
 *   date — the date to test.
 * Returns:
 *   1 if the date is a weekend day, 0 otherwise.
 */
int isWeekend(GregorianDate date) {
    int dow = dayOfWeek(date);
    return (dow == 0 || dow == 6) ? 1 : 0;  /* Sunday or Saturday */
}
