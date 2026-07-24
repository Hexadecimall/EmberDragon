// Tokenizer + word-frequency counter over a fixed paragraph, sorted by count desc.
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

struct Entry {
    std::string word;
    int count;
};

int main() {
    const std::string text =
        "the cat sat on the mat the cat ran the dog sat on the log "
        "and the dog and the cat sat together on the warm mat";

    std::vector<Entry> table;
    std::string token;
    for (size_t i = 0; i <= text.size(); ++i) {
        char c = (i < text.size()) ? text[i] : ' ';
        if (std::isalpha(static_cast<unsigned char>(c))) {
            token += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (!token.empty()) {
            bool found = false;
            for (auto &e : table) {
                if (e.word == token) { e.count++; found = true; break; }
            }
            if (!found) table.push_back({token, 1});
            token.clear();
        }
    }

    std::sort(table.begin(), table.end(), [](const Entry &a, const Entry &b) {
        if (a.count != b.count) return a.count > b.count;
        return a.word < b.word;
    });

    for (const auto &e : table) {
        std::cout << e.word << ": " << e.count << "\n";
    }
    return 0;
}
