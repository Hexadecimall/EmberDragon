/*
 * priority_event_queue.c
 *
 * A binary-heap priority queue for timer/event scheduling. Events are ordered
 * by their scheduled fire time so the soonest-due event is always at the root,
 * giving O(log n) insertion and O(log n) extraction of the minimum. This is the
 * core data structure behind a discrete-event simulation or a timer wheel's
 * overflow list.
 */

#include <stdint.h>

/* Maximum number of pending events the scheduler can hold. */
#define HEAP_CAPACITY 256

/* A scheduled event: it should fire at `deadline`, and `id` identifies which
 * callback/timer it belongs to. Smaller `deadline` means higher priority. */
typedef struct {
    uint64_t deadline;
    int32_t  id;
} TimerEvent;

/* A min-heap stored in the usual implicit-tree array layout: the children of
 * node i live at 2*i+1 and 2*i+2, and its parent at (i-1)/2. */
typedef struct {
    TimerEvent nodes[HEAP_CAPACITY];
    int        size;
} EventHeap;

/*
 * Initialize an empty heap.
 * @param h  heap to reset
 */
void event_heap_init(EventHeap *h) {
    h->size = 0;
}

/* Swap two heap slots. Internal helper. */
static void heap_swap(EventHeap *h, int a, int b) {
    TimerEvent t = h->nodes[a];
    h->nodes[a]  = h->nodes[b];
    h->nodes[b]  = t;
}

/*
 * Restore the heap property by moving node `i` toward the root.
 * @param h  heap to fix
 * @param i  index of the node that may be smaller than its parent
 * Used after an insertion. O(log n).
 */
static void heap_sift_up(EventHeap *h, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        /* Stop once the parent is no later than this node: the min-heap
         * ordering (parent.deadline <= child.deadline) is satisfied. */
        if (h->nodes[parent].deadline <= h->nodes[i].deadline) {
            break;
        }
        heap_swap(h, i, parent);
        i = parent;
    }
}

/*
 * Restore the heap property by moving node `i` toward the leaves.
 * @param h  heap to fix
 * @param i  index of the node that may be larger than a child
 * Used after removing the root. O(log n).
 */
static void heap_sift_down(EventHeap *h, int i) {
    for (;;) {
        int left     = 2 * i + 1;
        int right    = 2 * i + 2;
        int smallest = i;

        /* Pick the smaller of this node and its existing children. */
        if (left < h->size &&
            h->nodes[left].deadline < h->nodes[smallest].deadline) {
            smallest = left;
        }
        if (right < h->size &&
            h->nodes[right].deadline < h->nodes[smallest].deadline) {
            smallest = right;
        }
        if (smallest == i) {
            break; /* node is already <= both children; done */
        }
        heap_swap(h, i, smallest);
        i = smallest;
    }
}

/*
 * Schedule an event by inserting it into the heap.
 * @param h         heap to insert into
 * @param deadline  absolute time the event should fire
 * @param id        opaque event identifier
 * @return 1 on success, 0 if the heap was full.
 * O(log n).
 */
int event_heap_schedule(EventHeap *h, uint64_t deadline, int32_t id) {
    if (h->size >= HEAP_CAPACITY) {
        return 0;
    }
    int i = h->size++;
    h->nodes[i].deadline = deadline;
    h->nodes[i].id       = id;
    heap_sift_up(h, i); /* bubble the newcomer up to its proper depth */
    return 1;
}

/*
 * Inspect the soonest-due event without removing it.
 * @param h    heap to peek
 * @param out  destination for the minimum event
 * @return 1 if an event exists, 0 if the heap is empty.
 */
int event_heap_peek(const EventHeap *h, TimerEvent *out) {
    if (h->size == 0) {
        return 0;
    }
    *out = h->nodes[0]; /* the root is always the minimum */
    return 1;
}

/*
 * Remove and return the soonest-due event.
 * @param h    heap to pop from
 * @param out  destination for the extracted event
 * @return 1 if an event was extracted, 0 if the heap was empty.
 * Replaces the root with the last element and sifts it down. O(log n).
 */
int event_heap_pop(EventHeap *h, TimerEvent *out) {
    if (h->size == 0) {
        return 0;
    }
    *out = h->nodes[0];
    h->size--;
    if (h->size > 0) {
        /* Move the last leaf to the root and restore order downward. */
        h->nodes[0] = h->nodes[h->size];
        heap_sift_down(h, 0);
    }
    return 1;
}

/*
 * Pop every event that is due at or before a cutoff time.
 * @param h       heap to drain from
 * @param now     current time; events with deadline <= now have expired
 * @param fired   output array receiving the expired events in fire order
 * @param max     capacity of `fired`
 * @return number of events written to `fired` (stops early if `max` is hit).
 * Because the heap yields events in deadline order, the first one not yet due
 * lets us stop immediately.
 */
int event_heap_pop_expired(EventHeap *h, uint64_t now,
                           TimerEvent *fired, int max) {
    int n = 0;
    while (n < max && h->size > 0 && h->nodes[0].deadline <= now) {
        event_heap_pop(h, &fired[n]);
        n++;
    }
    return n;
}
