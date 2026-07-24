#include <iostream>
#include <string>

// Counting pass: tally letter frequencies into a 26-bucket histogram,
// then render each bucket as a row of asterisks.
int main() {
    const std::string text =
        "the quick brown fox jumps over the lazy dog the end";
    int counts[26] = {0};
    for (char ch : text) {
        if (ch >= 'a' && ch <= 'z')
            counts[ch - 'a']++;
    }
    int total = 0, peak = 0;
    for (int i = 0; i < 26; i++) {
        total += counts[i];
        if (counts[i] > peak) peak = counts[i];
    }
    for (int i = 0; i < 26; i++) {
        if (counts[i] == 0) continue;
        std::cout << char('a' + i) << " (" << counts[i] << ") ";
        for (int b = 0; b < counts[i]; b++) std::cout << '*';
        std::cout << "\n";
    }
    std::cout << "total letters: " << total << ", peak: " << peak << "\n";
    return 0;
}
