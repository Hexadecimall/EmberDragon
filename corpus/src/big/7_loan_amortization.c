/*
 * loan_amortization.c — Fixed-rate installment loan amortization in integer
 * cents.
 *
 * Given a principal, a monthly interest rate in basis points, and a fixed
 * monthly payment, this module steps a loan forward month by month, splitting
 * each payment into interest and principal and tracking the declining balance.
 * It also reports the total interest paid over the life of the loan. No
 * floating point is used anywhere; interest is computed with rounded integer
 * basis-point arithmetic.
 */

#include <stdint.h>

/* The state of a loan as it amortizes. */
typedef struct {
    int64_t principal_cents;   /* remaining balance owed, in cents          */
    int32_t monthly_rate_bp;   /* monthly interest rate, basis points       */
    int64_t payment_cents;     /* scheduled payment each month, in cents    */
    int32_t months_elapsed;    /* number of payments applied so far         */
    int64_t total_interest;    /* cumulative interest paid, in cents        */
} Loan;

/* The decomposition of a single month's payment. */
typedef struct {
    int64_t interest;   /* portion of the payment that covered interest */
    int64_t principal;  /* portion that reduced the balance             */
    int64_t balance;    /* remaining balance after this payment         */
} PaymentBreakdown;

/*
 * Initialize a loan. `monthly_rate_bp` is the periodic (monthly) rate in basis
 * points — e.g. an annual 6% nominal rate is 50 bp per month. The payment must
 * be set high enough to exceed the first month's interest, or the balance will
 * never decline (see loan_will_amortize).
 */
void loan_init(Loan *loan, int64_t principal_cents, int32_t monthly_rate_bp,
               int64_t payment_cents) {
    loan->principal_cents = principal_cents;
    loan->monthly_rate_bp = monthly_rate_bp;
    loan->payment_cents   = payment_cents;
    loan->months_elapsed  = 0;
    loan->total_interest  = 0;
}

/*
 * Compute this period's interest on the outstanding balance, rounded to the
 * nearest cent. Rounding adds half the divisor before the integer division so
 * that, for instance, 49.5 cents rounds up to 50 rather than truncating to 49.
 */
static int64_t accrue_interest(const Loan *loan) {
    int64_t numer = loan->principal_cents * loan->monthly_rate_bp;
    return (numer + 5000) / 10000;   /* +5000 == +0.5 * 10000 for rounding */
}

/*
 * Report whether the configured payment is large enough to amortize the loan.
 * If the scheduled payment does not exceed the first month's interest, the
 * principal can never shrink. Returns 1 if the loan will pay down, else 0.
 */
int loan_will_amortize(const Loan *loan) {
    return loan->payment_cents > accrue_interest(loan);
}

/*
 * Apply one month's payment and fill `out` with the interest/principal split
 * and resulting balance. The final payment is capped so the borrower never
 * overpays past a zero balance. Returns 1 if a payment was applied, or 0 if the
 * loan is already paid off (balance == 0).
 */
int loan_step(Loan *loan, PaymentBreakdown *out) {
    if (loan->principal_cents <= 0)
        return 0;                        /* nothing left to pay */

    int64_t interest = accrue_interest(loan);
    int64_t payment  = loan->payment_cents;

    /* On the last installment, owed may be less than the scheduled payment. */
    int64_t owed = loan->principal_cents + interest;
    if (payment > owed)
        payment = owed;                  /* clamp so balance lands exactly 0 */

    int64_t principal_paid = payment - interest;
    loan->principal_cents -= principal_paid;
    loan->total_interest  += interest;
    loan->months_elapsed  += 1;

    out->interest  = interest;
    out->principal = principal_paid;
    out->balance   = loan->principal_cents;
    return 1;
}

/*
 * Run the loan to completion, applying payments until the balance reaches zero.
 * Returns the number of months required, or -1 if the payment is too small to
 * ever amortize (which would otherwise loop forever).
 */
int loan_payoff_months(Loan *loan) {
    if (!loan_will_amortize(loan))
        return -1;                       /* guard against an infinite loop */
    PaymentBreakdown bd;
    while (loan_step(loan, &bd))
        ;                                /* iterate until fully paid off */
    return loan->months_elapsed;
}
