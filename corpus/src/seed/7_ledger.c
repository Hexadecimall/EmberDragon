#include <stdint.h>
#include <stdlib.h>

typedef enum {
    ENTRY_DEBIT = 0,
    ENTRY_CREDIT = 1
} EntryType;

typedef struct LedgerEntry {
    int32_t txn_id;
    int32_t account_code;
    EntryType type;
    int64_t amount_cents;
    struct LedgerEntry *next;
} LedgerEntry;

typedef struct {
    LedgerEntry *head;
    LedgerEntry *tail;
    int32_t size;
} Ledger;

void ledger_init(Ledger *ledger) {
    ledger->head = 0;
    ledger->tail = 0;
    ledger->size = 0;
}

LedgerEntry *post_entry(Ledger *ledger, int32_t txn_id, int32_t account, EntryType type, int64_t amount) {
    LedgerEntry *entry = (LedgerEntry *)malloc(sizeof(LedgerEntry));
    if (entry == 0) {
        return 0;
    }
    entry->txn_id = txn_id;
    entry->account_code = account;
    entry->type = type;
    entry->amount_cents = amount;
    entry->next = 0;
    if (ledger->tail == 0) {
        ledger->head = entry;
        ledger->tail = entry;
    } else {
        ledger->tail->next = entry;
        ledger->tail = entry;
    }
    ledger->size++;
    return entry;
}

int64_t account_balance(Ledger *ledger, int32_t account) {
    int64_t balance = 0;
    LedgerEntry *cur = ledger->head;
    while (cur != 0) {
        if (cur->account_code == account) {
            if (cur->type == ENTRY_DEBIT) {
                balance += cur->amount_cents;
            } else {
                balance -= cur->amount_cents;
            }
        }
        cur = cur->next;
    }
    return balance;
}

int is_balanced(Ledger *ledger) {
    int64_t debits = 0;
    int64_t credits = 0;
    LedgerEntry *cur = ledger->head;
    while (cur != 0) {
        if (cur->type == ENTRY_DEBIT) {
            debits += cur->amount_cents;
        } else {
            credits += cur->amount_cents;
        }
        cur = cur->next;
    }
    return debits == credits ? 1 : 0;
}

int count_txn_entries(Ledger *ledger, int32_t txn_id) {
    int n = 0;
    LedgerEntry *cur = ledger->head;
    while (cur != 0) {
        if (cur->txn_id == txn_id) {
            n++;
        }
        cur = cur->next;
    }
    return n;
}

void ledger_free(Ledger *ledger) {
    LedgerEntry *cur = ledger->head;
    while (cur != 0) {
        LedgerEntry *next = cur->next;
        free(cur);
        cur = next;
    }
    ledger->head = 0;
    ledger->tail = 0;
    ledger->size = 0;
}
