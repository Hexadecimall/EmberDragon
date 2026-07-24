// Levenshtein edit distance via a rolling 1-D DP table (insert/delete/substitute).
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

static int edit_distance(const std::string &a, const std::string &b) {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());
    std::vector<int> prev(m + 1), curr(m + 1);
    for (int j = 0; j <= m; ++j) prev[j] = j;
    for (int i = 1; i <= n; ++i) {
        curr[0] = i;
        for (int j = 1; j <= m; ++j) {
            if (a[i - 1] == b[j - 1]) {
                curr[j] = prev[j - 1];
            } else {
                int del = prev[j] + 1;
                int ins = curr[j - 1] + 1;
                int sub = prev[j - 1] + 1;
                curr[j] = std::min(del, std::min(ins, sub));
            }
        }
        std::swap(prev, curr);
    }
    return prev[m];
}

int main() {
    const char *pairs[][2] = {
        {"kitten", "sitting"},
        {"flaw", "lawn"},
        {"intention", "execution"},
        {"abc", "abc"},
    };
    int count = static_cast<int>(sizeof(pairs) / sizeof(pairs[0]));
    for (int i = 0; i < count; ++i) {
        std::string s1 = pairs[i][0];
        std::string s2 = pairs[i][1];
        std::cout << s1 << " -> " << s2 << " : "
                  << edit_distance(s1, s2) << "\n";
    }
    return 0;
}
