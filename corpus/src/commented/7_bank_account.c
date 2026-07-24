/*
 * bank_account.c
 *
 * A minimal single-account banking module. It models a checking account
 * with an overdraft limit and a fixed-capacity transaction journal, and
 * provides deposit, withdrawal, transfer, and reconciliation operations
 * using integer "cents" arithmetic so no floating point is ever required.
 */

#include <stdint.h>
#include <string.h>

#define MAX_TRANSACTIONS 64

/* A single posted movement of money, in cents. Positive amounts are
 * credits (deposits) and negative amounts are debits (withdrawals). */
typedef struct {
    int64_t amount_cents;   /* signed delta applied to the balance      */
    uint32_t timestamp;     /* caller-supplied monotonic tick / epoch    */
} Transaction;

/* A checking account. The journal is a ring of at most MAX_TRANSACTIONS
 * entries; once full, the oldest entries are silently overwritten. The
 * invariant balance_cents >= -overdraft_limit_cents is enforced on debit. */
typedef struct {
    int64_t balance_cents;          /* current cleared balance            */
    int64_t overdraft_limit_cents;  /* max negative balance allowed (>=0) */
    Transaction journal[MAX_TRANSACTIONS];
    int count;                      /* number of journal slots written    */
    int head;                       /* next slot to write (ring index)    */
} Account;

/*
 * Initialize an account in place with a starting balance and overdraft cap.
 * Parameters:
 *   acct      - account to initialize (must be non-NULL).
 *   opening   - opening balance in cents.
 *   overdraft - overdraft limit in cents; negative values are clamped to 0.
 * The journal is emptied. No value is returned.
 */
void account_init(Account *acct, int64_t opening, int64_t overdraft) {
    acct->balance_cents = opening;
    /* An overdraft limit is a magnitude, so it can never be negative. */
    acct->overdraft_limit_cents = overdraft < 0 ? 0 : overdraft;
    acct->count = 0;
    acct->head = 0;
}

/*
 * Append a transaction to the ring journal, evicting the oldest if full.
 * Parameters:
 *   acct   - target account.
 *   amount - signed delta in cents that was applied to the balance.
 *   ts     - timestamp to record alongside the entry.
 * Returns nothing; updates head and count.
 */
static void journal_push(Account *acct, int64_t amount, uint32_t ts) {
    acct->journal[acct->head].amount_cents = amount;
    acct->journal[acct->head].timestamp = ts;
    acct->head = (acct->head + 1) % MAX_TRANSACTIONS;
    if (acct->count < MAX_TRANSACTIONS)
        acct->count++;
}

/*
 * Credit the account and record the movement.
 * Parameters: acct, amount (cents, must be > 0), ts (timestamp).
 * Returns 1 on success, 0 if amount is non-positive (rejected, no change).
 */
int account_deposit(Account *acct, int64_t amount, uint32_t ts) {
    if (amount <= 0)
        return 0;               /* deposits must move money in */
    acct->balance_cents += amount;
    journal_push(acct, amount, ts);
    return 1;
}

/*
 * Debit the account if doing so keeps it within the overdraft limit.
 * Parameters: acct, amount (cents, must be > 0), ts (timestamp).
 * Returns 1 on success, 0 if the amount is non-positive or the withdrawal
 * would push the balance below -overdraft_limit_cents (no change made).
 */
int account_withdraw(Account *acct, int64_t amount, uint32_t ts) {
    if (amount <= 0)
        return 0;
    /* Reject if the resulting balance would breach the overdraft floor. */
    if (acct->balance_cents - amount < -acct->overdraft_limit_cents)
        return 0;
    acct->balance_cents -= amount;
    journal_push(acct, -amount, ts);
    return 1;
}

/*
 * Move money from one account to another atomically.
 * Parameters: from, to (distinct accounts), amount (cents), ts (timestamp).
 * Returns 1 if both legs succeed; 0 otherwise. On a failed debit nothing is
 * transferred, so the two accounts are never left inconsistent.
 */
int account_transfer(Account *from, Account *to, int64_t amount, uint32_t ts) {
    if (!account_withdraw(from, amount, ts))
        return 0;               /* source could not cover it; abort early */
    /* The credit leg only fails on a non-positive amount, which the debit
     * leg already rejected, so this is effectively guaranteed to succeed. */
    return account_deposit(to, amount, ts);
}

/*
 * Sum every recorded journal entry and compare against the live balance.
 * Parameters: acct, opening (the opening balance used at init time).
 * Returns the discrepancy in cents: opening + sum(journal) - balance.
 * A return of 0 means the journal fully explains the current balance;
 * note that a wrapped (full) journal can legitimately produce a nonzero
 * result because evicted entries are no longer counted. O(count).
 */
int64_t account_reconcile(const Account *acct, int64_t opening) {
    int64_t running = opening;
    for (int i = 0; i < acct->count; i++)
        running += acct->journal[i].amount_cents;
    return running - acct->balance_cents;
}
