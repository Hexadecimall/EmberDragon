#include <stdint.h>

/* Levenshtein edit distance and a couple of derived string metrics. */

class StringMetrics {
public:
    static int length_of(const char *text) {
        int count = 0;
        while (text[count] != '\0') {
            count++;
        }
        return count;
    }

    static int minimum_of(int a, int b, int c) {
        int smallest = a;
        if (b < smallest) {
            smallest = b;
        }
        if (c < smallest) {
            smallest = c;
        }
        return smallest;
    }

    static int levenshtein(const char *source, const char *target) {
        int source_len = length_of(source);
        int target_len = length_of(target);
        if (source_len > 64 || target_len > 64) {
            return -1;
        }
        int previous[65];
        int current[65];
        for (int j = 0; j <= target_len; j++) {
            previous[j] = j;
        }
        for (int i = 1; i <= source_len; i++) {
            current[0] = i;
            for (int j = 1; j <= target_len; j++) {
                int substitution_cost = (source[i - 1] == target[j - 1]) ? 0 : 1;
                int deletion = previous[j] + 1;
                int insertion = current[j - 1] + 1;
                int substitution = previous[j - 1] + substitution_cost;
                current[j] = minimum_of(deletion, insertion, substitution);
            }
            for (int j = 0; j <= target_len; j++) {
                previous[j] = current[j];
            }
        }
        return previous[target_len];
    }

    static int hamming(const char *first, const char *second) {
        int distance = 0;
        int index = 0;
        while (first[index] != '\0' && second[index] != '\0') {
            if (first[index] != second[index]) {
                distance++;
            }
            index++;
        }
        if (first[index] != '\0' || second[index] != '\0') {
            return -1;
        }
        return distance;
    }

    static int similarity_percent(const char *source, const char *target) {
        int distance = levenshtein(source, target);
        if (distance < 0) {
            return -1;
        }
        int source_len = length_of(source);
        int target_len = length_of(target);
        int longer = source_len > target_len ? source_len : target_len;
        if (longer == 0) {
            return 100;
        }
        return (longer - distance) * 100 / longer;
    }
};

int compare_words(const char *a, const char *b) {
    return StringMetrics::levenshtein(a, b);
}
