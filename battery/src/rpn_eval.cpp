// Stack-based RPN (postfix) evaluator over whitespace-separated tokens.
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

static double apply(double a, double b, const std::string &op, bool &ok) {
    ok = true;
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") {
        if (b == 0.0) { ok = false; return 0.0; }
        return a / b;
    }
    ok = false;
    return 0.0;
}

static double eval_rpn(const std::string &expr, bool &ok) {
    std::vector<double> stack;
    std::istringstream iss(expr);
    std::string tok;
    ok = true;
    while (iss >> tok) {
        if (tok == "+" || tok == "-" || tok == "*" || tok == "/") {
            if (stack.size() < 2) { ok = false; return 0.0; }
            double b = stack.back(); stack.pop_back();
            double a = stack.back(); stack.pop_back();
            bool step_ok;
            double r = apply(a, b, tok, step_ok);
            if (!step_ok) { ok = false; return 0.0; }
            stack.push_back(r);
        } else {
            stack.push_back(std::atof(tok.c_str()));
        }
    }
    if (stack.size() != 1) { ok = false; return 0.0; }
    return stack.back();
}

int main() {
    const char *exprs[] = {
        "3 4 +",
        "5 1 2 + 4 * + 3 -",
        "2 3 4 * +",
        "10 2 /",
    };
    int n = static_cast<int>(sizeof(exprs) / sizeof(exprs[0]));
    for (int i = 0; i < n; ++i) {
        bool ok;
        double result = eval_rpn(exprs[i], ok);
        std::cout << "\"" << exprs[i] << "\" = ";
        if (ok) std::cout << result << "\n";
        else std::cout << "(error)\n";
    }
    return 0;
}
