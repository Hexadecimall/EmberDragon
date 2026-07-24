/*
 * ring_event_queue.c
 *
 * A fixed-capacity single-producer/single-consumer ring buffer for input
 * events (key presses, mouse moves, window resizes, etc.). The queue stores
 * events in a power-of-two-sized array and uses monotonically increasing head
 * and tail indices that are masked down to slots, which keeps enqueue and
 * dequeue at O(1) with no element shifting and no modulo on the hot path.
 */

#include <stdint.h>
#include <string.h>

/* Categories of events the queue can carry. */
typedef enum {
    EVENT_NONE = 0,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_MOUSE_MOVE,
    EVENT_RESIZE,
    EVENT_QUIT
} EventType;

/* A single event record. The two payload words are interpreted per type
 * (e.g. key code, or x/y coordinates for mouse moves). */
typedef struct {
    EventType type;
    int32_t   arg0;
    int32_t   arg1;
    uint64_t  timestamp;
} Event;

/* The queue capacity MUST be a power of two so that index & MASK is a valid
 * substitute for index % CAPACITY. */
#define QUEUE_CAPACITY 64u
#define QUEUE_MASK     (QUEUE_CAPACITY - 1u)

/* Ring buffer state. `head` is the next slot to write; `tail` is the next slot
 * to read. Both run freely upward and are masked at access time, so the buffer
 * is empty when head == tail and full when (head - tail) == QUEUE_CAPACITY. */
typedef struct {
    Event    slots[QUEUE_CAPACITY];
    uint32_t head;
    uint32_t tail;
} EventQueue;

/*
 * Reset a queue to the empty state.
 * @param q  queue to initialize (must be non-NULL)
 * No return value. Safe to call on a freshly allocated, uninitialized struct.
 */
void event_queue_init(EventQueue *q) {
    q->head = 0;
    q->tail = 0;
    /* Zeroing the backing store is not required for correctness, but it makes
     * stale slots read as EVENT_NONE which simplifies debugging. */
    memset(q->slots, 0, sizeof(q->slots));
}

/*
 * Report how many events are currently buffered.
 * @param q  queue to inspect
 * @return number of unread events, always in [0, QUEUE_CAPACITY].
 * Relies on unsigned wraparound so it stays correct even after head/tail wrap.
 */
uint32_t event_queue_count(const EventQueue *q) {
    return q->head - q->tail;
}

/*
 * Test whether the queue can accept another event.
 * @param q  queue to inspect
 * @return 1 if full, 0 otherwise.
 */
int event_queue_full(const EventQueue *q) {
    return event_queue_count(q) == QUEUE_CAPACITY;
}

/*
 * Append an event to the back of the queue (producer side).
 * @param q  destination queue
 * @param e  event to copy in
 * @return 1 on success, 0 if the queue was full and the event was dropped.
 * O(1). The event is copied by value, so the caller may reuse its storage.
 */
int event_queue_push(EventQueue *q, Event e) {
    if (event_queue_full(q)) {
        /* Drop on overflow rather than overwrite: losing the newest event is
         * usually less harmful than corrupting one the consumer is reading. */
        return 0;
    }
    q->slots[q->head & QUEUE_MASK] = e;
    q->head++;
    return 1;
}

/*
 * Remove and return the oldest event (consumer side).
 * @param q   source queue
 * @param out destination for the dequeued event (must be non-NULL)
 * @return 1 if an event was produced, 0 if the queue was empty.
 * O(1). On an empty queue *out is left untouched.
 */
int event_queue_pop(EventQueue *q, Event *out) {
    if (q->head == q->tail) {
        return 0; /* empty */
    }
    *out = q->slots[q->tail & QUEUE_MASK];
    q->tail++;
    return 1;
}

/*
 * Look at the oldest event without consuming it.
 * @param q   queue to peek
 * @param out destination for a copy of the front event
 * @return 1 if an event is available, 0 if empty.
 */
int event_queue_peek(const EventQueue *q, Event *out) {
    if (q->head == q->tail) {
        return 0;
    }
    *out = q->slots[q->tail & QUEUE_MASK];
    return 1;
}

/*
 * Drain every pending event, counting how many were of a given type.
 * @param q          queue to drain (left empty afterward)
 * @param wanted     event type to tally
 * @return number of drained events whose type == wanted.
 * Useful for collapsing, e.g., a burst of MOUSE_MOVE events into one update.
 */
uint32_t event_queue_drain_count(EventQueue *q, EventType wanted) {
    uint32_t matches = 0;
    Event e;
    while (event_queue_pop(q, &e)) {
        if (e.type == wanted) {
            matches++;
        }
    }
    return matches;
}
