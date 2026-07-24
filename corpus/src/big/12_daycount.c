/*
 * daycount.c — Serial day numbers and date arithmetic.
 *
 * Converts Gregorian dates to and from a continuous integer day count (days
 * since a fixed epoch). With dates reduced to integers, differences and offset
 * arithmetic become simple subtraction and addition.
 */

#include <stdint.h>

/* A Gregorian calendar date. */
typedef struct {
    int year;
    int month;   /* 1..12 */
    int day;     /* 1..31 */
} Date;

/*
 * isLeap — Internal leap-year predicate (Gregorian rules).
 *
 * Parameters: year — the year to test.
 * Returns: 1 if leap, 0 otherwise.
 */
static int isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * toSerial — Convert a date to a serial day number.
 *
 * Implements a well-known integer algorithm that maps a Gregorian date onto a
 * continuous day count by treating March as the start of the year, which moves
 * the irregular leap day to the very end and removes month-length special
 * cases from the core arithmetic.
 *
 * Parameters:
 *   d — the date to convert (assumed valid).
 * Returns:
 *   A 64-bit serial day number; the absolute epoch is unspecified but stable,
 *   so differences between two serial numbers give an exact day count.
 * Complexity: O(1).
 */
int64_t toSerial(Date d) {
    int y = d.year;
    int m = d.month;

    /* Shift so that March = month 0; Jan/Feb roll into the previous year. */
    int a = (14 - m) / 12;   /* 1 for Jan/Feb, else 0 */
    y = y + 4800 - a;        /* offset to keep all working values positive */
    m = m + 12 * a - 3;      /* remap month to 0..11 with March = 0 */

    /*
     * (153*m + 2)/5 is the running day offset of each remapped month; the
     * year terms add the accumulated leap days. The trailing -32045 anchors
     * the count to the conventional epoch.
     */
    return d.day
         + (153 * (int64_t)m + 2) / 5
         + 365LL * y
         + y / 4
         - y / 100
         + y / 400
         - 32045;
}

/*
 * fromSerial — Convert a serial day number back to a Gregorian date.
 *
 * Exact inverse of toSerial(); the two round-trip for any valid date.
 *
 * Parameters:
 *   serial — a serial day number produced by toSerial().
 * Returns:
 *   The corresponding Date.
 * Complexity: O(1).
 */
Date fromSerial(int64_t serial) {
    int64_t a = serial + 32044;
    int64_t b = (4 * a + 3) / 146097;     /* which 400-year cycle */
    int64_t c = a - (146097 * b) / 4;     /* day within that cycle */

    int64_t y = (4 * c + 3) / 1461;       /* year within the cycle */
    int64_t day_in_year = c - (1461 * y) / 4;
    int64_t m = (5 * day_in_year + 2) / 153;  /* remapped month, March = 0 */

    Date result;
    result.day   = (int)(day_in_year - (153 * m + 2) / 5 + 1);
    result.month = (int)(m + 3 - 12 * (m / 10));  /* undo the March-shift */
    result.year  = (int)(100 * b + y - 4800 + m / 10);
    return result;
}

/*
 * daysBetween — Signed number of days from one date to another.
 *
 * Parameters:
 *   from — the earlier (or reference) date.
 *   to   — the later date.
 * Returns:
 *   to - from in days; negative if `to` precedes `from`.
 */
int64_t daysBetween(Date from, Date to) {
    return toSerial(to) - toSerial(from);
}

/*
 * addDays — Produce the date that lies `offset` days from a starting date.
 *
 * Parameters:
 *   start  — the base date.
 *   offset — number of days to add; may be negative to move backward.
 * Returns:
 *   The resulting Date, with month and year carry handled automatically.
 */
Date addDays(Date start, int64_t offset) {
    return fromSerial(toSerial(start) + offset);
}
