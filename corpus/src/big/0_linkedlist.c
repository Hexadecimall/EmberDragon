/*
 * Singly linked list of integer keys with a maintained length counter.
 *
 * This module implements the classic intrusive-free singly linked list: each
 * node owns a heap-allocated cell holding one value and a forward pointer.
 * It is useful when you need cheap O(1) prepends and stable element identity
 * but do not require random access. The list keeps a cached size so callers
 * can query length without walking the chain.
 */

#include <stdlib.h>
#include <stdint.h>

/* A single chain cell. The list is owned through `head`; every node past it
 * is reachable by following `next` until the NULL terminator. */
typedef struct ListNode {
    int32_t value;          /* the payload stored in this cell */
    struct ListNode *next;  /* successor, or NULL if this is the tail */
} ListNode;

/* List handle. Holding both `head` and `size` lets length() run in O(1)
 * instead of re-counting the chain on every query. */
typedef struct {
    ListNode *head;  /* first node, or NULL when the list is empty */
    int32_t   size;  /* number of nodes currently linked in */
} LinkedList;

/*
 * Initialize an already-allocated list handle to the empty state.
 * @param list  handle to reset; must be non-NULL.
 * No return value. Does not free any prior contents — call clear() first.
 */
void list_init(LinkedList *list) {
    list->head = NULL;
    list->size = 0;
}

/*
 * Insert a value at the front of the list.
 * @param list   destination list.
 * @param value  payload to store.
 * @return 0 on success, -1 if allocation failed (list left unchanged).
 * Runs in O(1): we only splice a new node ahead of the current head.
 */
int list_push_front(LinkedList *list, int32_t value) {
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    if (node == NULL) {
        return -1;  /* propagate OOM; caller decides how to recover */
    }
    node->value = value;
    node->next = list->head;  /* old head becomes our successor */
    list->head = node;        /* new node is now the entry point */
    list->size++;
    return 0;
}

/*
 * Remove and return the value at the front of the list.
 * @param list      source list (must be non-empty).
 * @param out_value receives the popped value; ignored if NULL.
 * @return 0 on success, -1 if the list was empty.
 */
int list_pop_front(LinkedList *list, int32_t *out_value) {
    if (list->head == NULL) {
        return -1;  /* nothing to pop */
    }
    ListNode *old_head = list->head;
    if (out_value != NULL) {
        *out_value = old_head->value;
    }
    list->head = old_head->next;  /* advance past the cell we are freeing */
    free(old_head);
    list->size--;
    return 0;
}

/*
 * Find the zero-based index of the first node holding `target`.
 * @return the index, or -1 if no node carries that value.
 * Linear scan, O(n).
 */
int32_t list_index_of(const LinkedList *list, int32_t target) {
    int32_t index = 0;
    for (const ListNode *cur = list->head; cur != NULL; cur = cur->next) {
        if (cur->value == target) {
            return index;
        }
        index++;
    }
    return -1;
}

/*
 * Reverse the list in place by re-pointing every `next` link backward.
 * After this call the former tail is the head. Runs in O(n) with O(1)
 * extra space using the standard three-pointer walk.
 */
void list_reverse(LinkedList *list) {
    ListNode *prev = NULL;
    ListNode *cur = list->head;
    while (cur != NULL) {
        ListNode *next = cur->next;  /* stash before we overwrite the link */
        cur->next = prev;            /* flip this node's pointer backward */
        prev = cur;                  /* slide the window forward */
        cur = next;
    }
    list->head = prev;  /* prev now references the former tail */
}

/*
 * Free every node and reset the handle to empty. Safe to call repeatedly.
 */
void list_clear(LinkedList *list) {
    ListNode *cur = list->head;
    while (cur != NULL) {
        ListNode *next = cur->next;  /* save successor before freeing */
        free(cur);
        cur = next;
    }
    list->head = NULL;
    list->size = 0;
}
