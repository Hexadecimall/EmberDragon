/*
 * isoweek.c — ISO 8601 week-numbering and ordinal-day utilities.
 *
 * Computes the ISO 8601 week number and week-based year for a Gregorian date.
 * ISO weeks start on Monday, and week 1 is the week containing the year's first
 * Thursday, so a date in early January can belong to the previous ISO year.
 */

#include <stdint.h>

/* A Gregorian date. */
typedef struct {
    int year;
    int month;   /* 1..12 */
    int day;     /* 1..31 */
} Date;

/* The ISO-week coordinates of a date. */
typedef struct {
    int weekYear;  /* the year that owns the ISO week (may differ from .year) */
    int week;      /* ISO week number, 1..53 */
} IsoWeek;

/*
 * isLeap — Gregorian leap-year predicate.
 * Parameters: year. Returns: 1 if leap, 0 otherwise.
 */
static int isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * ordinalDay — Day-of-year for a date (Jan 1 == 1).
 *
 * Parameters:
 *   d — the date to evaluate.
 * Returns:
 *   The ordinal day in 1..365 (or 1..366 in a leap year).
 * Complexity: O(1) — uses a cumulative month-offset table.
 */
int ordinalDay(Date d) {
    /* Cumulative days before the start of each month in a non-leap year. */
    static const int cumulative[13] = {
        0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };
    int doy = cumulative[d.month] + d.day;
    /* After February in a leap year, every ordinal day shifts forward by one. */
    if (d.month > 2 && isLeap(d.year))
        doy += 1;
    return doy;
}

/*
 * isoWeekday — ISO weekday for a date: Monday = 1 ... Sunday = 7.
 *
 * Uses an integer day-of-week computation (Sakamoto's method) and remaps the
 * Sunday=0 result into the ISO Monday-based 1..7 range.
 *
 * Parameters:
 *   d — the date to evaluate.
 * Returns:
 *   ISO weekday in 1..7.
 */
int isoWeekday(Date d) {
    /* Per-month magic offsets used by Sakamoto's day-of-week algorithm. */
    static const int t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = d.year;
    if (d.month < 3)
        y -= 1;  /* treat Jan/Feb as part of the previous year for leap math */
    int dow = (y + y / 4 - y / 100 + y / 400 + t[d.month - 1] + d.day) % 7;
    /* dow has 0 = Sunday; convert to ISO where Monday = 1 and Sunday = 7. */
    return (dow == 0) ? 7 : dow;
}

/*
 * isoWeek — Compute the ISO 8601 week number and owning week-year.
 *
 * The core identity: week = (ordinalDay - isoWeekday + 10) / 7. A result of 0
 * means the date actually belongs to the last week of the previous year, and a
 * result of 53 may belong to week 1 of the next year unless the current year is
 * long enough to have a real week 53.
 *
 * Parameters:
 *   d — the date to evaluate.
 * Returns:
 *   An IsoWeek with weekYear and week filled in.
 * Complexity: O(1).
 */
IsoWeek isoWeek(Date d) {
    IsoWeek out;
    int doy = ordinalDay(d);
    int wd = isoWeekday(d);

    /*
     * The "+10" centers the week boundary on Thursday: it shifts the ordinal
     * so that integer division by 7 lands every Mon-Sun block in the week that
     * owns its Thursday, which is the ISO definition of week 1.
     */
    int week = (doy - wd + 10) / 7;

    if (week < 1) {
        /* Belongs to the final week of the previous ISO year. */
        out.weekYear = d.year - 1;
        /* A year has 53 ISO weeks iff Jan 1 is Thursday, or it's a leap year
         * whose Jan 1 is Wednesday; check that condition for the prior year. */
        int py = d.year - 1;
        int jan1 = isoWeekday((Date){py, 1, 1});
        out.week = (jan1 == 4 || (jan1 == 3 && isLeap(py))) ? 53 : 52;
    } else if (week > 52) {
        /* Might roll over into week 1 of the next year. */
        int jan1 = isoWeekday((Date){d.year, 1, 1});
        int has53 = (jan1 == 4 || (jan1 == 3 && isLeap(d.year)));
        if (has53) {
            out.weekYear = d.year;
            out.week = 53;
        } else {
            out.weekYear = d.year + 1;
            out.week = 1;
        }
    } else {
        out.weekYear = d.year;
        out.week = week;
    }
    return out;
}
