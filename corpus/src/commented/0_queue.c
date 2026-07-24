/*
 * Circular FIFO queue of integers over a fixed-size array.
 *
 * Head and tail indices wrap around the buffer modulo its capacity,
 * so enqueue and dequeue are both O(1) with no element shifting. A
 * separate count disambiguates the otherwise-identical full and empty
 * states (head == tail).
 */
#include <stdio.h>
#include <stdlib.h>

/*
 * Queue state. Elements live in [head, head + count) interpreted
 * circularly; `tail` is the index of the next free slot.
 */
typedef struct {
    int *buffer;
    int capacity;
    int head;  /* Index of the oldest element. */
    int tail;  /* Index where the next element will be written. */
    int count; /* Number of elements currently queued. */
} Queue;

/*
 * Allocate a queue able to hold `capacity` elements.
 * Returns a heap-allocated Queue (caller owns it), or NULL on failure
 * or if `capacity` is not positive.
 */
Queue *queueCreate(int capacity) {
    if (capacity <= 0) {
        return NULL;
    }
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q == NULL) {
        return NULL;
    }
    q->buffer = (int *)malloc(sizeof(int) * capacity);
    if (q->buffer == NULL) {
        free(q); /* Avoid leaking the partially built queue. */
        return NULL;
    }
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    return q;
}

/* Return true when no elements are queued. */
int queueIsEmpty(const Queue *q) {
    return q->count == 0;
}

/* Return true when the queue cannot accept more elements. */
int queueIsFull(const Queue *q) {
    return q->count == q->capacity;
}

/*
 * Append `value` to the back of the queue. O(1).
 * Returns 1 on success, 0 if the queue is full.
 */
int enqueue(Queue *q, int value) {
    if (queueIsFull(q)) {
        return 0;
    }
    q->buffer[q->tail] = value;
    /* Advance the write index, wrapping at the end of the buffer. */
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    return 1;
}

/*
 * Remove the oldest element into `*out`. O(1).
 * Returns 1 on success, 0 if the queue is empty.
 */
int dequeue(Queue *q, int *out) {
    if (queueIsEmpty(q)) {
        return 0;
    }
    *out = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity; /* Wrap the read index too. */
    q->count--;
    return 1;
}

/*
 * Inspect the front element without removing it.
 * Returns 1 and writes `*out` if non-empty, 0 otherwise.
 */
int queuePeek(const Queue *q, int *out) {
    if (queueIsEmpty(q)) {
        return 0;
    }
    *out = q->buffer[q->head];
    return 1;
}

/*
 * Free the buffer and the queue struct itself.
 * Safe to pass NULL, in which case it is a no-op.
 */
void queueDestroy(Queue *q) {
    if (q == NULL) {
        return;
    }
    free(q->buffer);
    free(q);
}
