/*
 * Dynamic-array-backed stack of integers with amortized O(1) push.
 *
 * This is a classic LIFO container implemented over a manually managed,
 * geometrically growing buffer. Doubling the capacity on overflow keeps the
 * average cost of a push constant even though individual pushes occasionally
 * pay for a reallocation and copy.
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>

/*
 * A growable last-in/first-out stack of 64-bit signed integers.
 * Invariant: 0 <= count <= capacity, and `data` holds `capacity` slots
 * (or is null only while capacity is zero).
 */
class IntStack {
public:
    /* Construct an empty stack with no backing allocation yet. */
    IntStack() : data_(nullptr), count_(0), capacity_(0) {}

    /* Release the backing buffer. */
    ~IntStack() {
        free(data_);
    }

    /*
     * Push `value` onto the top of the stack.
     * Grows the buffer (doubling, starting at 8) when full.
     * @return true on success, false if a reallocation failed.
     * Amortized O(1).
     */
    bool push(int64_t value) {
        if (count_ == capacity_) {
            /* Pick the next capacity: 8 for the first growth, else double. */
            int32_t new_cap = (capacity_ == 0) ? 8 : capacity_ * 2;
            int64_t *grown =
                (int64_t *)realloc(data_, (size_t)new_cap * sizeof(int64_t));
            if (grown == nullptr) {
                return false;  /* original buffer is still valid and intact */
            }
            data_ = grown;
            capacity_ = new_cap;
        }
        data_[count_] = value;  /* the top always lives at index count_ */
        count_++;
        return true;
    }

    /*
     * Pop the top element.
     * @param out  receives the removed value if non-null.
     * @return true if an element was removed, false if the stack was empty.
     */
    bool pop(int64_t *out) {
        if (count_ == 0) {
            return false;  /* underflow guard */
        }
        count_--;
        if (out != nullptr) {
            *out = data_[count_];
        }
        return true;
    }

    /*
     * Read the top element without removing it.
     * @param out  receives the top value.
     * @return true if the stack is non-empty, false otherwise.
     */
    bool peek(int64_t *out) const {
        if (count_ == 0) {
            return false;
        }
        *out = data_[count_ - 1];
        return true;
    }

    /* @return the number of elements currently held. */
    int32_t size() const {
        return count_;
    }

    /* @return true when the stack holds no elements. */
    bool empty() const {
        return count_ == 0;
    }

    /*
     * Logically clear the stack without shrinking the buffer, so subsequent
     * pushes can reuse the existing capacity. O(1).
     */
    void clear() {
        count_ = 0;
    }

private:
    int64_t *data_;     /* heap buffer of `capacity_` slots */
    int32_t  count_;    /* number of live elements; top is at count_ - 1 */
    int32_t  capacity_; /* allocated slot count */
};
