#include <iostream>
#include <string>

using namespace std;

void BankAccount(long a, long b, long c);
void deposit(long a, long b);
int withdraw(long a, long b);
long owner(long a);


class BankAccount {
    char _pad0[24];
    long value;
    long balance() const {
        return this->value;
    }
};

int main(int argc, char** argv) {
    int result;
    long v104;
    int v52;
    long v40;
    long v32;
    long v24;
    long v16;
    long v8;
    result = 0;
    std::string v80 = "Gideon";
    BankAccount(&v104, &v80, 10000);
    deposit(&v104, 2500);
    v52 = withdraw(&v104, 50000);
    if (!(v52 != 0)) {
        cout << "overdraw rejected\n";
    } else {
        withdraw(&v104, 7500);
        v40 = owner(&v104);
        v32 = cout << v40;
        v24 = v32 << " balance: ";
        v16 = v104.balance();
        v8 = v24 << v16;
        v8 << " cents\n";
        return 0;
    }
}

BankAccount::BankAccount(std::string const& b, long c) {
    long result;
    result = a;
    BankAccount(a, b, c);
    return;
}

struct Struct0 {
    char _pad0[24];
    long total;
};
void BankAccount::deposit(long b) {
    struct Struct0* obj;
    obj = a;
    if (b > 0) {
        obj->total += b;
        return;
    }
    return;
}

int BankAccount::withdraw(long b) {
    struct Struct0* obj;
    char v31;
    obj = a;
    if (!(b > 0 && b <= obj->total)) {
        v31 = 0 & 1 & 1;
        return (v31 & 1);
    }
    obj->total -= b;
    v31 = 1 & 1 & 1;
    return (v31 & 1);
}

long BankAccount::owner() const {
    long result;
    return a;
    long result;
    return a;
}

BankAccount::BankAccount(std::string const& b, long c) {
    struct Struct0* obj;
    obj = a;
    std::string a = b;
    obj->total = c;
    return;
    return a;
}

