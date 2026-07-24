/*
 * trie.cpp
 *
 * A prefix tree (trie) over lowercase ASCII letters 'a'..'z'. Supports
 * inserting words, exact-membership queries, prefix queries, and counting how
 * many stored words begin with a given prefix. Memory is owned by the tree and
 * released recursively in the destructor.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

/* Branching factor: one child slot per lowercase letter. */
static const int ALPHABET_SIZE = 26;

/*
 * One node in the trie. `children[c]` points at the subtree reached by
 * appending letter ('a' + c). `isEndOfWord` marks that some inserted word ends
 * exactly here, and `prefixCount` records how many inserted words pass through
 * this node (i.e. share the prefix spelled out by the path to it).
 */
struct TrieNode {
    TrieNode *children[ALPHABET_SIZE];
    bool      isEndOfWord;
    int       prefixCount;
};

/*
 * Allocate and zero-initialize a fresh trie node. Returns a node with no
 * children, not marking a word end, and a zero prefix count. The caller owns
 * the returned pointer. Returns nullptr on allocation failure.
 */
static TrieNode *createNode() {
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (node == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = nullptr;
    }
    node->isEndOfWord = false;
    node->prefixCount = 0;
    return node;
}

/*
 * Map an ASCII character to its child slot index, or -1 if the character is
 * outside the supported 'a'..'z' range. Keeps the range check in one place.
 */
static int charToIndex(char c) {
    if (c < 'a' || c > 'z') {
        return -1;
    }
    return c - 'a';
}

/* The trie itself: just a root node plus the operations that walk it. */
class Trie {
public:
    /* Construct an empty trie holding only the (wordless) root node. */
    Trie() {
        root = createNode();
    }

    /* Recursively free every node owned by the trie. */
    ~Trie() {
        freeSubtree(root);
    }

    /*
     * Insert `word` into the trie, creating nodes as needed and bumping the
     * prefix count along the path. Characters outside 'a'..'z' abort the
     * insertion (the word is rejected wholesale). Returns true if the word was
     * stored, false on a bad character or allocation failure. O(L) in the
     * word length L.
     */
    bool insert(const char *word) {
        TrieNode *node = root;
        for (const char *p = word; *p != '\0'; p++) {
            int index = charToIndex(*p);
            if (index < 0) {
                return false;  /* unsupported character: reject the word */
            }
            if (node->children[index] == nullptr) {
                TrieNode *child = createNode();
                if (child == nullptr) {
                    return false;
                }
                node->children[index] = child;
            }
            node = node->children[index];
            node->prefixCount++;  /* one more word travels through this node */
        }
        node->isEndOfWord = true;
        return true;
    }

    /*
     * Return true if `word` was inserted exactly (not merely as a prefix of a
     * longer word). Returns false if the path runs out or the final node is
     * not a word end. O(L).
     */
    bool contains(const char *word) const {
        const TrieNode *node = findNode(word);
        return node != nullptr && node->isEndOfWord;
    }

    /*
     * Return true if any inserted word starts with `prefix`. The empty prefix
     * always matches. O(L) in the prefix length.
     */
    bool startsWith(const char *prefix) const {
        return findNode(prefix) != nullptr;
    }

    /*
     * Return the number of inserted words that begin with `prefix`. Returns 0
     * if no such word exists. Reads the cached prefixCount on the prefix's
     * terminal node, so it is O(L) rather than O(number of words).
     */
    int countWordsWithPrefix(const char *prefix) const {
        const TrieNode *node = findNode(prefix);
        return (node == nullptr) ? 0 : node->prefixCount;
    }

private:
    TrieNode *root;

    /*
     * Walk the trie following `key` and return the node reached by the final
     * character, or nullptr if any character is unsupported or absent. Shared
     * by contains(), startsWith(), and countWordsWithPrefix().
     */
    const TrieNode *findNode(const char *key) const {
        const TrieNode *node = root;
        for (const char *p = key; *p != '\0'; p++) {
            int index = charToIndex(*p);
            if (index < 0 || node->children[index] == nullptr) {
                return nullptr;
            }
            node = node->children[index];
        }
        return node;
    }

    /*
     * Recursively free `node` and all of its descendants. A nullptr is a safe
     * no-op base case, so leaves terminate the recursion cleanly.
     */
    static void freeSubtree(TrieNode *node) {
        if (node == nullptr) {
            return;
        }
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            freeSubtree(node->children[i]);
        }
        free(node);
    }
};
