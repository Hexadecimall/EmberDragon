/*
 * trie.c — Prefix tree (trie) over lowercase ASCII letters.
 *
 * A trie stores a set of words as a tree where each edge is labeled by a
 * letter and each path from the root spells a prefix. It supports fast exact
 * lookup, prefix queries, and counting, all proportional to the length of the
 * key rather than the number of stored words.
 */

#include <stdlib.h>
#include <string.h>

/* Branching factor: one child slot per lowercase letter 'a'..'z'. */
#define TRIE_WIDTH 26

/*
 * A single node in the trie. `children` indexes by (letter - 'a'); a NULL
 * slot means that letter does not continue any stored word here.
 * `is_terminal` marks the end of an inserted word, and `word_count` records
 * how many times the word ending here was inserted (supports multisets).
 */
typedef struct TrieNode {
    struct TrieNode *children[TRIE_WIDTH];
    int is_terminal;
    int word_count;
} TrieNode;

/*
 * Allocate and zero-initialize a fresh trie node.
 *
 * Returns a pointer to the new node, or NULL on allocation failure. All child
 * pointers start NULL and the node is non-terminal.
 */
static TrieNode *trie_node_create(void) {
    TrieNode *node = calloc(1, sizeof(TrieNode));
    return node; /* calloc already zeroed children, is_terminal, word_count. */
}

/*
 * Create an empty trie and return its root node.
 *
 * Returns the root, or NULL on allocation failure. The caller owns the trie
 * and must release it with trie_free.
 */
TrieNode *trie_create(void) {
    return trie_node_create();
}

/*
 * Insert `word` into the trie, creating nodes along the way as needed.
 *
 * Returns 1 on success, 0 on allocation failure or if `word` contains a
 * character outside 'a'..'z'. Re-inserting an existing word increments its
 * count rather than failing. Complexity: O(length of word).
 */
int trie_insert(TrieNode *root, const char *word) {
    TrieNode *node = root;
    for (const char *p = word; *p; p++) {
        int index = *p - 'a';
        if (index < 0 || index >= TRIE_WIDTH) {
            return 0;        /* Reject anything outside the supported alphabet. */
        }
        if (!node->children[index]) {
            node->children[index] = trie_node_create();
            if (!node->children[index]) {
                return 0;    /* Out of memory midway; partial path is harmless. */
            }
        }
        node = node->children[index];
    }
    node->is_terminal = 1;   /* Mark the final node as a complete word. */
    node->word_count++;
    return 1;
}

/*
 * Walk the trie following `key` and return the node it lands on.
 *
 * Returns the node reached after consuming every character of `key`, or NULL
 * if the path breaks anywhere. Shared by lookup and prefix queries.
 * Complexity: O(length of key).
 */
static TrieNode *trie_descend(TrieNode *root, const char *key) {
    TrieNode *node = root;
    for (const char *p = key; *p; p++) {
        int index = *p - 'a';
        if (index < 0 || index >= TRIE_WIDTH || !node->children[index]) {
            return NULL;     /* Out-of-alphabet or missing edge: no such path. */
        }
        node = node->children[index];
    }
    return node;
}

/*
 * Test whether `word` was inserted into the trie as a complete word.
 *
 * Returns 1 if present, 0 otherwise. A prefix that is not itself a stored
 * word returns 0 even though its path exists. Complexity: O(length of word).
 */
int trie_contains(TrieNode *root, const char *word) {
    TrieNode *node = trie_descend(root, word);
    return node != NULL && node->is_terminal;
}

/*
 * Test whether any stored word starts with `prefix`.
 *
 * Returns 1 if at least one inserted word has `prefix` as a prefix, else 0.
 * The empty prefix returns 1 as long as the trie has any node (the root).
 * Complexity: O(length of prefix).
 */
int trie_starts_with(TrieNode *root, const char *prefix) {
    return trie_descend(root, prefix) != NULL;
}

/*
 * Count the number of distinct stored words in the subtree rooted at `node`.
 *
 * Returns that count. A terminal node contributes 1 regardless of its
 * word_count multiplicity. Complexity: O(number of nodes in the subtree).
 */
int trie_count_words(TrieNode *node) {
    if (!node) return 0;
    int total = node->is_terminal ? 1 : 0;
    for (int i = 0; i < TRIE_WIDTH; i++) {
        total += trie_count_words(node->children[i]);
    }
    return total;
}

/*
 * Recursively free the trie rooted at `node`, including `node` itself.
 *
 * Safe to call with NULL. After this returns the pointer is dangling and must
 * not be used. Complexity: O(number of nodes).
 */
void trie_free(TrieNode *node) {
    if (!node) return;
    for (int i = 0; i < TRIE_WIDTH; i++) {
        trie_free(node->children[i]);
    }
    free(node);
}
