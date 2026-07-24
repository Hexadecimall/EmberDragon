/*
 * subscription.c — A recurring-billing subscription manager with proration and
 * tiered plans.
 *
 * Tracks customer subscriptions to named plans, computes prorated charges when
 * a subscription starts partway through a billing cycle, and supports upgrading
 * or downgrading a plan with a credit/charge adjustment. All amounts are
 * integer cents and a billing cycle is a fixed number of days.
 */

#include <stdint.h>
#include <string.h>

#define MAX_PLANS         8
#define MAX_SUBSCRIPTIONS 64
#define PLAN_NAME_LEN     20
#define CYCLE_DAYS        30   /* days in one billing cycle */

/* A named pricing tier billed once per cycle. */
typedef struct {
    char    name[PLAN_NAME_LEN];  /* NUL-terminated plan name        */
    int64_t price_cents;          /* full-cycle price in cents       */
} Plan;

/* An active subscription linking a customer to a plan. */
typedef struct {
    int32_t customer_id;   /* owning customer                       */
    int     plan_index;    /* index into the plan catalog           */
    int     active;        /* 1 while live, 0 once cancelled        */
} Subscription;

/* The billing system: a plan catalog and a list of subscriptions. */
typedef struct {
    Plan         plans[MAX_PLANS];
    int          plan_count;
    Subscription subs[MAX_SUBSCRIPTIONS];
    int          sub_count;
} BillingSystem;

/* Initialize an empty billing system. */
void billing_init(BillingSystem *b) {
    b->plan_count = 0;
    b->sub_count  = 0;
}

/*
 * Define a new plan. Returns the plan's index, or -1 if the catalog is full.
 * The name is copied with truncation safety.
 */
int billing_add_plan(BillingSystem *b, const char *name, int64_t price_cents) {
    if (b->plan_count >= MAX_PLANS)
        return -1;
    Plan *p = &b->plans[b->plan_count];
    strncpy(p->name, name, PLAN_NAME_LEN - 1);
    p->name[PLAN_NAME_LEN - 1] = '\0';
    p->price_cents = price_cents;
    return b->plan_count++;
}

/*
 * Compute the prorated charge for joining a plan with `days_remaining` left in
 * the current cycle. The charge is price * days_remaining / CYCLE_DAYS, with
 * `days_remaining` clamped to [0, CYCLE_DAYS]. Returns cents.
 */
int64_t billing_prorate(const BillingSystem *b, int plan_index, int days_remaining) {
    if (plan_index < 0 || plan_index >= b->plan_count)
        return 0;
    if (days_remaining < 0)          days_remaining = 0;
    if (days_remaining > CYCLE_DAYS) days_remaining = CYCLE_DAYS;
    return b->plans[plan_index].price_cents * days_remaining / CYCLE_DAYS;
}

/*
 * Subscribe a customer to a plan. Returns the subscription index, or -1 if the
 * plan is invalid or the subscription table is full. A customer may hold
 * multiple subscriptions; deduplication is the caller's responsibility.
 */
int billing_subscribe(BillingSystem *b, int32_t customer_id, int plan_index) {
    if (plan_index < 0 || plan_index >= b->plan_count)
        return -1;
    if (b->sub_count >= MAX_SUBSCRIPTIONS)
        return -1;
    Subscription *s = &b->subs[b->sub_count];
    s->customer_id = customer_id;
    s->plan_index  = plan_index;
    s->active      = 1;
    return b->sub_count++;
}

/*
 * Change an existing subscription to a different plan and return the proration
 * adjustment for the `days_remaining` left in the cycle. A positive result is
 * an additional charge (upgrade); a negative result is a credit (downgrade).
 * Returns 0 and makes no change if either index is invalid or the subscription
 * is inactive.
 */
int64_t billing_change_plan(BillingSystem *b, int sub_index, int new_plan,
                            int days_remaining) {
    if (sub_index < 0 || sub_index >= b->sub_count)
        return 0;
    if (new_plan < 0 || new_plan >= b->plan_count)
        return 0;
    Subscription *s = &b->subs[sub_index];
    if (!s->active)
        return 0;

    /* The adjustment bills the price difference, prorated over what remains. */
    int64_t old_prorated = billing_prorate(b, s->plan_index, days_remaining);
    int64_t new_prorated = billing_prorate(b, new_plan,      days_remaining);
    s->plan_index = new_plan;
    return new_prorated - old_prorated;
}

/*
 * Cancel a subscription, marking it inactive. Returns 1 if it was active and is
 * now cancelled, 0 if the index is invalid or it was already inactive.
 */
int billing_cancel(BillingSystem *b, int sub_index) {
    if (sub_index < 0 || sub_index >= b->sub_count)
        return 0;
    if (!b->subs[sub_index].active)
        return 0;
    b->subs[sub_index].active = 0;
    return 1;
}

/*
 * Sum the full-cycle revenue from all currently active subscriptions. This is
 * the monthly recurring revenue (MRR) before proration. Returns cents.
 * O(sub_count).
 */
int64_t billing_recurring_revenue(const BillingSystem *b) {
    int64_t total = 0;
    for (int i = 0; i < b->sub_count; i++) {
        if (b->subs[i].active)
            total += b->plans[b->subs[i].plan_index].price_cents;
    }
    return total;
}
