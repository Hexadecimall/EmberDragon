/*
 * word_frequency.cpp — A word-frequency counter built on a chained hash map.
 *
 * Splits text into lowercase alphabetic words and tallies how often each one
 * appears, using a hand-rolled separate-chaining hash table so the data
 * structure is fully visible rather than hidden behind the standard library.
 * Exposes lookup of a single word's count and discovery of the most frequent.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>

/*
 * One node in a hash bucket's linked list. Holds an owned copy of the word,
 * its running count, and a pointer to the next collision in the same bucket.
 */
struct WordNode {
    char *word;
    int count;
    WordNode *next;
};

/*
 * A separate-chaining hash table mapping words to counts. `buckets` is an
 * array of `bucket_count` list heads; `unique_words` tracks the number of
 * distinct keys for quick reporting.
 */
struct WordCounter {
    WordNode **buckets;
    int bucket_count;
    int unique_words;
};

/*
 * Hash a NUL-terminated word with the djb2 algorithm.
 *
 * Returns a 32-bit hash. djb2 is compact and spreads short lowercase words
 * well enough for a frequency counter. Complexity: O(length).
 */
static uint32_t hash_word(const char *word) {
    uint32_t hash = 5381;
    for (const unsigned char *p = (const unsigned char *)word; *p; p++) {
        hash = hash * 33u + *p;  /* Classic "hash * 33 + c" step. */
    }
    return hash;
}

/*
 * Duplicate a NUL-terminated string onto the heap.
 *
 * Returns a freshly allocated copy the caller must free, or nullptr on
 * allocation failure. Replaces strdup to stay within the allowed headers.
 */
static char *clone_string(const char *source) {
    size_t length = std::strlen(source);
    char *copy = static_cast<char *>(std::malloc(length + 1));
    if (copy) {
        std::memcpy(copy, source, length + 1);
    }
    return copy;
}

/*
 * Create a word counter with `bucket_count` hash buckets.
 *
 * Returns a heap-allocated WordCounter to release with word_counter_free, or
 * nullptr on allocation failure. `bucket_count` is forced to at least 16.
 */
WordCounter *word_counter_create(int bucket_count) {
    if (bucket_count < 16) bucket_count = 16;
    WordCounter *counter =
        static_cast<WordCounter *>(std::malloc(sizeof(WordCounter)));
    if (!counter) return nullptr;

    counter->buckets = static_cast<WordNode **>(
        std::calloc(static_cast<size_t>(bucket_count), sizeof(WordNode *)));
    if (!counter->buckets) {
        std::free(counter);
        return nullptr;
    }
    counter->bucket_count = bucket_count;
    counter->unique_words = 0;
    return counter;
}

/*
 * Record one occurrence of `word`, inserting it if new.
 *
 * Increments the stored count for an existing word, or creates a node with
 * count 1 for a new word. Returns the word's updated count, or -1 on
 * allocation failure. Complexity: O(length + chain length).
 */
int word_counter_add(WordCounter *counter, const char *word) {
    uint32_t index = hash_word(word) % static_cast<uint32_t>(counter->bucket_count);

    /* Walk the bucket chain looking for an existing entry. */
    for (WordNode *node = counter->buckets[index]; node; node = node->next) {
        if (std::strcmp(node->word, word) == 0) {
            return ++node->count; /* Seen before: bump and report. */
        }
    }

    /* New word: prepend a node so insertion stays O(1) per collision. */
    WordNode *node = static_cast<WordNode *>(std::malloc(sizeof(WordNode)));
    if (!node) return -1;
    node->word = clone_string(word);
    if (!node->word) {
        std::free(node);
        return -1;
    }
    node->count = 1;
    node->next = counter->buckets[index];
    counter->buckets[index] = node;
    counter->unique_words++;
    return 1;
}

/*
 * Tokenize `text` into lowercase alphabetic words and tally each.
 *
 * Letters are folded to lowercase so "The" and "the" count together; any
 * non-letter acts as a separator. Returns the number of word occurrences
 * processed (total, not distinct), or -1 if any insertion fails midway.
 * Complexity: O(length of text).
 */
int word_counter_ingest(WordCounter *counter, const char *text) {
    char buffer[256];        /* Accumulates the current word; words are capped. */
    int length = 0;
    int total = 0;

    for (const char *p = text; ; p++) {
        char c = *p;
        int is_letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        if (is_letter) {
            /* Fold uppercase to lowercase, guarding the fixed buffer. */
            if (length < static_cast<int>(sizeof(buffer)) - 1) {
                char lower = (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
                buffer[length++] = lower;
            }
        } else if (length > 0) {
            /* A separator ends the current word; flush it to the table. */
            buffer[length] = '\0';
            if (word_counter_add(counter, buffer) < 0) return -1;
            total++;
            length = 0;
        }
        if (c == '\0') break; /* Loop terminates after handling final word. */
    }
    return total;
}

/*
 * Look up how many times `word` has been counted.
 *
 * Returns the stored count, or 0 if the word was never seen. Does not modify
 * the table. Complexity: O(length + chain length).
 */
int word_counter_get(const WordCounter *counter, const char *word) {
    uint32_t index = hash_word(word) % static_cast<uint32_t>(counter->bucket_count);
    for (WordNode *node = counter->buckets[index]; node; node = node->next) {
        if (std::strcmp(node->word, word) == 0) {
            return node->count;
        }
    }
    return 0;                /* Absent words have a frequency of zero. */
}

/*
 * Find the most frequent word in the counter.
 *
 * Returns a pointer to the table-owned string with the highest count, or
 * nullptr if no words have been recorded. Ties are broken by whichever word
 * the scan reaches first. The caller must not free the result.
 * Complexity: O(number of nodes).
 */
const char *word_counter_most_frequent(const WordCounter *counter) {
    const char *best_word = nullptr;
    int best_count = 0;
    for (int i = 0; i < counter->bucket_count; i++) {
        for (WordNode *node = counter->buckets[i]; node; node = node->next) {
            if (node->count > best_count) {
                best_count = node->count;
                best_word = node->word;
            }
        }
    }
    return best_word;
}

/*
 * Free a word counter and every node and string it owns.
 *
 * Safe to call with nullptr. Invalidates all pointers previously returned by
 * lookup functions. Complexity: O(number of nodes).
 */
void word_counter_free(WordCounter *counter) {
    if (!counter) return;
    for (int i = 0; i < counter->bucket_count; i++) {
        WordNode *node = counter->buckets[i];
        while (node) {
            WordNode *next = node->next;
            std::free(node->word);
            std::free(node);
            node = next;
        }
    }
    std::free(counter->buckets);
    std::free(counter);
}
