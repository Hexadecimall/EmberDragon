// Small class hierarchy via COMPOSITION: a Car owns an Engine and Wheels.
#include <iostream>
#include <string>

class Engine {
private:
    int horsepower_;
    bool running_;

public:
    explicit Engine(int hp) : horsepower_(hp), running_(false) {}

    void start() { running_ = true; }
    void stop()  { running_ = false; }
    bool running() const { return running_; }
    int  horsepower() const { return horsepower_; }
};

class Wheels {
private:
    int count_;
    int psi_;

public:
    Wheels(int count, int psi) : count_(count), psi_(psi) {}

    void inflate(int delta) { psi_ += delta; }
    int  psi() const { return psi_; }
    int  count() const { return count_; }
};

class Car {
private:
    std::string name_;
    Engine engine_;     // composition
    Wheels wheels_;     // composition

public:
    Car(const std::string &name, int hp)
        : name_(name), engine_(hp), wheels_(4, 30) {}

    void prepare() {
        engine_.start();
        wheels_.inflate(2);
    }

    void report() const {
        std::cout << name_ << ": "
                  << engine_.horsepower() << "hp, "
                  << (engine_.running() ? "on" : "off") << ", "
                  << wheels_.count() << " wheels @ "
                  << wheels_.psi() << " psi\n";
    }
};

int main() {
    Car car("Roadster", 240);
    car.report();
    car.prepare();
    car.report();
    return 0;
}
