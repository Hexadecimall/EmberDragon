/*
 * payroll.c
 *
 * A small payroll engine that computes gross and net pay for hourly
 * employees, applying overtime rules and progressive tax brackets. All
 * money is tracked in integer cents and all rates as integer basis points
 * (1 bp = 0.01%), so no floating-point arithmetic is needed anywhere.
 */

#include <stdint.h>
#include <string.h>

#define MAX_BRACKETS 8
#define STD_HOURS    40      /* weekly hours before overtime kicks in */

/* One employee's pay parameters. wage_cents is the regular hourly rate;
 * hours_worked is the total for the pay period. */
typedef struct {
    uint32_t id;
    int32_t  wage_cents;     /* regular pay per hour, in cents     */
    int32_t  hours_worked;   /* hours in the period (>= 0)         */
    int32_t  pretax_deduct;  /* per-period pretax deduction, cents */
} Employee;

/* One progressive tax bracket: income up to `ceiling_cents` (per period) is
 * taxed at `rate_bp` basis points. The final bracket should use a ceiling of
 * INT64_MAX to cover all remaining income. */
typedef struct {
    int64_t ceiling_cents;
    int32_t rate_bp;         /* basis points: 2500 == 25.00% */
} TaxBracket;

/* An ordered table of brackets, ascending by ceiling. */
typedef struct {
    TaxBracket brackets[MAX_BRACKETS];
    int count;
} TaxTable;

/*
 * Compute gross pay for a period, applying time-and-a-half overtime.
 * Parameters: e - employee whose wage and hours are read.
 * Returns gross pay in cents. Hours beyond STD_HOURS are paid at 1.5x the
 * regular rate; to stay in integer arithmetic the overtime portion is
 * computed as (ot_hours * wage * 3) / 2. O(1).
 */
int64_t payroll_gross(const Employee *e) {
    int32_t hours = e->hours_worked;
    if (hours <= STD_HOURS) {
        /* No overtime: straight-time pay for every hour worked. */
        return (int64_t)hours * e->wage_cents;
    }
    int32_t ot_hours = hours - STD_HOURS;
    int64_t regular = (int64_t)STD_HOURS * e->wage_cents;
    /* Multiply before dividing so the 1.5x factor loses no cents. */
    int64_t overtime = (int64_t)ot_hours * e->wage_cents * 3 / 2;
    return regular + overtime;
}

/*
 * Compute the income tax owed on a taxable amount using progressive brackets.
 * Parameters: table - ascending bracket table; taxable_cents - income to tax.
 * Returns total tax in cents. Each bracket taxes only the slice of income
 * that falls within it. Income above the last ceiling is taxed at the last
 * bracket's rate. Returns 0 for non-positive income. O(count).
 */
int64_t payroll_tax(const TaxTable *table, int64_t taxable_cents) {
    if (taxable_cents <= 0)
        return 0;
    int64_t tax = 0;
    int64_t lower = 0;          /* bottom of the current bracket slice */
    for (int i = 0; i < table->count; i++) {
        int64_t ceiling = table->brackets[i].ceiling_cents;
        /* The slice taxed here is the overlap of [lower, ceiling] with income. */
        int64_t top = taxable_cents < ceiling ? taxable_cents : ceiling;
        int64_t slice = top - lower;
        if (slice > 0) {
            /* basis points: divide by 10000 to convert bp back to a fraction */
            tax += slice * table->brackets[i].rate_bp / 10000;
        }
        if (taxable_cents <= ceiling)
            break;              /* all income accounted for */
        lower = ceiling;
    }
    return tax;
}

/*
 * Compute net (take-home) pay for an employee for the period.
 * Parameters: e - employee; table - progressive tax table.
 * Returns net pay in cents: gross, less the pretax deduction, less the tax
 * computed on the post-deduction taxable amount. The taxable base is floored
 * at zero so an oversized deduction never produces negative tax. O(brackets).
 */
int64_t payroll_net(const Employee *e, const TaxTable *table) {
    int64_t gross = payroll_gross(e);
    int64_t taxable = gross - e->pretax_deduct;
    if (taxable < 0)
        taxable = 0;            /* deductions cannot create negative income */
    int64_t tax = payroll_tax(table, taxable);
    return gross - e->pretax_deduct - tax;
}

/*
 * Run payroll for a roster and accumulate the totals.
 * Parameters:
 *   roster      - array of employees.
 *   n           - number of employees.
 *   table       - shared progressive tax table.
 *   out_gross   - if non-NULL, receives the summed gross pay in cents.
 *   out_tax     - if non-NULL, receives the summed tax withheld in cents.
 * Returns the summed net pay in cents across the whole roster. O(n*brackets).
 */
int64_t payroll_run(const Employee *roster, int n, const TaxTable *table,
                    int64_t *out_gross, int64_t *out_tax) {
    int64_t total_net = 0, total_gross = 0, total_tax = 0;
    for (int i = 0; i < n; i++) {
        int64_t gross = payroll_gross(&roster[i]);
        int64_t taxable = gross - roster[i].pretax_deduct;
        if (taxable < 0)
            taxable = 0;
        int64_t tax = payroll_tax(table, taxable);
        total_gross += gross;
        total_tax += tax;
        total_net += gross - roster[i].pretax_deduct - tax;
    }
    /* Only write the optional out-params when the caller asked for them. */
    if (out_gross) *out_gross = total_gross;
    if (out_tax)   *out_tax = total_tax;
    return total_net;
}
