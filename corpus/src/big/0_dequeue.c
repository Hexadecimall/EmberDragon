/*
 * Doubly linked list serving as a double-ended queue (deque).
 *
 * Every node carries pointers to both neighbors, so insertion and removal at
 * either end run in O(1). The list keeps sentinel-free head and tail pointers
 * and a live element count. This is the structure to reach for when a workload
 * needs both stack- and queue-style access against the same collection.
 */

#include <stdlib.h>
#include <stdint.h>

/* A two-way linked node. `prev`/`next` are NULL at the respective ends. */
typedef struct DequeNode {
    int32_t value;
    struct DequeNode *prev;  /* toward the front */
    struct DequeNode *next;  /* toward the back */
} DequeNode;

/* Deque handle holding both ends plus a cached length. */
typedef struct {
    DequeNode *front;
    DequeNode *back;
    int32_t    length;
} Deque;

/* Reset a handle to the empty state. Does not free existing nodes. */
void deque_init(Deque *dq) {
    dq->front = NULL;
    dq->back = NULL;
    dq->length = 0;
}

/* Allocate a detached node holding `value`, or NULL on OOM. */
static DequeNode *deque_make_node(int32_t value) {
    DequeNode *node = (DequeNode *)malloc(sizeof(DequeNode));
    if (node != NULL) {
        node->value = value;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

/*
 * Insert `value` at the front of the deque.
 * @return 0 on success, -1 on allocation failure.
 */
int deque_push_front(Deque *dq, int32_t value) {
    DequeNode *node = deque_make_node(value);
    if (node == NULL) {
        return -1;
    }
    if (dq->front == NULL) {
        /* Empty deque: the new node is simultaneously front and back. */
        dq->front = node;
        dq->back = node;
    } else {
        node->next = dq->front;   /* splice ahead of the current front */
        dq->front->prev = node;
        dq->front = node;
    }
    dq->length++;
    return 0;
}

/*
 * Insert `value` at the back of the deque.
 * @return 0 on success, -1 on allocation failure.
 */
int deque_push_back(Deque *dq, int32_t value) {
    DequeNode *node = deque_make_node(value);
    if (node == NULL) {
        return -1;
    }
    if (dq->back == NULL) {
        dq->front = node;
        dq->back = node;
    } else {
        node->prev = dq->back;    /* splice after the current back */
        dq->back->next = node;
        dq->back = node;
    }
    dq->length++;
    return 0;
}

/*
 * Remove and return the front element.
 * @param out  receives the removed value if non-NULL.
 * @return 0 on success, -1 if the deque was empty.
 */
int deque_pop_front(Deque *dq, int32_t *out) {
    if (dq->front == NULL) {
        return -1;
    }
    DequeNode *node = dq->front;
    if (out != NULL) {
        *out = node->value;
    }
    dq->front = node->next;
    if (dq->front == NULL) {
        dq->back = NULL;          /* removed the last element */
    } else {
        dq->front->prev = NULL;   /* new front has no predecessor */
    }
    free(node);
    dq->length--;
    return 0;
}

/*
 * Remove and return the back element.
 * @param out  receives the removed value if non-NULL.
 * @return 0 on success, -1 if the deque was empty.
 */
int deque_pop_back(Deque *dq, int32_t *out) {
    if (dq->back == NULL) {
        return -1;
    }
    DequeNode *node = dq->back;
    if (out != NULL) {
        *out = node->value;
    }
    dq->back = node->prev;
    if (dq->back == NULL) {
        dq->front = NULL;         /* removed the last element */
    } else {
        dq->back->next = NULL;    /* new back has no successor */
    }
    free(node);
    dq->length--;
    return 0;
}

/*
 * Peek at the front value without removing it.
 * @return 1 and sets *out if non-empty, else 0.
 */
int deque_peek_front(const Deque *dq, int32_t *out) {
    if (dq->front == NULL) {
        return 0;
    }
    *out = dq->front->value;
    return 1;
}

/* Free every node and reset the handle. Safe on an empty deque. */
void deque_destroy(Deque *dq) {
    DequeNode *cur = dq->front;
    while (cur != NULL) {
        DequeNode *next = cur->next;
        free(cur);
        cur = next;
    }
    deque_init(dq);
}
