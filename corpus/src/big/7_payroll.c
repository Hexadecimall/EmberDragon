/*
 * payroll.c — Hourly payroll calculator with overtime and progressive tax.
 *
 * This module computes net pay for hourly employees. Gross pay applies a 1.5x
 * overtime multiplier to hours worked beyond a weekly threshold, then a simple
 * progressive bracket table withholds tax. All money is in integer cents and
 * all rates are in basis points to stay free of floating point.
 */

#include <stdint.h>
#include <string.h>

#define NAME_LEN        24
#define OVERTIME_HOURS  40   /* weekly hours before overtime kicks in       */
#define MAX_BRACKETS    4    /* number of progressive tax brackets supported */

/* One employee's pay-relevant configuration. */
typedef struct {
    char    name[NAME_LEN];      /* NUL-terminated display name              */
    int64_t hourly_rate_cents;   /* base pay per hour, in cents              */
} Employee;

/*
 * A single progressive tax bracket. Income up to `upper_cents` (per period) is
 * taxed at `rate_bp` basis points. The final bracket should use a very large
 * upper bound to act as the catch-all top rate.
 */
typedef struct {
    int64_t upper_cents;   /* inclusive upper edge of this bracket          */
    int32_t rate_bp;       /* marginal rate in basis points                 */
} TaxBracket;

/* The withholding schedule: an ordered list of ascending brackets. */
typedef struct {
    TaxBracket brackets[MAX_BRACKETS];
    int        count;
} TaxTable;

/*
 * Initialize an employee record, copying the name with truncation safety.
 */
void employee_init(Employee *e, const char *name, int64_t hourly_rate_cents) {
    strncpy(e->name, name, NAME_LEN - 1);
    e->name[NAME_LEN - 1] = '\0';
    e->hourly_rate_cents  = hourly_rate_cents;
}

/* Start an empty tax table. */
void tax_table_init(TaxTable *t) {
    t->count = 0;
}

/*
 * Append a bracket to the table. Brackets must be added in ascending order of
 * `upper_cents` for the progressive computation to be correct. Returns 1 on
 * success, 0 if the table is full.
 */
int tax_table_add(TaxTable *t, int64_t upper_cents, int32_t rate_bp) {
    if (t->count >= MAX_BRACKETS)
        return 0;
    t->brackets[t->count].upper_cents = upper_cents;
    t->brackets[t->count].rate_bp     = rate_bp;
    t->count++;
    return 1;
}

/*
 * Compute gross pay in cents for `hours` worked in a week. Hours beyond
 * OVERTIME_HOURS are paid at 1.5x, implemented as base + base/2 to avoid
 * fractional multipliers. Negative hours are treated as zero.
 */
int64_t payroll_gross(const Employee *e, int32_t hours) {
    if (hours <= 0)
        return 0;

    int32_t regular  = hours > OVERTIME_HOURS ? OVERTIME_HOURS : hours;
    int32_t overtime = hours > OVERTIME_HOURS ? hours - OVERTIME_HOURS : 0;

    int64_t pay = (int64_t)regular * e->hourly_rate_cents;
    /* Overtime = base rate + half the base rate, i.e. 1.5x, integer-only. */
    int64_t ot_rate = e->hourly_rate_cents + e->hourly_rate_cents / 2;
    pay += (int64_t)overtime * ot_rate;
    return pay;
}

/*
 * Withhold tax from `gross` cents using the progressive table. Each bracket
 * taxes only the slice of income that falls within it, accumulating the total
 * withholding. Returns the tax owed in cents. O(brackets).
 */
int64_t payroll_tax(const TaxTable *t, int64_t gross) {
    int64_t tax  = 0;
    int64_t prev = 0;          /* lower edge of the current bracket */
    for (int i = 0; i < t->count && gross > prev; i++) {
        int64_t upper = t->brackets[i].upper_cents;
        /* The slice taxed in this bracket is bounded above by `gross`. */
        int64_t top   = gross < upper ? gross : upper;
        int64_t slice = top - prev;
        if (slice > 0)
            tax += slice * t->brackets[i].rate_bp / 10000;
        prev = upper;
    }
    return tax;
}

/*
 * Compute net (take-home) pay for one week: gross earnings minus progressive
 * tax. Returns cents. Convenience wrapper combining payroll_gross and
 * payroll_tax.
 */
int64_t payroll_net(const Employee *e, const TaxTable *t, int32_t hours) {
    int64_t gross = payroll_gross(e, hours);
    return gross - payroll_tax(t, gross);
}
