#include <stdint.h>

#define QUEUE_CAPACITY 256

typedef enum {
    PRIORITY_LOW = 0,
    PRIORITY_STANDARD = 1,
    PRIORITY_RUSH = 2
} OrderPriority;

typedef struct {
    int32_t order_id;
    int32_t customer_id;
    OrderPriority priority;
    int32_t item_count;
    int64_t total_cents;
} Order;

typedef struct {
    Order ring[QUEUE_CAPACITY];
    int32_t front;
    int32_t back;
    int32_t size;
} OrderQueue;

void queue_init(OrderQueue *queue) {
    queue->front = 0;
    queue->back = 0;
    queue->size = 0;
}

int queue_is_full(const OrderQueue *queue) {
    return queue->size == QUEUE_CAPACITY ? 1 : 0;
}

int queue_is_empty(const OrderQueue *queue) {
    return queue->size == 0 ? 1 : 0;
}

int enqueue_order(OrderQueue *queue, Order order) {
    if (queue_is_full(queue)) {
        return -1;
    }
    queue->ring[queue->back] = order;
    queue->back = (queue->back + 1) % QUEUE_CAPACITY;
    queue->size++;
    return 0;
}

int dequeue_order(OrderQueue *queue, Order *out) {
    if (queue_is_empty(queue)) {
        return -1;
    }
    *out = queue->ring[queue->front];
    queue->front = (queue->front + 1) % QUEUE_CAPACITY;
    queue->size--;
    return 0;
}

int peek_highest_priority(OrderQueue *queue, Order *out) {
    if (queue_is_empty(queue)) {
        return -1;
    }
    int32_t idx = queue->front;
    int32_t best = idx;
    for (int32_t n = 0; n < queue->size; n++) {
        if (queue->ring[idx].priority > queue->ring[best].priority) {
            best = idx;
        }
        idx = (idx + 1) % QUEUE_CAPACITY;
    }
    *out = queue->ring[best];
    return 0;
}

int64_t pending_revenue(const OrderQueue *queue) {
    int64_t total = 0;
    int32_t idx = queue->front;
    for (int32_t n = 0; n < queue->size; n++) {
        total += queue->ring[idx].total_cents;
        idx = (idx + 1) % QUEUE_CAPACITY;
    }
    return total;
}

int count_by_priority(const OrderQueue *queue, OrderPriority priority) {
    int count = 0;
    int32_t idx = queue->front;
    for (int32_t n = 0; n < queue->size; n++) {
        if (queue->ring[idx].priority == priority) {
            count++;
        }
        idx = (idx + 1) % QUEUE_CAPACITY;
    }
    return count;
}
