#include <stdint.h>

/* Naive and Knuth-Morris-Pratt substring search over byte strings. */

static int string_length(const char *text) {
    int length = 0;
    while (text[length] != '\0') {
        length++;
    }
    return length;
}

int naive_search(const char *haystack, const char *needle) {
    int haystack_len = string_length(haystack);
    int needle_len = string_length(needle);
    if (needle_len == 0) {
        return 0;
    }
    for (int start = 0; start + needle_len <= haystack_len; start++) {
        int matched = 1;
        for (int offset = 0; offset < needle_len; offset++) {
            if (haystack[start + offset] != needle[offset]) {
                matched = 0;
                break;
            }
        }
        if (matched) {
            return start;
        }
    }
    return -1;
}

static void build_failure_table(const char *pattern, int pattern_len, int *failure) {
    failure[0] = 0;
    int prefix_len = 0;
    for (int index = 1; index < pattern_len; index++) {
        while (prefix_len > 0 && pattern[index] != pattern[prefix_len]) {
            prefix_len = failure[prefix_len - 1];
        }
        if (pattern[index] == pattern[prefix_len]) {
            prefix_len++;
        }
        failure[index] = prefix_len;
    }
}

int kmp_search(const char *haystack, const char *needle) {
    int haystack_len = string_length(haystack);
    int needle_len = string_length(needle);
    if (needle_len == 0) {
        return 0;
    }
    if (needle_len > 256) {
        return -1;
    }
    int failure[256];
    build_failure_table(needle, needle_len, failure);

    int matched_count = 0;
    for (int scan = 0; scan < haystack_len; scan++) {
        while (matched_count > 0 && haystack[scan] != needle[matched_count]) {
            matched_count = failure[matched_count - 1];
        }
        if (haystack[scan] == needle[matched_count]) {
            matched_count++;
        }
        if (matched_count == needle_len) {
            return scan - needle_len + 1;
        }
    }
    return -1;
}

int count_occurrences(const char *haystack, const char *needle) {
    int needle_len = string_length(needle);
    if (needle_len == 0) {
        return 0;
    }
    int count = 0;
    int position = 0;
    const char *cursor = haystack;
    while (1) {
        int found = kmp_search(cursor, needle);
        if (found < 0) {
            break;
        }
        count++;
        position += found + 1;
        cursor = cursor + found + 1;
    }
    return count;
}
