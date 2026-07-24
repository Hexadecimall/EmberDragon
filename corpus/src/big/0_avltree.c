/*
 * Self-balancing AVL tree of integer keys.
 *
 * An AVL tree is a binary search tree that keeps every node's two subtrees
 * within a height difference of one, restoring that balance after each insert
 * with at most a constant number of rotations. This guarantees O(log n)
 * search, insert, and worst-case height — unlike a plain BST that can degrade
 * to a linked list on sorted input.
 */

#include <stdlib.h>
#include <stdint.h>

/* A node carries its key plus a cached subtree height so balance factors can
 * be computed in O(1) without re-walking children. A lone leaf has height 1. */
typedef struct AvlNode {
    int32_t key;
    int32_t height;
    struct AvlNode *left;
    struct AvlNode *right;
} AvlNode;

/* Height of a possibly-NULL subtree; the empty tree has height 0. */
static int32_t avl_height(const AvlNode *node) {
    return (node == NULL) ? 0 : node->height;
}

/* Small integer max helper used when recomputing cached heights. */
static int32_t max_i32(int32_t a, int32_t b) {
    return (a > b) ? a : b;
}

/* Recompute and store a node's height from its children's cached heights. */
static void avl_update_height(AvlNode *node) {
    node->height = 1 + max_i32(avl_height(node->left), avl_height(node->right));
}

/*
 * Balance factor = left height minus right height.
 * Positive means left-heavy, negative means right-heavy; the AVL invariant
 * keeps this in the range [-1, 1] for every node after rebalancing.
 */
static int32_t avl_balance(const AvlNode *node) {
    if (node == NULL) {
        return 0;
    }
    return avl_height(node->left) - avl_height(node->right);
}

/* Allocate a fresh leaf node, or return NULL if out of memory. */
static AvlNode *avl_new_node(int32_t key) {
    AvlNode *node = (AvlNode *)malloc(sizeof(AvlNode));
    if (node != NULL) {
        node->key = key;
        node->height = 1;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

/*
 * Right rotation around `y`, used to fix a left-heavy imbalance.
 * Its left child `x` becomes the new subtree root. Returns the new root.
 * Heights of the two moved nodes are refreshed before returning.
 */
static AvlNode *avl_rotate_right(AvlNode *y) {
    AvlNode *x = y->left;
    AvlNode *orphan = x->right;  /* subtree that must change parents */

    x->right = y;        /* y descends to become x's right child */
    y->left = orphan;    /* the orphan reattaches under y */

    avl_update_height(y);  /* update the lower node first */
    avl_update_height(x);
    return x;
}

/*
 * Left rotation around `x`, the mirror image used for right-heavy imbalance.
 * Its right child `y` becomes the new subtree root. Returns the new root.
 */
static AvlNode *avl_rotate_left(AvlNode *x) {
    AvlNode *y = x->right;
    AvlNode *orphan = y->left;

    y->left = x;
    x->right = orphan;

    avl_update_height(x);
    avl_update_height(y);
    return y;
}

/*
 * Insert `key` and rebalance on the way back up the recursion.
 * @return the new subtree root for the parent to re-link.
 * Duplicate keys are ignored. Overall O(log n) with at most one rotation.
 */
AvlNode *avl_insert(AvlNode *root, int32_t key) {
    /* Phase 1: ordinary BST insertion. */
    if (root == NULL) {
        return avl_new_node(key);
    }
    if (key < root->key) {
        root->left = avl_insert(root->left, key);
    } else if (key > root->key) {
        root->right = avl_insert(root->right, key);
    } else {
        return root;  /* duplicate: set semantics, no change */
    }

    /* Phase 2: refresh height and inspect the balance factor. */
    avl_update_height(root);
    int32_t balance = avl_balance(root);

    /* Phase 3: four canonical imbalance cases. */
    if (balance > 1 && key < root->left->key) {
        return avl_rotate_right(root);              /* Left-Left */
    }
    if (balance < -1 && key > root->right->key) {
        return avl_rotate_left(root);               /* Right-Right */
    }
    if (balance > 1 && key > root->left->key) {
        root->left = avl_rotate_left(root->left);   /* Left-Right */
        return avl_rotate_right(root);
    }
    if (balance < -1 && key < root->right->key) {
        root->right = avl_rotate_right(root->right);/* Right-Left */
        return avl_rotate_left(root);
    }
    return root;  /* already balanced */
}

/*
 * Search for `key`. @return 1 if present, 0 otherwise. Iterative, O(log n).
 */
int avl_contains(const AvlNode *root, int32_t key) {
    while (root != NULL) {
        if (key == root->key) {
            return 1;
        }
        root = (key < root->key) ? root->left : root->right;
    }
    return 0;
}

/* Recursively release every node in the tree. */
void avl_destroy(AvlNode *root) {
    if (root == NULL) {
        return;
    }
    avl_destroy(root->left);
    avl_destroy(root->right);
    free(root);
}
