/*
 * bank_account.c — A minimal single-account ledger with overdraft protection.
 *
 * This module models one checking account that tracks a running balance in
 * integer cents (never floating point, to avoid rounding drift) and keeps an
 * append-only history of the most recent transactions. It enforces an
 * overdraft limit so a withdrawal can never push the balance below a
 * configurable floor.
 */

#include <stdint.h>
#include <string.h>

/* Maximum number of transactions retained in the rolling history buffer. */
#define MAX_HISTORY 16

/* Discriminates the two kinds of movements recorded in the ledger. */
typedef enum {
    TXN_DEPOSIT  = 0,
    TXN_WITHDRAW = 1
} TxnKind;

/* A single posted movement: signed is implied by `kind`, amount is positive. */
typedef struct {
    TxnKind kind;       /* deposit or withdrawal                          */
    int64_t amount;     /* magnitude in cents, always > 0                 */
    int64_t balance;    /* resulting balance after the movement posted    */
} Transaction;

/* The account aggregate: balance plus a fixed-capacity transaction ring. */
typedef struct {
    int64_t      balance;             /* current funds in cents            */
    int64_t      overdraft_limit;     /* most negative allowed balance     */
    Transaction  history[MAX_HISTORY];/* rolling window of recent activity */
    int          count;               /* number of valid history entries   */
} Account;

/*
 * Initialize an account in place with a starting balance and overdraft floor.
 * `overdraft_limit` is the lowest balance the account may reach and should be
 * passed as a non-positive number (e.g. -50000 for a $500 overdraft buffer).
 */
void account_init(Account *acct, int64_t opening_balance, int64_t overdraft_limit) {
    acct->balance         = opening_balance;
    acct->overdraft_limit = overdraft_limit;
    acct->count           = 0;
}

/*
 * Append a transaction record to the rolling history. When the buffer is full
 * the oldest entry is dropped by shifting everything down one slot, keeping the
 * window pinned to the MAX_HISTORY most recent movements. O(MAX_HISTORY).
 */
static void account_record(Account *acct, TxnKind kind, int64_t amount) {
    if (acct->count == MAX_HISTORY) {
        /* Evict the oldest record (index 0) to make room at the tail. */
        memmove(&acct->history[0], &acct->history[1],
                (MAX_HISTORY - 1) * sizeof(Transaction));
        acct->count = MAX_HISTORY - 1;
    }
    Transaction *t = &acct->history[acct->count++];
    t->kind    = kind;
    t->amount  = amount;
    t->balance = acct->balance;
}

/*
 * Credit funds to the account. Returns 1 on success, 0 if `amount` is not a
 * strictly positive value (a zero or negative deposit is rejected as invalid).
 */
int account_deposit(Account *acct, int64_t amount) {
    if (amount <= 0)
        return 0;
    acct->balance += amount;
    account_record(acct, TXN_DEPOSIT, amount);
    return 1;
}

/*
 * Debit funds from the account, honoring the overdraft floor. Returns 1 if the
 * withdrawal posted, 0 if the amount is non-positive or would breach the
 * overdraft limit (in which case the balance is left untouched).
 */
int account_withdraw(Account *acct, int64_t amount) {
    if (amount <= 0)
        return 0;
    /* Reject if the post-withdrawal balance would dip below the floor. */
    if (acct->balance - amount < acct->overdraft_limit)
        return 0;
    acct->balance -= amount;
    account_record(acct, TXN_WITHDRAW, amount);
    return 1;
}

/*
 * Sum the magnitudes of all retained deposits in the history window. This is a
 * derived statistic, not authoritative — older deposits evicted from the ring
 * are not counted. O(count).
 */
int64_t account_total_deposited(const Account *acct) {
    int64_t sum = 0;
    for (int i = 0; i < acct->count; i++) {
        if (acct->history[i].kind == TXN_DEPOSIT)
            sum += acct->history[i].amount;
    }
    return sum;
}

/*
 * Return a pointer to the most recent transaction, or NULL if no activity has
 * been recorded yet. The pointer aliases internal storage and is invalidated by
 * the next deposit/withdraw, so the caller must not retain it.
 */
const Transaction *account_last_txn(const Account *acct) {
    if (acct->count == 0)
        return 0;
    return &acct->history[acct->count - 1];
}
