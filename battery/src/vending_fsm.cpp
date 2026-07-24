// Tiny finite-state machine: a vending machine fed a fixed sequence of coin/button events.
#include <iostream>
#include <vector>
#include <string>

enum class State { Idle, Collecting, Dispensing };

struct Event { int coin; bool buy; }; // coin in cents (0 = none), buy = press button

int main() {
    const int PRICE = 75;
    State st = State::Idle;
    int credit = 0;
    int dispensed = 0;

    // Deterministic event tape: insert coins, press buy, repeat.
    std::vector<Event> tape = {
        {25, false}, {25, false}, {0, true},      // not enough -> rejected
        {25, false}, {0, true},                   // now 75 -> dispense, 0 change
        {50, false}, {50, false}, {0, true},      // 100 -> dispense, 25 change
    };

    for (const Event &e : tape) {
        if (e.coin > 0) {
            credit += e.coin;
            st = State::Collecting;
            std::cout << "inserted " << e.coin << "c, credit=" << credit << "\n";
        }
        if (e.buy) {
            if (st == State::Collecting && credit >= PRICE) {
                st = State::Dispensing;
                int change = credit - PRICE;
                dispensed++;
                std::cout << "DISPENSE #" << dispensed
                          << " change=" << change << "c\n";
                credit = 0;
                st = State::Idle;
            } else {
                std::cout << "rejected: need "
                          << (PRICE - credit) << "c more\n";
            }
        }
    }

    std::cout << "total dispensed=" << dispensed
              << " final credit=" << credit << "\n";
    return 0;
}
