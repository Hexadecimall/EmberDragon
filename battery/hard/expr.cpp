// Recursive-descent arithmetic expression parser + evaluator.
// Grammar (lowest to highest precedence):
//   expr   := term  (('+' | '-') term)*
//   term   := unary (('*' | '/' | '%') unary)*
//   unary  := ('+' | '-') unary | power
//   power  := atom ('^' power)?          // right-associative
//   atom   := number | '(' expr ')'
// No I/O input: a fixed table of expressions is evaluated deterministically.
#include <string>
#include <vector>
#include <cstdint>

namespace calc {

struct Parser {
    const std::string& s;
    size_t pos = 0;
    bool error = false;

    explicit Parser(const std::string& src) : s(src) {}

    void skip() { while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t')) ++pos; }
    char peek() { skip(); return pos < s.size() ? s[pos] : '\0'; }
    char get()  { skip(); return pos < s.size() ? s[pos++] : '\0'; }

    double parse() {
        double v = expr();
        skip();
        if (pos != s.size()) error = true;
        return v;
    }

    double expr() {
        double v = term();
        for (char c = peek(); c == '+' || c == '-'; c = peek()) {
            ++pos;
            double r = term();
            v = (c == '+') ? v + r : v - r;
        }
        return v;
    }

    double term() {
        double v = unary();
        for (char c = peek(); c == '*' || c == '/' || c == '%'; c = peek()) {
            ++pos;
            double r = unary();
            if (c == '*') v *= r;
            else if (c == '/') { if (r == 0.0) error = true; else v /= r; }
            else { // modulo on integral-valued doubles
                int64_t a = (int64_t)v, b = (int64_t)r;
                if (b == 0) error = true; else v = (double)(a % b);
            }
        }
        return v;
    }

    double unary() {
        char c = peek();
        if (c == '+') { ++pos; return unary(); }
        if (c == '-') { ++pos; return -unary(); }
        return power();
    }

    double power() {
        double base = atom();
        if (peek() == '^') {
            ++pos;
            double e = unary();        // right-assoc via recursion through unary->power
            double r = 1.0;
            int n = (int)e;
            bool neg = n < 0;
            if (neg) n = -n;
            for (int i = 0; i < n; ++i) r *= base;
            return neg ? 1.0 / r : r;
        }
        return base;
    }

    double atom() {
        char c = peek();
        if (c == '(') {
            ++pos;
            double v = expr();
            if (get() != ')') error = true;
            return v;
        }
        size_t start = pos;
        while (pos < s.size() && ((s[pos] >= '0' && s[pos] <= '9') || s[pos] == '.'))
            ++pos;
        if (pos == start) { error = true; return 0.0; }
        return std::stod(s.substr(start, pos - start));
    }
};

} // namespace calc

int main() {
    const char* exprs[] = {
        "1 + 2 * 3",
        "(1 + 2) * 3",
        "2 ^ 3 ^ 2",          // right-assoc => 2^9 = 512
        "-3 + 4 * -2",
        "10 % 3 + 1",
        "100 / (2 + 3) / 2",
    };
    long long checksum = 0;
    for (const char* e : exprs) {
        std::string src(e);
        calc::Parser p(src);
        double v = p.parse();
        checksum += (long long)(v * 1000.0);
        checksum = checksum * 31 + (p.error ? 1 : 0);
    }
    // Deterministic exit code derived from the accumulated checksum.
    return (int)(checksum & 0x7f);
}
