#include <stdint.h>
#include <string.h>

#define MAX_ACCOUNTS 64

typedef struct {
    int32_t id;
    char owner[32];
    int64_t balance;
    int32_t active;
} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int32_t count;
} Bank;

void bank_init(Bank *bank) {
    bank->count = 0;
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        bank->accounts[i].id = -1;
        bank->accounts[i].balance = 0;
        bank->accounts[i].active = 0;
    }
}

Account *open_account(Bank *bank, int32_t id, const char *owner) {
    if (bank->count >= MAX_ACCOUNTS) {
        return 0;
    }
    Account *acct = &bank->accounts[bank->count];
    acct->id = id;
    acct->balance = 0;
    acct->active = 1;
    int i = 0;
    while (owner[i] != '\0' && i < 31) {
        acct->owner[i] = owner[i];
        i++;
    }
    acct->owner[i] = '\0';
    bank->count++;
    return acct;
}

Account *find_account(Bank *bank, int32_t id) {
    for (int i = 0; i < bank->count; i++) {
        if (bank->accounts[i].id == id && bank->accounts[i].active) {
            return &bank->accounts[i];
        }
    }
    return 0;
}

int deposit(Bank *bank, int32_t id, int64_t amount) {
    Account *acct = find_account(bank, id);
    if (acct == 0 || amount <= 0) {
        return -1;
    }
    acct->balance += amount;
    return 0;
}

int withdraw(Bank *bank, int32_t id, int64_t amount) {
    Account *acct = find_account(bank, id);
    if (acct == 0 || amount <= 0) {
        return -1;
    }
    if (acct->balance < amount) {
        return -2;
    }
    acct->balance -= amount;
    return 0;
}

int transfer(Bank *bank, int32_t from_id, int32_t to_id, int64_t amount) {
    if (withdraw(bank, from_id, amount) != 0) {
        return -1;
    }
    if (deposit(bank, to_id, amount) != 0) {
        deposit(bank, from_id, amount);
        return -2;
    }
    return 0;
}

int64_t total_assets(Bank *bank) {
    int64_t sum = 0;
    for (int i = 0; i < bank->count; i++) {
        if (bank->accounts[i].active) {
            sum += bank->accounts[i].balance;
        }
    }
    return sum;
}
