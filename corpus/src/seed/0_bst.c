#include <stdio.h>
#include <stdlib.h>

/* Binary search tree keyed by integer id. */
struct TreeNode {
    int key;
    int height;
    struct TreeNode *left;
    struct TreeNode *right;
};

struct TreeNode *create_tree_node(int key) {
    struct TreeNode *node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    node->key = key;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

struct TreeNode *insert_key(struct TreeNode *root, int key) {
    if (root == NULL) {
        return create_tree_node(key);
    }
    if (key < root->key) {
        root->left = insert_key(root->left, key);
    } else if (key > root->key) {
        root->right = insert_key(root->right, key);
    }
    return root;
}

struct TreeNode *find_key(struct TreeNode *root, int key) {
    struct TreeNode *cursor = root;
    while (cursor != NULL) {
        if (key == cursor->key) {
            return cursor;
        }
        if (key < cursor->key) {
            cursor = cursor->left;
        } else {
            cursor = cursor->right;
        }
    }
    return NULL;
}

int find_minimum(struct TreeNode *root) {
    struct TreeNode *cursor = root;
    if (cursor == NULL) {
        return -1;
    }
    while (cursor->left != NULL) {
        cursor = cursor->left;
    }
    return cursor->key;
}

int tree_height(struct TreeNode *root) {
    int left_height;
    int right_height;
    if (root == NULL) {
        return 0;
    }
    left_height = tree_height(root->left);
    right_height = tree_height(root->right);
    if (left_height > right_height) {
        return left_height + 1;
    }
    return right_height + 1;
}

int count_nodes(struct TreeNode *root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + count_nodes(root->left) + count_nodes(root->right);
}

int sum_in_range(struct TreeNode *root, int low, int high) {
    int total = 0;
    if (root == NULL) {
        return 0;
    }
    if (root->key >= low && root->key <= high) {
        total = total + root->key;
    }
    if (root->key > low) {
        total = total + sum_in_range(root->left, low, high);
    }
    if (root->key < high) {
        total = total + sum_in_range(root->right, low, high);
    }
    return total;
}

void free_tree(struct TreeNode *root) {
    if (root == NULL) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}
