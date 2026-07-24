// A tiny JSON-ish tokenizer. Splits a JSON document into a token stream:
// braces, brackets, colons, commas, strings (with escapes incl. \uXXXX),
// numbers, and the literals true/false/null.
// No input I/O: tokenizes a fixed embedded document deterministically.
#include <string>
#include <vector>
#include <cstdint>

namespace jtok {

enum class Kind {
    LBrace, RBrace, LBracket, RBracket, Colon, Comma,
    String, Number, True, False, Null, Error, End
};

struct Token { Kind kind; std::string text; };   // text = decoded value for strings

struct Lexer {
    const std::string& s;
    size_t i = 0;
    explicit Lexer(const std::string& src) : s(src) {}

    static bool isDigit(char c) { return c >= '0' && c <= '9'; }
    void ws() {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
            ++i;
    }

    Token string() {
        std::string out;
        ++i; // opening quote
        while (i < s.size() && s[i] != '"') {
            char c = s[i++];
            if (c != '\\') { out.push_back(c); continue; }
            if (i >= s.size()) return {Kind::Error, ""};
            char e = s[i++];
            switch (e) {
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case 'r': out.push_back('\r'); break;
                case '"': case '\\': case '/': out.push_back(e); break;
                case 'u': {                // \uXXXX -> low byte (deterministic)
                    if (i + 4 > s.size()) return {Kind::Error, ""};
                    int code = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = s[i++];
                        int d = (h >= '0' && h <= '9') ? h - '0'
                              : (h >= 'a' && h <= 'f') ? h - 'a' + 10
                              : (h >= 'A' && h <= 'F') ? h - 'A' + 10 : -1;
                        if (d < 0) return {Kind::Error, ""};
                        code = code * 16 + d;
                    }
                    out.push_back((char)(code & 0xff));
                    break;
                }
                default: return {Kind::Error, ""};
            }
        }
        if (i >= s.size()) return {Kind::Error, ""};
        ++i; // closing quote
        return {Kind::String, out};
    }

    Token number() {
        size_t start = i;
        if (s[i] == '-') ++i;
        while (i < s.size() && isDigit(s[i])) ++i;
        if (i < s.size() && s[i] == '.') { ++i; while (i < s.size() && isDigit(s[i])) ++i; }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {   // optional exponent
            ++i;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
            while (i < s.size() && isDigit(s[i])) ++i;
        }
        return {Kind::Number, s.substr(start, i - start)};
    }

    Token literal(const char* lit, Kind k) {
        size_t n = 0; while (lit[n]) ++n;
        if (s.compare(i, n, lit) == 0) { i += n; return {k, std::string(lit)}; }
        ++i; return {Kind::Error, ""};
    }

    Token next() {
        ws();
        if (i >= s.size()) return {Kind::End, ""};
        char c = s[i];
        switch (c) {
            case '{': ++i; return {Kind::LBrace, "{"};   case '}': ++i; return {Kind::RBrace, "}"};
            case '[': ++i; return {Kind::LBracket, "["}; case ']': ++i; return {Kind::RBracket, "]"};
            case ':': ++i; return {Kind::Colon, ":"};    case ',': ++i; return {Kind::Comma, ","};
            case '"': return string();
            case 't': return literal("true", Kind::True);
            case 'f': return literal("false", Kind::False);
            case 'n': return literal("null", Kind::Null);
            default:
                if (c == '-' || isDigit(c)) return number();
                ++i; return {Kind::Error, ""};
        }
    }
};
} // namespace jtok

int main() {
    const std::string doc =
        "{ \"name\": \"a\\u0062c\", \"vals\": [1, -2.5, 3e2, true, false, null],"
        "  \"nested\": { \"ok\\t\": \"line\\nbreak\" } }";
    jtok::Lexer lex(doc);
    uint64_t hash = 1469598103934665603ull;            // FNV-1a offset basis
    int counts[(int)jtok::Kind::End + 1] = {0};
    for (;;) {
        jtok::Token t = lex.next();
        counts[(int)t.kind]++;
        for (char ch : t.text) { hash ^= (uint8_t)ch; hash *= 1099511628211ull; }
        hash ^= (uint64_t)t.kind + 7u;
        hash *= 1099511628211ull;
        if (t.kind == jtok::Kind::End || t.kind == jtok::Kind::Error) break;
    }
    int strings = counts[(int)jtok::Kind::String], numbers = counts[(int)jtok::Kind::Number];
    return (int)((hash ^ (uint64_t)(strings * 100 + numbers)) & 0x7f);
}
