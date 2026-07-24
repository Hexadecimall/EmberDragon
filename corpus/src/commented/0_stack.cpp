/*
 * Fixed-capacity integer stack backed by a dynamically grown array.
 *
 * Implements the classic LIFO discipline: push appends to the top,
 * pop removes from the top. The backing buffer doubles on demand so
 * push is amortized O(1).
 */
#include <cstdlib>
#include <cstdint>

/*
 * A growable stack of 32-bit integers. `data` points at a heap buffer
 * of `capacity` slots, of which the lowest `size` are in use; the top
 * of the stack is the element at index size - 1.
 */
class IntStack {
public:
    /*
     * Construct an empty stack with the given starting capacity.
     * A zero or negative hint is clamped up to a minimum of 4 slots.
     */
    explicit IntStack(int initialCapacity) {
        if (initialCapacity < 4) {
            initialCapacity = 4;
        }
        capacity = initialCapacity;
        size = 0;
        data = (int32_t *)malloc(sizeof(int32_t) * capacity);
    }

    /* Release the backing buffer. */
    ~IntStack() {
        free(data);
    }

    /*
     * Push `value` onto the top of the stack, growing if full.
     * Returns true on success, false if a reallocation failed.
     */
    bool push(int32_t value) {
        if (size == capacity) {
            if (!grow()) {
                return false;
            }
        }
        data[size] = value;
        size++;
        return true;
    }

    /*
     * Pop the top element into `*out`. O(1).
     * Returns true if a value was removed, false if the stack was empty.
     */
    bool pop(int32_t *out) {
        if (size == 0) {
            return false; /* Underflow: nothing to remove. */
        }
        size--;
        *out = data[size];
        return true;
    }

    /*
     * Read the top element without removing it.
     * Returns true and writes `*out` if non-empty, false otherwise.
     */
    bool peek(int32_t *out) const {
        if (size == 0) {
            return false;
        }
        *out = data[size - 1];
        return true;
    }

    /* Return the number of elements currently stored. */
    int count() const {
        return size;
    }

    /* Return true when the stack holds no elements. */
    bool isEmpty() const {
        return size == 0;
    }

private:
    int32_t *data;
    int capacity;
    int size;

    /*
     * Double the backing buffer's capacity.
     * Returns true on success; on failure the old buffer is preserved.
     */
    bool grow() {
        int newCapacity = capacity * 2;
        int32_t *resized =
            (int32_t *)realloc(data, sizeof(int32_t) * newCapacity);
        if (resized == nullptr) {
            return false; /* Keep the existing buffer intact on OOM. */
        }
        data = resized;
        capacity = newCapacity;
        return true;
    }
};

/*
 * Reverse `array` of `length` elements in place using a scratch stack.
 * Each element is pushed then popped back in reverse order. O(n) time
 * and O(n) extra space. Does nothing for a length of zero or one.
 */
void reverseWithStack(int32_t *array, int length) {
    IntStack stack(length > 0 ? length : 4);
    for (int i = 0; i < length; i++) {
        stack.push(array[i]);
    }
    /* Popping yields elements in last-in-first-out (reversed) order. */
    for (int i = 0; i < length; i++) {
        int32_t value;
        stack.pop(&value);
        array[i] = value;
    }
}
