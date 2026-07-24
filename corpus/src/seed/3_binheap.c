#include <stdlib.h>

/* Binary min-heap used as a priority queue of scheduled tasks. */

typedef struct TaskHeap {
    int *priorities;
    int size;
    int capacity;
} TaskHeap;

static void siftUp(TaskHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->priorities[parent] <= heap->priorities[index]) {
            break;
        }
        int held = heap->priorities[parent];
        heap->priorities[parent] = heap->priorities[index];
        heap->priorities[index] = held;
        index = parent;
    }
}

static void siftDown(TaskHeap *heap, int index) {
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;
        if (left < heap->size &&
            heap->priorities[left] < heap->priorities[smallest]) {
            smallest = left;
        }
        if (right < heap->size &&
            heap->priorities[right] < heap->priorities[smallest]) {
            smallest = right;
        }
        if (smallest == index) {
            break;
        }
        int held = heap->priorities[smallest];
        heap->priorities[smallest] = heap->priorities[index];
        heap->priorities[index] = held;
        index = smallest;
    }
}

int pushTask(TaskHeap *heap, int priority) {
    if (heap->size >= heap->capacity) {
        return -1;
    }
    heap->priorities[heap->size] = priority;
    siftUp(heap, heap->size);
    heap->size = heap->size + 1;
    return 0;
}

int popTask(TaskHeap *heap, int *outPriority) {
    if (heap->size == 0) {
        return -1;
    }
    *outPriority = heap->priorities[0];
    heap->size = heap->size - 1;
    heap->priorities[0] = heap->priorities[heap->size];
    siftDown(heap, 0);
    return 0;
}

int peekTask(const TaskHeap *heap, int *outPriority) {
    if (heap->size == 0) {
        return -1;
    }
    *outPriority = heap->priorities[0];
    return 0;
}

int isHeapEmpty(const TaskHeap *heap) {
    return heap->size == 0 ? 1 : 0;
}
