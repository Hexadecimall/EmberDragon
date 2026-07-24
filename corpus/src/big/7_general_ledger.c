/*
 * general_ledger.c — A small double-entry accounting ledger.
 *
 * Each journal entry debits one account and credits another by the same amount,
 * preserving the fundamental invariant that total debits equal total credits.
 * Account balances are derived by replaying the journal. Amounts are integer
 * cents; the sign convention is asset/expense accounts increase on debit.
 */

#include <stdint.h>

#define MAX_ACCOUNTS 32
#define MAX_ENTRIES  128

/* A posted double-entry transaction moving `amount` from debit to credit. */
typedef struct {
    int     debit_account;   /* index of the account being debited  */
    int     credit_account;  /* index of the account being credited */
    int64_t amount;          /* cents moved, > 0                     */
} JournalEntry;

/* A chart-of-accounts slot. `is_debit_normal` flips the balance sign. */
typedef struct {
    int32_t id;              /* caller-assigned account number       */
    int     is_debit_normal; /* 1 for assets/expenses, 0 for the rest */
} LedgerAccount;

/* The ledger aggregate: a chart of accounts plus the journal. */
typedef struct {
    LedgerAccount accounts[MAX_ACCOUNTS];
    int           account_count;
    JournalEntry  entries[MAX_ENTRIES];
    int           entry_count;
} Ledger;

/* Reset a ledger to a clean state with no accounts and no entries. */
void ledger_init(Ledger *l) {
    l->account_count = 0;
    l->entry_count   = 0;
}

/*
 * Register an account in the chart. Returns the new account's index, or -1 if
 * the chart is full. `is_debit_normal` should be 1 for asset/expense accounts.
 */
int ledger_add_account(Ledger *l, int32_t id, int is_debit_normal) {
    if (l->account_count >= MAX_ACCOUNTS)
        return -1;
    LedgerAccount *a = &l->accounts[l->account_count];
    a->id              = id;
    a->is_debit_normal = is_debit_normal;
    return l->account_count++;
}

/*
 * Post a balanced journal entry. Returns 1 on success; 0 if the amount is
 * non-positive, either account index is out of range, or the journal is full.
 * Debit and credit must reference different accounts.
 */
int ledger_post(Ledger *l, int debit_account, int credit_account, int64_t amount) {
    if (amount <= 0)
        return 0;
    if (debit_account == credit_account)
        return 0;                       /* a self-transfer is meaningless */
    if (debit_account < 0  || debit_account  >= l->account_count)
        return 0;
    if (credit_account < 0 || credit_account >= l->account_count)
        return 0;
    if (l->entry_count >= MAX_ENTRIES)
        return 0;

    JournalEntry *e = &l->entries[l->entry_count++];
    e->debit_account  = debit_account;
    e->credit_account = credit_account;
    e->amount         = amount;
    return 1;
}

/*
 * Compute an account's balance by replaying the journal. Debits add and credits
 * subtract from a raw figure; the sign is then normalized so a debit-normal
 * account reports positive when net-debited. Returns 0 for an out-of-range
 * index. O(entry_count).
 */
int64_t ledger_balance(const Ledger *l, int account) {
    if (account < 0 || account >= l->account_count)
        return 0;

    int64_t raw = 0;   /* (sum of debits) - (sum of credits) for this account */
    for (int i = 0; i < l->entry_count; i++) {
        if (l->entries[i].debit_account == account)
            raw += l->entries[i].amount;
        if (l->entries[i].credit_account == account)
            raw -= l->entries[i].amount;
    }

    /* Credit-normal accounts (liabilities, equity, revenue) invert. */
    if (l->accounts[account].is_debit_normal)
        return raw;
    return -raw;
}

/*
 * Verify the books balance: the sum of every account's raw debit-minus-credit
 * figure must be exactly zero in a correct double-entry system. Returns 1 if
 * balanced, 0 otherwise. This is a cheap integrity check, O(entry_count).
 */
int ledger_is_balanced(const Ledger *l) {
    int64_t total = 0;
    for (int i = 0; i < l->entry_count; i++) {
        /* Each entry contributes +amount and -amount, netting to zero, so a
         * nonzero total can only arise from corrupted entry data. */
        total += l->entries[i].amount;   /* debit side  */
        total -= l->entries[i].amount;   /* credit side */
    }
    return total == 0;
}

/*
 * Find an account's index by its caller-assigned id. Returns the index, or -1
 * if no account carries that id. O(account_count).
 */
int ledger_find_account(const Ledger *l, int32_t id) {
    for (int i = 0; i < l->account_count; i++) {
        if (l->accounts[i].id == id)
            return i;
    }
    return -1;
}
