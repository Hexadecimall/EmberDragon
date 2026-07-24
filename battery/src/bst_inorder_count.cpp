// Binary search tree: insert + recursive inorder traversal counting nodes & sum.
#include <iostream>

struct TNode {
    int key;
    TNode *left;
    TNode *right;
};

static TNode *insert(TNode *root, int key) {
    if (root == nullptr) {
        TNode *n = new TNode;
        n->key = key;
        n->left = n->right = nullptr;
        return n;
    }
    if (key < root->key)
        root->left = insert(root->left, key);
    else
        root->right = insert(root->right, key);
    return root;
}

static void inorder(const TNode *root, int &count, long &sum) {
    if (root == nullptr) return;
    inorder(root->left, count, sum);
    count++;
    sum += root->key;
    inorder(root->right, count, sum);
}

int main() {
    int keys[] = {50, 30, 70, 20, 40, 60, 80, 10};
    TNode *root = nullptr;
    for (int k : keys)
        root = insert(root, k);
    int count = 0;
    long sum = 0;
    inorder(root, count, sum);
    std::cout << "nodes=" << count << " sum=" << sum << "\n";
    return 0;
}
