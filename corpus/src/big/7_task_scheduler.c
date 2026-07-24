/*
 * task_scheduler.c — A priority-ordered task queue backed by a binary min-heap.
 *
 * Tasks carry an integer priority (lower value = more urgent) and a monotonic
 * sequence number used to break ties in FIFO order, so two tasks of equal
 * priority dequeue in the order they were inserted. The heap gives O(log n)
 * push and pop while keeping the most urgent task at the root.
 */

#include <stdint.h>

#define MAX_TASKS 128

/* A schedulable unit of work. `seq` is assigned by the scheduler on push. */
typedef struct {
    int32_t  id;        /* caller-supplied task identifier      */
    int32_t  priority;  /* lower runs first                     */
    uint64_t seq;       /* insertion order, for stable tie-break */
} Task;

/* The scheduler: a heap array plus the next sequence number to hand out. */
typedef struct {
    Task     heap[MAX_TASKS];
    int      size;       /* number of tasks currently queued    */
    uint64_t next_seq;   /* monotonically increasing counter    */
} Scheduler;

/* Initialize an empty scheduler. */
void scheduler_init(Scheduler *s) {
    s->size     = 0;
    s->next_seq = 0;
}

/*
 * Order two tasks: returns 1 if `a` should run before `b`. Primary key is
 * priority (ascending); ties fall back to sequence number so earlier inserts
 * win, giving stable FIFO behavior within a priority class.
 */
static int task_less(const Task *a, const Task *b) {
    if (a->priority != b->priority)
        return a->priority < b->priority;
    return a->seq < b->seq;
}

/* Swap two heap slots in place. */
static void task_swap(Task *a, Task *b) {
    Task t = *a;
    *a = *b;
    *b = t;
}

/*
 * Restore the heap property by sifting the element at `i` upward toward the
 * root while it is more urgent than its parent. O(log n).
 */
static void sift_up(Scheduler *s, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (!task_less(&s->heap[i], &s->heap[parent]))
            break;                       /* parent already more urgent */
        task_swap(&s->heap[i], &s->heap[parent]);
        i = parent;
    }
}

/*
 * Restore the heap property by sifting the element at `i` downward, swapping
 * with its more-urgent child until both children are less urgent. O(log n).
 */
static void sift_down(Scheduler *s, int i) {
    for (;;) {
        int left   = 2 * i + 1;
        int right  = 2 * i + 2;
        int best   = i;
        if (left  < s->size && task_less(&s->heap[left],  &s->heap[best]))
            best = left;
        if (right < s->size && task_less(&s->heap[right], &s->heap[best]))
            best = right;
        if (best == i)
            break;                       /* heap property satisfied */
        task_swap(&s->heap[i], &s->heap[best]);
        i = best;
    }
}

/*
 * Enqueue a task with the given id and priority. The scheduler stamps it with
 * the next sequence number for tie-breaking. Returns 1 on success, 0 if the
 * heap is full. O(log n).
 */
int scheduler_push(Scheduler *s, int32_t id, int32_t priority) {
    if (s->size >= MAX_TASKS)
        return 0;
    Task *t = &s->heap[s->size];
    t->id       = id;
    t->priority = priority;
    t->seq      = s->next_seq++;
    sift_up(s, s->size);
    s->size++;
    return 1;
}

/*
 * Remove and return the most urgent task via `out`. Returns 1 on success, 0 if
 * the scheduler is empty (in which case `out` is untouched). The last element
 * is moved to the root and sifted down to repair the heap. O(log n).
 */
int scheduler_pop(Scheduler *s, Task *out) {
    if (s->size == 0)
        return 0;
    *out = s->heap[0];               /* root is always the most urgent */
    s->heap[0] = s->heap[--s->size]; /* promote the last leaf to the root */
    if (s->size > 0)
        sift_down(s, 0);
    return 1;
}

/*
 * Peek at the most urgent task without removing it. Returns a pointer to the
 * root task, or NULL if the scheduler is empty. The pointer is invalidated by
 * the next push or pop.
 */
const Task *scheduler_peek(const Scheduler *s) {
    if (s->size == 0)
        return 0;
    return &s->heap[0];
}

/* Return the number of tasks currently queued. */
int scheduler_size(const Scheduler *s) {
    return s->size;
}
