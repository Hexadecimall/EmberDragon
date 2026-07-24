/*
 * ledger.c
 *
 * A double-entry accounting ledger. Each journal entry debits one account
 * and credits another by the same amount, preserving the fundamental
 * invariant that total debits equal total credits. The module computes
 * per-account balances and verifies that the books are in balance.
 */

#include <stdint.h>
#include <string.h>

#define MAX_ACCOUNTS 32
#define MAX_ENTRIES  256
#define NAME_LEN     24

/* Account classification. Debits increase assets and expenses but decrease
 * liabilities, equity, and revenue; this sign convention is applied when
 * deriving the normalized balance below. */
typedef enum {
    ACCT_ASSET = 0,
    ACCT_LIABILITY,
    ACCT_EQUITY,
    ACCT_REVENUE,
    ACCT_EXPENSE
} AccountType;

/* A ledger account identified by a small integer id (its array index). */
typedef struct {
    char name[NAME_LEN];
    AccountType type;
} LedgerAccount;

/* A balanced double-entry line: move `amount` cents from debit to credit. */
typedef struct {
    int debit_acct;     /* index of account to debit  */
    int credit_acct;    /* index of account to credit */
    int64_t amount;     /* magnitude in cents (> 0)   */
} JournalEntry;

/* The full set of books. */
typedef struct {
    LedgerAccount accounts[MAX_ACCOUNTS];
    int account_count;
    JournalEntry entries[MAX_ENTRIES];
    int entry_count;
} Ledger;

/*
 * Initialize an empty ledger with no accounts and no entries.
 * Parameters: lg - ledger to reset (non-NULL). Returns nothing.
 */
void ledger_init(Ledger *lg) {
    lg->account_count = 0;
    lg->entry_count = 0;
}

/*
 * Open a new account.
 * Parameters: lg, name (null-terminated, copied and truncated to fit),
 *             type (account classification).
 * Returns the new account's id (its index), or -1 if the ledger is full.
 */
int ledger_open_account(Ledger *lg, const char *name, AccountType type) {
    if (lg->account_count >= MAX_ACCOUNTS)
        return -1;
    int id = lg->account_count;
    strncpy(lg->accounts[id].name, name, NAME_LEN - 1);
    lg->accounts[id].name[NAME_LEN - 1] = '\0';
    lg->accounts[id].type = type;
    lg->account_count++;
    return id;
}

/*
 * Record a balanced journal entry debiting one account and crediting another.
 * Parameters: lg, debit (account id), credit (account id), amount (cents).
 * Returns 1 on success, 0 if amount <= 0, the ledger is full, the two
 * accounts are the same, or either id is out of range.
 */
int ledger_post(Ledger *lg, int debit, int credit, int64_t amount) {
    if (amount <= 0 || lg->entry_count >= MAX_ENTRIES)
        return 0;
    if (debit == credit)
        return 0;                       /* a self-entry moves nothing */
    if (debit < 0 || debit >= lg->account_count ||
        credit < 0 || credit >= lg->account_count)
        return 0;                       /* unknown account */
    JournalEntry *e = &lg->entries[lg->entry_count++];
    e->debit_acct = debit;
    e->credit_acct = credit;
    e->amount = amount;
    return 1;
}

/*
 * Compute the raw debit-minus-credit total touching a single account.
 * Parameters: lg, acct (account id).
 * Returns sum of amounts debited to the account minus amounts credited,
 * in cents. Positive means net-debited. O(entry_count).
 */
static int64_t ledger_raw_balance(const Ledger *lg, int acct) {
    int64_t bal = 0;
    for (int i = 0; i < lg->entry_count; i++) {
        if (lg->entries[i].debit_acct == acct)
            bal += lg->entries[i].amount;
        if (lg->entries[i].credit_acct == acct)
            bal -= lg->entries[i].amount;
    }
    return bal;
}

/*
 * Compute an account's normalized balance following accounting sign rules.
 * Parameters: lg, acct (account id).
 * Returns the natural-sign balance in cents: for assets and expenses this is
 * the raw debit-minus-credit total; for liabilities, equity, and revenue it
 * is negated so a normal credit balance reads positive. Returns 0 for an
 * out-of-range account id. O(entry_count).
 */
int64_t ledger_balance(const Ledger *lg, int acct) {
    if (acct < 0 || acct >= lg->account_count)
        return 0;
    int64_t raw = ledger_raw_balance(lg, acct);
    switch (lg->accounts[acct].type) {
        case ACCT_ASSET:
        case ACCT_EXPENSE:
            return raw;                 /* debit-normal accounts */
        default:
            return -raw;                /* credit-normal accounts */
    }
}

/*
 * Verify the books balance: every entry contributes equal debit and credit,
 * so the sum of all raw account balances must be exactly zero.
 * Parameters: lg - ledger to check.
 * Returns 1 if the ledger is in balance, 0 otherwise. O(accounts * entries).
 */
int ledger_is_balanced(const Ledger *lg) {
    int64_t sum = 0;
    for (int a = 0; a < lg->account_count; a++)
        sum += ledger_raw_balance(lg, a);
    /* If debits == credits everywhere, the signed totals must cancel out. */
    return sum == 0;
}
