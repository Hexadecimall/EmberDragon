#include <stdio.h>
#include <stdlib.h>

/* Singly linked list of integer transactions. */
struct Node {
    int value;
    struct Node *next;
};

struct LinkedList {
    struct Node *head;
    int count;
};

void list_init(struct LinkedList *list) {
    list->head = NULL;
    list->count = 0;
}

struct Node *make_node(int value) {
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->value = value;
    node->next = NULL;
    return node;
}

void push_front(struct LinkedList *list, int value) {
    struct Node *node = make_node(value);
    node->next = list->head;
    list->head = node;
    list->count = list->count + 1;
}

void append(struct LinkedList *list, int value) {
    struct Node *node = make_node(value);
    if (list->head == NULL) {
        list->head = node;
    } else {
        struct Node *cursor = list->head;
        while (cursor->next != NULL) {
            cursor = cursor->next;
        }
        cursor->next = node;
    }
    list->count = list->count + 1;
}

int sum_values(struct LinkedList *list) {
    int total = 0;
    struct Node *cursor = list->head;
    while (cursor != NULL) {
        total = total + cursor->value;
        cursor = cursor->next;
    }
    return total;
}

int remove_value(struct LinkedList *list, int target) {
    struct Node *prev = NULL;
    struct Node *cursor = list->head;
    while (cursor != NULL) {
        if (cursor->value == target) {
            if (prev == NULL) {
                list->head = cursor->next;
            } else {
                prev->next = cursor->next;
            }
            free(cursor);
            list->count = list->count - 1;
            return 1;
        }
        prev = cursor;
        cursor = cursor->next;
    }
    return 0;
}

void reverse_list(struct LinkedList *list) {
    struct Node *prev = NULL;
    struct Node *cursor = list->head;
    while (cursor != NULL) {
        struct Node *next = cursor->next;
        cursor->next = prev;
        prev = cursor;
        cursor = next;
    }
    list->head = prev;
}

int count_greater(struct LinkedList *list, int threshold) {
    int matches = 0;
    struct Node *cursor = list->head;
    while (cursor != NULL) {
        if (cursor->value > threshold) {
            matches = matches + 1;
        }
        cursor = cursor->next;
    }
    return matches;
}
