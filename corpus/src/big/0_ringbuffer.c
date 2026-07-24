/*
 * Fixed-capacity circular FIFO byte queue (ring buffer).
 *
 * A ring buffer stores a bounded stream of bytes in a single contiguous
 * array, wrapping the write and read positions around the end so no data is
 * ever shifted. It is the standard structure for producer/consumer pipes,
 * audio sample buffers, and serial I/O where the maximum backlog is known
 * ahead of time. All operations are O(1).
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 * Ring buffer state. `head` is the index of the oldest unread byte and
 * `tail` is the next slot to write. To distinguish a full ring from an empty
 * one (both would otherwise have head == tail) we track `count` explicitly.
 */
typedef struct {
    uint8_t *buffer;    /* backing storage of `capacity` bytes */
    uint32_t capacity;  /* total slots available */
    uint32_t head;      /* read cursor: oldest byte */
    uint32_t tail;      /* write cursor: next free slot */
    uint32_t count;     /* number of bytes currently stored */
} RingBuffer;

/*
 * Allocate a ring with room for `capacity` bytes.
 * @param rb        handle to initialize.
 * @param capacity  number of bytes the ring can hold; must be > 0.
 * @return 0 on success, -1 on invalid argument or allocation failure.
 */
int ring_init(RingBuffer *rb, uint32_t capacity) {
    if (capacity == 0) {
        return -1;  /* a zero-length ring can never accept data */
    }
    rb->buffer = (uint8_t *)malloc(capacity);
    if (rb->buffer == NULL) {
        return -1;
    }
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    return 0;
}

/* @return the number of bytes that can still be written before the ring fills. */
uint32_t ring_free_space(const RingBuffer *rb) {
    return rb->capacity - rb->count;
}

/*
 * Append one byte to the ring.
 * @return 0 on success, -1 if the ring is full (byte not stored).
 */
int ring_put(RingBuffer *rb, uint8_t byte) {
    if (rb->count == rb->capacity) {
        return -1;  /* full: refuse rather than overwrite unread data */
    }
    rb->buffer[rb->tail] = byte;
    /* Advance the write cursor, wrapping back to 0 at the end of the array. */
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count++;
    return 0;
}

/*
 * Remove and return the oldest byte.
 * @param out  receives the byte; must be non-NULL.
 * @return 0 on success, -1 if the ring is empty.
 */
int ring_get(RingBuffer *rb, uint8_t *out) {
    if (rb->count == 0) {
        return -1;  /* empty: nothing to read */
    }
    *out = rb->buffer[rb->head];
    rb->head = (rb->head + 1) % rb->capacity;  /* wrap the read cursor too */
    rb->count--;
    return 0;
}

/*
 * Bulk-write up to `len` bytes from `src`, stopping early if the ring fills.
 * @return the number of bytes actually written (0..len).
 * Useful when partial writes are acceptable and the caller will retry.
 */
uint32_t ring_write(RingBuffer *rb, const uint8_t *src, uint32_t len) {
    uint32_t written = 0;
    while (written < len && rb->count < rb->capacity) {
        rb->buffer[rb->tail] = src[written];
        rb->tail = (rb->tail + 1) % rb->capacity;
        rb->count++;
        written++;
    }
    return written;
}

/*
 * Bulk-read up to `len` bytes into `dst`, stopping when the ring empties.
 * @return the number of bytes actually copied out (0..len).
 */
uint32_t ring_read(RingBuffer *rb, uint8_t *dst, uint32_t len) {
    uint32_t read = 0;
    while (read < len && rb->count > 0) {
        dst[read] = rb->buffer[rb->head];
        rb->head = (rb->head + 1) % rb->capacity;
        rb->count--;
        read++;
    }
    return read;
}

/* Discard all buffered bytes without freeing the backing array. O(1). */
void ring_reset(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

/* Free the backing array and null out the handle's pointer. */
void ring_destroy(RingBuffer *rb) {
    free(rb->buffer);
    rb->buffer = NULL;
    rb->capacity = 0;
    rb->head = rb->tail = rb->count = 0;
}
