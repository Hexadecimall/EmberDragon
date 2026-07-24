/*
 * recurring.cpp — Recurring-event expansion over a date window.
 *
 * Models a repeating calendar event (daily / weekly / monthly) and expands its
 * occurrences into an integer array of serial day numbers bounded by a query
 * window. All date math is integer-only and table-free.
 */

#include <stdint.h>
#include <stdlib.h>

/* The recurrence cadence of an event. */
enum class Frequency {
    Daily = 0,
    Weekly = 1,
    Monthly = 2
};

/* A recurring event anchored at a start day, repeating every `interval` units. */
struct RecurringEvent {
    int64_t startSerial;   /* serial day number of the first occurrence */
    Frequency frequency;   /* unit of repetition */
    int interval;          /* repeat every N units; must be >= 1 */
};

/*
 * isLeap — Gregorian leap-year predicate.
 * Parameters: year — the year to test. Returns: 1 if leap, 0 otherwise.
 */
static int isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * serialToYearMonth — Recover (year, month) from a serial day number.
 *
 * Only the year and month fields are needed by the monthly-stepping logic, so
 * the day-of-month is computed but discarded.
 *
 * Parameters:
 *   serial   — serial day number.
 *   outYear  — receives the year (must be non-null).
 *   outMonth — receives the month 1..12 (must be non-null).
 */
static void serialToYearMonth(int64_t serial, int *outYear, int *outMonth) {
    int64_t a = serial + 32044;
    int64_t b = (4 * a + 3) / 146097;
    int64_t c = a - (146097 * b) / 4;
    int64_t y = (4 * c + 3) / 1461;
    int64_t dInYear = c - (1461 * y) / 4;
    int64_t m = (5 * dInYear + 2) / 153;
    *outMonth = (int)(m + 3 - 12 * (m / 10));
    *outYear = (int)(100 * b + y - 4800 + m / 10);
}

/*
 * daysInMonth — Length of a month, leap-year aware.
 * Parameters: year, month (1..12). Returns: day count, 0 if month invalid.
 */
static int daysInMonth(int year, int month) {
    static const int len[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (month < 1 || month > 12) return 0;
    if (month == 2 && isLeap(year)) return 29;
    return len[month];
}

/*
 * nextOccurrence — Advance a serial day to the next occurrence of an event.
 *
 * Daily and weekly steps are constant-size jumps. Monthly recurrence must add
 * whole months while preserving the day-of-month, clamping to the last valid
 * day when the target month is shorter (e.g. Jan 31 + 1 month -> Feb 28/29).
 *
 * Parameters:
 *   current — the current occurrence's serial day.
 *   ev      — the event describing the cadence.
 * Returns:
 *   The serial day of the following occurrence.
 */
static int64_t nextOccurrence(int64_t current, const RecurringEvent &ev) {
    if (ev.frequency == Frequency::Daily)
        return current + (int64_t)ev.interval;
    if (ev.frequency == Frequency::Weekly)
        return current + (int64_t)ev.interval * 7;

    /* Monthly: decompose, advance whole months, then clamp the day. */
    int year, month;
    serialToYearMonth(current, &year, &month);

    /* Recover the original day-of-month from the current serial. */
    int64_t firstOfMonth;
    {
        /* Rebuild the serial of day 1 of (year, month) for the offset. */
        int y = year, m = month;
        int a = (14 - m) / 12;
        y = y + 4800 - a;
        m = m + 12 * a - 3;
        firstOfMonth = 1 + (153LL * m + 2) / 5 + 365LL * y
                     + y / 4 - y / 100 + y / 400 - 32045;
    }
    int dayOfMonth = (int)(current - firstOfMonth) + 1;

    /* Step forward by interval months, carrying into the year as needed. */
    int totalMonths = (year * 12 + (month - 1)) + ev.interval;
    int newYear = totalMonths / 12;
    int newMonth = totalMonths % 12 + 1;

    /* Clamp the day so we never land on a nonexistent date. */
    int maxDay = daysInMonth(newYear, newMonth);
    int newDay = dayOfMonth < maxDay ? dayOfMonth : maxDay;

    /* Re-encode (newYear, newMonth, newDay) into a serial day number. */
    {
        int a = (14 - newMonth) / 12;
        int y = newYear + 4800 - a;
        int m = newMonth + 12 * a - 3;
        return newDay + (153LL * m + 2) / 5 + 365LL * y
             + y / 4 - y / 100 + y / 400 - 32045;
    }
}

/*
 * expandOccurrences — Collect all event occurrences within [windowStart, windowEnd].
 *
 * Walks the recurrence forward from its anchor, skipping occurrences before the
 * window and collecting those inside it, until it passes windowEnd.
 *
 * Parameters:
 *   ev          — the recurring event (interval must be >= 1).
 *   windowStart — inclusive lower bound, as a serial day number.
 *   windowEnd   — inclusive upper bound, as a serial day number.
 *   outCount    — receives the number of occurrences found (must be non-null).
 * Returns:
 *   A heap-allocated array of serial day numbers of length *outCount, or
 *   nullptr if there are none or on invalid input. The CALLER OWNS and must
 *   free() the returned array.
 * Complexity: O(k) where k is the number of occurrences scanned.
 */
int64_t *expandOccurrences(const RecurringEvent &ev,
                           int64_t windowStart, int64_t windowEnd,
                           int *outCount) {
    *outCount = 0;
    if (ev.interval < 1 || windowStart > windowEnd)
        return nullptr;  /* malformed cadence or empty window */

    int capacity = 8;
    int64_t *result = (int64_t *)malloc((size_t)capacity * sizeof(int64_t));
    if (!result)
        return nullptr;  /* allocation failure: report no occurrences */

    int64_t cur = ev.startSerial;

    /* Fast-forward over occurrences that fall before the window opens. */
    while (cur < windowStart)
        cur = nextOccurrence(cur, ev);

    /* Collect every occurrence up to and including windowEnd. */
    int n = 0;
    while (cur <= windowEnd) {
        if (n == capacity) {
            /* Grow geometrically to keep amortized append cost constant. */
            capacity *= 2;
            int64_t *bigger = (int64_t *)realloc(result,
                                  (size_t)capacity * sizeof(int64_t));
            if (!bigger) {       /* realloc failed: return what we have so far */
                *outCount = n;
                return result;
            }
            result = bigger;
        }
        result[n++] = cur;
        cur = nextOccurrence(cur, ev);
    }

    *outCount = n;
    if (n == 0) {
        free(result);            /* nothing landed in the window */
        return nullptr;
    }
    return result;
}
