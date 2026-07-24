#include <cstdio>
#include <cstdlib>

/* Circular ring-buffer queue modeling a print job scheduler. */
class CircularQueue {
public:
    static const int Capacity = 32;

    CircularQueue() : head(0), tail(0), size(0) {}

    bool isEmpty() const {
        return size == 0;
    }

    bool isFull() const {
        return size == Capacity;
    }

    bool enqueue(int jobId) {
        if (isFull()) {
            return false;
        }
        buffer[tail] = jobId;
        tail = (tail + 1) % Capacity;
        size = size + 1;
        return true;
    }

    bool dequeue(int &outJob) {
        if (isEmpty()) {
            return false;
        }
        outJob = buffer[head];
        head = (head + 1) % Capacity;
        size = size - 1;
        return true;
    }

    int front() const {
        if (isEmpty()) {
            return -1;
        }
        return buffer[head];
    }

    int countMatching(int target) const {
        int matches = 0;
        int index = head;
        int remaining = size;
        while (remaining > 0) {
            if (buffer[index] == target) {
                matches = matches + 1;
            }
            index = (index + 1) % Capacity;
            remaining = remaining - 1;
        }
        return matches;
    }

    int drainSum() {
        int total = 0;
        int job;
        while (dequeue(job)) {
            total = total + job;
        }
        return total;
    }

private:
    int buffer[Capacity];
    int head;
    int tail;
    int size;
};

int processBatch(int *jobIds, int length, int rejectAbove) {
    CircularQueue queue;
    int accepted = 0;
    for (int i = 0; i < length; i = i + 1) {
        if (jobIds[i] <= rejectAbove) {
            if (queue.enqueue(jobIds[i])) {
                accepted = accepted + 1;
            }
        }
    }
    return accepted + queue.drainSum();
}

int rotateQueue(CircularQueue &queue, int steps) {
    int moved = 0;
    for (int i = 0; i < steps; i = i + 1) {
        int value;
        if (!queue.dequeue(value)) {
            break;
        }
        queue.enqueue(value);
        moved = moved + 1;
    }
    return moved;
}
