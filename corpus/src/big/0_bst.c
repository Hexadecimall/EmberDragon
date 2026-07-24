/*
 * Unbalanced binary search tree keyed on signed integers.
 *
 * Each node stores a key and pointers to a left subtree (smaller keys) and a
 * right subtree (larger keys). This is the textbook ordered-set structure:
 * average-case O(log n) search/insert when keys arrive in random order, but
 * it degrades to O(n) on sorted input since no rebalancing is performed.
 */

#include <stdlib.h>
#include <stdint.h>

/* A node in the search tree. The BST ordering invariant holds at every node:
 * all keys in `left` are strictly less than `key`, and all keys in `right`
 * are strictly greater. Duplicate keys are rejected by insert(). */
typedef struct TreeNode {
    int32_t key;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/*
 * Allocate and initialize a leaf node holding `key`.
 * @return the new node, or NULL on allocation failure.
 */
static TreeNode *node_create(int32_t key) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node != NULL) {
        node->key = key;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/*
 * Insert `key` into the subtree rooted at `root`.
 * @return the (possibly new) subtree root, so the parent can re-link.
 * Duplicate keys are ignored, preserving set semantics. Average O(log n).
 */
TreeNode *bst_insert(TreeNode *root, int32_t key) {
    if (root == NULL) {
        return node_create(key);  /* reached the empty slot for this key */
    }
    if (key < root->key) {
        root->left = bst_insert(root->left, key);
    } else if (key > root->key) {
        root->right = bst_insert(root->right, key);
    }
    /* key == root->key: already present, fall through and return unchanged */
    return root;
}

/*
 * Test whether `key` exists in the tree.
 * @return 1 if found, 0 otherwise. Iterative descent, average O(log n).
 */
int bst_contains(const TreeNode *root, int32_t key) {
    const TreeNode *cur = root;
    while (cur != NULL) {
        if (key == cur->key) {
            return 1;
        }
        /* Branch toward the side that can contain key per the BST invariant. */
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;
}

/*
 * Return the smallest key in a non-empty subtree.
 * @param root  must be non-NULL.
 * The minimum lives at the leftmost node, so we just keep going left.
 */
static TreeNode *bst_min_node(TreeNode *root) {
    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}

/*
 * Remove `key` from the subtree and return the new subtree root.
 * Handles all three deletion cases: leaf, single child, and two children
 * (replaced by its in-order successor). No-op if the key is absent.
 */
TreeNode *bst_remove(TreeNode *root, int32_t key) {
    if (root == NULL) {
        return NULL;  /* key not present in this branch */
    }
    if (key < root->key) {
        root->left = bst_remove(root->left, key);
    } else if (key > root->key) {
        root->right = bst_remove(root->right, key);
    } else {
        /* Found the node to delete. */
        if (root->left == NULL) {
            TreeNode *right = root->right;  /* may be NULL (leaf case) */
            free(root);
            return right;
        }
        if (root->right == NULL) {
            TreeNode *left = root->left;
            free(root);
            return left;
        }
        /* Two children: copy in the successor's key, then delete it below. */
        TreeNode *succ = bst_min_node(root->right);
        root->key = succ->key;
        root->right = bst_remove(root->right, succ->key);
    }
    return root;
}

/*
 * Compute the height of the subtree (edges on the longest root-to-leaf path).
 * @return -1 for an empty subtree so a single node reports height 0.
 */
int32_t bst_height(const TreeNode *root) {
    if (root == NULL) {
        return -1;
    }
    int32_t left_h = bst_height(root->left);
    int32_t right_h = bst_height(root->right);
    int32_t taller = (left_h > right_h) ? left_h : right_h;
    return taller + 1;
}

/*
 * Count the number of nodes in the subtree. O(n) post-order traversal.
 */
int32_t bst_size(const TreeNode *root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + bst_size(root->left) + bst_size(root->right);
}

/*
 * Recursively free every node. The handle's root should be set to NULL by
 * the caller afterward, since this leaves it dangling.
 */
void bst_destroy(TreeNode *root) {
    if (root == NULL) {
        return;
    }
    bst_destroy(root->left);
    bst_destroy(root->right);
    free(root);
}
