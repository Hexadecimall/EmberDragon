#include <cstdio>
#include <cstdlib>

/* Doubly linked list acting as a deque for an undo/redo history. */
struct HistoryNode {
    int command;
    HistoryNode *prev;
    HistoryNode *next;
};

class Deque {
public:
    Deque() : front(nullptr), back(nullptr), length(0) {}

    int getLength() const {
        return length;
    }

    bool isEmpty() const {
        return length == 0;
    }

    void pushFront(int command) {
        HistoryNode *node = allocate(command);
        if (front == nullptr) {
            front = node;
            back = node;
        } else {
            node->next = front;
            front->prev = node;
            front = node;
        }
        length = length + 1;
    }

    void pushBack(int command) {
        HistoryNode *node = allocate(command);
        if (back == nullptr) {
            front = node;
            back = node;
        } else {
            node->prev = back;
            back->next = node;
            back = node;
        }
        length = length + 1;
    }

    bool popFront(int &out) {
        if (front == nullptr) {
            return false;
        }
        HistoryNode *node = front;
        out = node->command;
        front = node->next;
        if (front == nullptr) {
            back = nullptr;
        } else {
            front->prev = nullptr;
        }
        delete node;
        length = length - 1;
        return true;
    }

    bool popBack(int &out) {
        if (back == nullptr) {
            return false;
        }
        HistoryNode *node = back;
        out = node->command;
        back = node->prev;
        if (back == nullptr) {
            front = nullptr;
        } else {
            back->next = nullptr;
        }
        delete node;
        length = length - 1;
        return true;
    }

    int sumForward() const {
        int total = 0;
        HistoryNode *cursor = front;
        while (cursor != nullptr) {
            total = total + cursor->command;
            cursor = cursor->next;
        }
        return total;
    }

    int sumBackward() const {
        int total = 0;
        HistoryNode *cursor = back;
        while (cursor != nullptr) {
            total = total + cursor->command;
            cursor = cursor->prev;
        }
        return total;
    }

private:
    HistoryNode *allocate(int command) {
        HistoryNode *node = new HistoryNode;
        node->command = command;
        node->prev = nullptr;
        node->next = nullptr;
        return node;
    }

    HistoryNode *front;
    HistoryNode *back;
    int length;
};

int trimOldest(Deque &deque, int keep) {
    int removed = 0;
    while (deque.getLength() > keep) {
        int discarded;
        if (!deque.popFront(discarded)) {
            break;
        }
        removed = removed + 1;
    }
    return removed;
}
