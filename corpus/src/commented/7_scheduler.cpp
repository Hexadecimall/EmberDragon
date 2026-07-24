/*
 * scheduler.cpp
 *
 * A priority-based task scheduler backed by a binary min-heap keyed on each
 * task's deadline. Tasks are dispatched earliest-deadline-first. The heap is
 * stored in a fixed-capacity array, so all operations avoid dynamic
 * allocation and run in O(log n) per insert/extract.
 */

#include <cstdint>
#include <cstring>

static const int MAX_TASKS = 128;

/* A unit of work. Smaller `deadline` means more urgent; `id` identifies the
 * task to the caller, and `cost` is an opaque service-time estimate in ticks. */
struct Task {
    uint32_t id;
    uint32_t deadline;  /* absolute tick by which the task should run */
    uint32_t cost;      /* estimated execution cost in ticks          */
};

/* A bounded min-heap of pending tasks ordered by deadline. The root (index 0)
 * is always the most urgent task. `size` is the number of live entries. */
struct Scheduler {
    Task heap[MAX_TASKS];
    int size;
};

/*
 * Initialize an empty scheduler.
 * Parameters: s - scheduler to reset (non-NULL). Returns nothing.
 */
void scheduler_init(Scheduler *s) {
    s->size = 0;
}

/*
 * Swap two task slots in the heap array.
 * Parameters: s, a, b - indices to exchange. Returns nothing.
 */
static void scheduler_swap(Scheduler *s, int a, int b) {
    Task tmp = s->heap[a];
    s->heap[a] = s->heap[b];
    s->heap[b] = tmp;
}

/*
 * Restore the heap property by moving the element at `i` up toward the root.
 * Parameters: s, i - index of the possibly-misplaced element.
 * Used after an insertion at the end of the array. O(log n).
 */
static void scheduler_sift_up(Scheduler *s, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        /* Stop once the child is no more urgent than its parent. */
        if (s->heap[i].deadline >= s->heap[parent].deadline)
            break;
        scheduler_swap(s, i, parent);
        i = parent;
    }
}

/*
 * Restore the heap property by moving the element at `i` down toward leaves.
 * Parameters: s, i - index of the possibly-misplaced element.
 * Used after the root is removed and replaced by the last element. O(log n).
 */
static void scheduler_sift_down(Scheduler *s, int i) {
    for (;;) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        /* Pick the most urgent of node, left child, and right child. */
        if (left < s->size &&
            s->heap[left].deadline < s->heap[smallest].deadline)
            smallest = left;
        if (right < s->size &&
            s->heap[right].deadline < s->heap[smallest].deadline)
            smallest = right;
        if (smallest == i)
            break;                      /* heap property already holds */
        scheduler_swap(s, i, smallest);
        i = smallest;
    }
}

/*
 * Schedule a task, inserting it into the heap by deadline.
 * Parameters: s, id, deadline, cost - the task fields.
 * Returns 1 on success, 0 if the scheduler is at capacity. O(log n).
 */
int scheduler_push(Scheduler *s, uint32_t id, uint32_t deadline, uint32_t cost) {
    if (s->size >= MAX_TASKS)
        return 0;
    int i = s->size++;
    s->heap[i].id = id;
    s->heap[i].deadline = deadline;
    s->heap[i].cost = cost;
    scheduler_sift_up(s, i);            /* bubble the new task into place */
    return 1;
}

/*
 * Remove and return the most urgent task (earliest deadline).
 * Parameters: s - scheduler; out - receives the popped task (non-NULL).
 * Returns 1 if a task was returned, 0 if the scheduler was empty. O(log n).
 */
int scheduler_pop(Scheduler *s, Task *out) {
    if (s->size == 0)
        return 0;
    *out = s->heap[0];                  /* root is the earliest deadline */
    s->size--;
    if (s->size > 0) {
        /* Move the last element to the root and sink it to its place. */
        s->heap[0] = s->heap[s->size];
        scheduler_sift_down(s, 0);
    }
    return 1;
}

/*
 * Peek at the most urgent deadline without removing anything.
 * Parameters: s - scheduler.
 * Returns the root task's deadline, or UINT32_MAX if the scheduler is empty.
 */
uint32_t scheduler_peek_deadline(const Scheduler *s) {
    if (s->size == 0)
        return UINT32_MAX;              /* sentinel: nothing pending */
    return s->heap[0].deadline;
}

/*
 * Sum the estimated cost of every pending task.
 * Parameters: s - scheduler.
 * Returns total queued service time in ticks. O(n).
 */
uint64_t scheduler_total_cost(const Scheduler *s) {
    uint64_t total = 0;
    for (int i = 0; i < s->size; i++)
        total += s->heap[i].cost;
    return total;
}
