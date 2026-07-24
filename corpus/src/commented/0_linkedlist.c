/*
 * Singly linked list with head/tail tracking.
 *
 * Provides O(1) append and prepend, plus value-based removal and a
 * reversal routine. The list owns every node it links in and frees
 * them on destruction.
 */
#include <stdio.h>
#include <stdlib.h>

/* One element of the list. `next` is NULL at the tail. */
typedef struct ListNode {
    int value;
    struct ListNode *next;
} ListNode;

/*
 * List container holding both ends plus a running count so that
 * size() is O(1) and append() never has to walk the chain.
 */
typedef struct {
    ListNode *head;
    ListNode *tail;
    int count;
} LinkedList;

/*
 * Initialize an empty list in place.
 * `list`: caller-provided storage to reset; must not be NULL.
 */
void listInit(LinkedList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

/*
 * Allocate and initialize a detached node.
 * Returns a heap node holding `value`, or NULL if allocation fails.
 * The caller is responsible for linking or freeing it.
 */
static ListNode *makeNode(int value) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    if (node == NULL) {
        return NULL;
    }
    node->value = value;
    node->next = NULL;
    return node;
}

/*
 * Append `value` to the end of the list. O(1) thanks to the tail pointer.
 * Returns 1 on success, 0 if memory allocation failed.
 */
int listAppend(LinkedList *list, int value) {
    ListNode *node = makeNode(value);
    if (node == NULL) {
        return 0;
    }
    if (list->tail == NULL) {
        /* Empty list: the new node becomes both head and tail. */
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->count++;
    return 1;
}

/*
 * Insert `value` at the front of the list. O(1).
 * Returns 1 on success, 0 if memory allocation failed.
 */
int listPrepend(LinkedList *list, int value) {
    ListNode *node = makeNode(value);
    if (node == NULL) {
        return 0;
    }
    node->next = list->head;
    list->head = node;
    if (list->tail == NULL) {
        /* The list was empty, so this node is also the tail. */
        list->tail = node;
    }
    list->count++;
    return 1;
}

/*
 * Remove the first node whose value equals `value`. O(n).
 * Returns 1 if a matching node was found and freed, 0 otherwise.
 */
int listRemove(LinkedList *list, int value) {
    ListNode *prev = NULL;
    ListNode *cur = list->head;
    while (cur != NULL) {
        if (cur->value == value) {
            if (prev == NULL) {
                list->head = cur->next; /* Removing the head. */
            } else {
                prev->next = cur->next;
            }
            /* Keep the tail pointer correct when deleting the last node. */
            if (cur == list->tail) {
                list->tail = prev;
            }
            free(cur);
            list->count--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

/*
 * Reverse the list in place by re-pointing every `next` link. O(n).
 * Head and tail are swapped; no nodes are allocated or freed.
 */
void listReverse(LinkedList *list) {
    ListNode *prev = NULL;
    ListNode *cur = list->head;
    list->tail = list->head; /* Old head will become the new tail. */
    while (cur != NULL) {
        ListNode *next = cur->next; /* Stash before we overwrite the link. */
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    list->head = prev;
}

/*
 * Free every node and reset the list to empty. O(n).
 * Safe to call on an already-empty list.
 */
void listDestroy(LinkedList *list) {
    ListNode *cur = list->head;
    while (cur != NULL) {
        ListNode *next = cur->next;
        free(cur);
        cur = next;
    }
    listInit(list);
}
