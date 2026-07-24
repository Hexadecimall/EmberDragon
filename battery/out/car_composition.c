#include <string>

using namespace std;

void Car(long a, long b, int c);
void report(long a);
void prepare(long a);
void Engine(long a, int b);
void Wheels(long a, int b, int c);


class Engine {
    int value;
    char flag;
    Engine(int a) {
        this->value = a;
        this->flag = 0;
        return;
    }
    long horsepower() const {
        return this->value;
    }
    void running() const {
        return;
    }
    long count() const {
        return this->value;
    }
    void start() {
        this->flag = 1;
        return;
    }
};

class Wheels {
    int value;
    int total;
    Wheels(int a, int b) {
        this->value = a;
        this->total = b;
        return;
    }
    long psi() const {
        return this->total;
    }
    long inflate(int a) {
        long obj2;
        this->total += a;
        return obj2;
    }
};

int main(int argc, char** argv) {
    int result;
    char buf[64];
    result = 0;
    std::string v24 = "Roadster";
    Car(buf, &v24, 240);
    report(buf);
    prepare(buf);
    report(buf);
    return 0;
}

Car::Car(std::string const& b, int c) {
    long result;
    result = a;
    Car(a, b, c);
    return;
}

void Car::report() const {
    long v0;
    long v8;
    long cond;
    long v16;
    long v32;
    v0 = operator_lsh": ";
    v0 << horsepower(a + 24);
    v8 = operator_lsh"hp, ";
    running(a + 24);
    v8 << (cond ? "on" : "off");
    v16 = operator_lsh", ";
    v16 << count(a + 32);
    v32 = operator_lsh" wheels @ ";
    v32 << psi(a + 32);
    operator_lsh" psi\n";
    return;
}

void Car::prepare() {
    start(a + 24);
    inflate(a + 32, 2);
    return;
    return a;
}

Car::Car(std::string const& b, int c) {
    long result;
    result = a;
    std::string a = b;
    Engine(result + 24, c);
    Wheels(result + 32, 4, 30);
    return;
}

Engine::Engine(int b) {
    long result;
    result = a;
    Engine(a, b);
    return;
}

Wheels::Wheels(int b, int c) {
    long result;
    result = a;
    Wheels(a, b, c);
    return;
    long result;
    return a;
}

