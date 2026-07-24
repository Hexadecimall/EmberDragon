/*
 * Canonical Huffman tree construction from byte frequencies.
 *
 * Given how often each of the 256 byte values occurs, this module builds an
 * optimal prefix-code tree by repeatedly merging the two least-frequent nodes,
 * then derives the bit length of every symbol's code. It deliberately stops at
 * code lengths rather than emitting bits, which is the reusable core shared by
 * most Huffman codecs.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* The alphabet is the full range of byte values. */
static const int kSymbolCount = 256;

/*
 * One node in the Huffman forest. Leaves carry a symbol; internal nodes carry
 * -1 and link to two children. `weight` is the combined frequency.
 */
struct HuffNode {
    uint64_t weight;
    int      symbol;   /* -1 for internal nodes. */
    HuffNode *left;
    HuffNode *right;
};

/*
 * Allocate and initialise a single Huffman node.
 *
 * symbol < 0 marks an internal node. The caller owns the returned pointer and
 * must release the whole tree via freeHuffTree().
 * Returns NULL if allocation fails.
 */
static HuffNode *makeNode(uint64_t weight, int symbol,
                          HuffNode *left, HuffNode *right) {
    HuffNode *n = (HuffNode *)malloc(sizeof(HuffNode));
    if (!n) {
        return nullptr;
    }
    n->weight = weight;
    n->symbol = symbol;
    n->left = left;
    n->right = right;
    return n;
}

/*
 * Remove and return the node with the smallest weight from a pointer array.
 *
 * This is a linear scan rather than a real heap: clear and adequate for a
 * 256-symbol alphabet. The chosen slot is back-filled with the last element so
 * the array stays packed. *count is decremented.
 *
 * Returns the extracted node, or NULL when the array is empty.
 * O(count) per call.
 */
static HuffNode *extractMin(HuffNode **nodes, int *count) {
    if (*count == 0) {
        return nullptr;
    }
    int best = 0;
    for (int i = 1; i < *count; i++) {
        if (nodes[i]->weight < nodes[best]->weight) {
            best = i;
        }
    }
    HuffNode *result = nodes[best];
    nodes[best] = nodes[*count - 1]; /* Compact by moving the tail into the hole. */
    (*count)--;
    return result;
}

/*
 * Recursively assign code lengths by walking the tree.
 *
 * Each edge descended adds one bit, so a leaf at depth d receives length d.
 * Lengths are written into codeLengths[symbol]. A degenerate tree with a single
 * leaf is handled by the caller, which forces a minimum length of 1.
 */
static void assignLengths(const HuffNode *node, int depth, uint8_t *codeLengths) {
    if (!node) {
        return;
    }
    if (node->symbol >= 0) {
        codeLengths[node->symbol] = (uint8_t)depth;
        return;
    }
    assignLengths(node->left, depth + 1, codeLengths);
    assignLengths(node->right, depth + 1, codeLengths);
}

/*
 * Free an entire Huffman tree with a post-order traversal.
 * Safe to call on NULL.
 */
void freeHuffTree(HuffNode *node) {
    if (!node) {
        return;
    }
    freeHuffTree(node->left);
    freeHuffTree(node->right);
    free(node);
}

/*
 * Build a Huffman tree from frequencies and fill in per-symbol code lengths.
 *
 * frequencies : array of kSymbolCount counts; zero-frequency symbols are
 *               excluded from the tree and get a code length of 0.
 * codeLengths : output array of kSymbolCount bytes, fully overwritten.
 *
 * Returns the tree root (caller frees via freeHuffTree), or NULL on allocation
 * failure or when no symbol has a non-zero frequency. Complexity O(k^2) for the
 * repeated linear extract-min, where k is the number of distinct symbols.
 */
HuffNode *buildHuffTree(const uint64_t *frequencies, uint8_t *codeLengths) {
    memset(codeLengths, 0, (size_t)kSymbolCount);

    HuffNode **heap = (HuffNode **)malloc(sizeof(HuffNode *) * kSymbolCount);
    if (!heap) {
        return nullptr;
    }
    int count = 0;
    for (int s = 0; s < kSymbolCount; s++) {
        if (frequencies[s] > 0) {
            heap[count++] = makeNode(frequencies[s], s, nullptr, nullptr);
        }
    }
    if (count == 0) {
        free(heap);
        return nullptr; /* Nothing to encode. */
    }
    /* Edge case: a one-symbol alphabet still needs a 1-bit code, not 0. */
    if (count == 1) {
        codeLengths[heap[0]->symbol] = 1;
        HuffNode *only = heap[0];
        free(heap);
        return only;
    }

    /* Merge the two lightest nodes until a single root remains. */
    while (count > 1) {
        HuffNode *a = extractMin(heap, &count);
        HuffNode *b = extractMin(heap, &count);
        HuffNode *parent = makeNode(a->weight + b->weight, -1, a, b);
        if (!parent) {
            heap[count++] = a;
            heap[count++] = b;
            break; /* On OOM, leave a partial forest the caller can still free. */
        }
        heap[count++] = parent;
    }
    HuffNode *root = heap[0];
    free(heap);
    assignLengths(root, 0, codeLengths);
    return root;
}
