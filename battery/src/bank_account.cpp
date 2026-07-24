// C++ class with private fields + methods: BankAccount deposit/withdraw.
#include <iostream>
#include <string>

class BankAccount {
private:
    std::string owner_;
    long balance_cents_;

public:
    BankAccount(const std::string &owner, long opening_cents)
        : owner_(owner), balance_cents_(opening_cents) {}

    void deposit(long cents) {
        if (cents > 0)
            balance_cents_ += cents;
    }

    bool withdraw(long cents) {
        if (cents <= 0 || cents > balance_cents_)
            return false;
        balance_cents_ -= cents;
        return true;
    }

    long balance() const { return balance_cents_; }
    const std::string &owner() const { return owner_; }
};

int main() {
    BankAccount acct("Gideon", 10000);
    acct.deposit(2500);

    if (!acct.withdraw(50000))
        std::cout << "overdraw rejected\n";

    acct.withdraw(7500);

    std::cout << acct.owner() << " balance: "
              << acct.balance() << " cents\n";
    return 0;
}
