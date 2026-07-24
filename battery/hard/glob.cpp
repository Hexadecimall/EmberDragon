// Glob / wildcard matcher with '*' (any run, incl. empty), '?' (one char),
// and '[...]' character classes (with ranges and leading '!' negation).
// Two engines cross-checked against each other: (1) recursive backtracking,
// (2) iterative two-pointer (O(n*m) worst, O(n) typical for '*').
// No input I/O: a fixed test table drives both engines deterministically.
#include <string>
#include <vector>
#include <cstdint>

namespace glob {

// Match a single character against a '[...]' class starting at pat[pp].
// On success advances pp past the closing ']'. Returns whether c matched.
static bool matchClass(const std::string& pat, size_t& pp, char c) {
    bool neg = false;
    ++pp; // skip '['
    if (pp < pat.size() && pat[pp] == '!') { neg = true; ++pp; }
    bool matched = false;
    bool first = true;
    while (pp < pat.size() && (pat[pp] != ']' || first)) {
        first = false;
        char lo = pat[pp];
        if (pp + 2 < pat.size() && pat[pp + 1] == '-' && pat[pp + 2] != ']') {
            char hi = pat[pp + 2];
            if (c >= lo && c <= hi) matched = true;
            pp += 3;
        } else {
            if (c == lo) matched = true;
            ++pp;
        }
    }
    if (pp < pat.size() && pat[pp] == ']') ++pp; // consume ']'
    return matched != neg;
}

// Engine 1: classic recursive backtracking.
static bool recMatch(const std::string& pat, size_t pp,
                     const std::string& str, size_t sp) {
    while (pp < pat.size()) {
        char pc = pat[pp];
        if (pc == '*') {
            while (pp < pat.size() && pat[pp] == '*') ++pp; // collapse runs
            if (pp == pat.size()) return true;
            for (size_t k = sp; k <= str.size(); ++k)
                if (recMatch(pat, pp, str, k)) return true;
            return false;
        } else if (pc == '?') {
            if (sp >= str.size()) return false;
            ++pp; ++sp;
        } else if (pc == '[') {
            if (sp >= str.size()) return false;
            size_t save = pp;
            if (!matchClass(pat, save, str[sp])) return false;
            pp = save; ++sp;
        } else {
            if (sp >= str.size() || str[sp] != pc) return false;
            ++pp; ++sp;
        }
    }
    return sp == str.size();
}

// Engine 2: iterative two-pointer with star backtrack points.
static bool iterMatch(const std::string& pat, const std::string& str) {
    size_t pp = 0, sp = 0;
    size_t starP = std::string::npos, starS = 0;
    while (sp < str.size()) {
        if (pp < pat.size() && pat[pp] == '*') {
            while (pp < pat.size() && pat[pp] == '*') ++pp;
            starP = pp; starS = sp;
            if (pp == pat.size()) return true;
        } else if (pp < pat.size() && pat[pp] == '?') {
            ++pp; ++sp;
        } else if (pp < pat.size() && pat[pp] == '[') {
            size_t save = pp;
            if (matchClass(pat, save, str[sp])) { pp = save; ++sp; }
            else if (starP != std::string::npos) { pp = starP; sp = ++starS; }
            else return false;
        } else if (pp < pat.size() && pat[pp] == str[sp]) {
            ++pp; ++sp;
        } else if (starP != std::string::npos) {
            pp = starP; sp = ++starS;
        } else {
            return false;
        }
    }
    while (pp < pat.size() && pat[pp] == '*') ++pp;
    return pp == pat.size();
}

} // namespace glob

int main() {
    struct Case { const char* pat; const char* str; };
    const Case cases[] = {
        {"*.txt",        "report.txt"},
        {"*.txt",        "report.csv"},
        {"a?c",          "abc"},
        {"a?c",          "ac"},
        {"[a-z]*[0-9]",  "hello9"},
        {"[!0-9]*",      "x123"},
        {"f**o",         "fooooo"},
        {"*",            ""},
        {"???",          "abcd"},
        {"a[bc]d",       "abd"},
        {"a[bc]d",       "aed"},
        {"prefix*suffix","prefix_middle_suffix"},
    };
    int agree = 0, total = 0, trueHits = 0;
    for (const Case& c : cases) {
        std::string pat(c.pat), str(c.str);
        bool r = glob::recMatch(pat, 0, str, 0);
        bool it = glob::iterMatch(pat, str);
        if (r == it) ++agree;
        if (r) ++trueHits;
        ++total;
    }
    // Encode results so the optimizer cannot fold everything away.
    return (int)(((agree == total) ? 0 : 64) + (trueHits & 0x3f));
}
