/*
 * Unbalanced binary search tree of integers.
 *
 * Supports insertion, membership lookup, deletion (including the
 * two-child case via in-order successor), and an in-order minimum
 * query. Average operations are O(log n); worst case is O(n) on a
 * degenerate, sorted-input tree.
 */
#include <stdio.h>
#include <stdlib.h>

/* A node holds a key and pointers to its two ordered subtrees. */
typedef struct TreeNode {
    int key;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

/*
 * Allocate a leaf node holding `key`.
 * Returns the new node, or NULL on allocation failure.
 */
static TreeNode *createNode(int key) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node == NULL) {
        return NULL;
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/*
 * Insert `key` into the subtree `root`, preserving BST ordering.
 * Returns the (possibly new) subtree root. Duplicate keys are ignored,
 * so the tree behaves as a set.
 */
TreeNode *bstInsert(TreeNode *root, int key) {
    if (root == NULL) {
        return createNode(key); /* Reached an empty slot: plant the key. */
    }
    if (key < root->key) {
        root->left = bstInsert(root->left, key);
    } else if (key > root->key) {
        root->right = bstInsert(root->right, key);
    }
    /* key == root->key: already present, leave the tree unchanged. */
    return root;
}

/*
 * Test whether `key` exists in the tree. O(h) where h is the height.
 * Returns 1 if found, 0 otherwise. Iterative to avoid recursion depth.
 */
int bstContains(TreeNode *root, int key) {
    TreeNode *cur = root;
    while (cur != NULL) {
        if (key == cur->key) {
            return 1;
        }
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;
}

/*
 * Return the node with the smallest key in `root`.
 * Assumes `root` is non-NULL; the minimum is the leftmost node.
 */
static TreeNode *findMin(TreeNode *root) {
    TreeNode *cur = root;
    while (cur->left != NULL) {
        cur = cur->left;
    }
    return cur;
}

/*
 * Delete `key` from the subtree `root` if present.
 * Returns the new subtree root. Handles the three classic cases:
 * leaf, single child, and two children (replaced by in-order successor).
 */
TreeNode *bstDelete(TreeNode *root, int key) {
    if (root == NULL) {
        return NULL; /* Key not in tree; nothing to do. */
    }
    if (key < root->key) {
        root->left = bstDelete(root->left, key);
    } else if (key > root->key) {
        root->right = bstDelete(root->right, key);
    } else {
        /* Found the node to remove. */
        if (root->left == NULL) {
            TreeNode *right = root->right;
            free(root);
            return right; /* Promote the right child (may be NULL). */
        }
        if (root->right == NULL) {
            TreeNode *left = root->left;
            free(root);
            return left; /* Promote the left child. */
        }
        /* Two children: copy the in-order successor's key here, then
         * delete that successor from the right subtree. */
        TreeNode *successor = findMin(root->right);
        root->key = successor->key;
        root->right = bstDelete(root->right, successor->key);
    }
    return root;
}

/*
 * Count the nodes in the tree. O(n).
 * Returns 0 for an empty (NULL) tree.
 */
int bstCount(TreeNode *root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + bstCount(root->left) + bstCount(root->right);
}

/*
 * Free the entire tree using a post-order traversal so children are
 * released before their parent. After this call the caller's pointer
 * is dangling and must not be reused.
 */
void bstDestroy(TreeNode *root) {
    if (root == NULL) {
        return;
    }
    bstDestroy(root->left);
    bstDestroy(root->right);
    free(root);
}
