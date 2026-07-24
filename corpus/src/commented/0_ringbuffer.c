/*
 * Power-of-two ring buffer for bytes, used as a lossy or lossless
 * streaming FIFO.
 *
 * The capacity is rounded up to a power of two so index wrapping can
 * use a cheap bitmask instead of a modulo. Writes can either reject
 * overflow (writeStrict) or overwrite the oldest data (writeOverwrite).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Ring buffer state. `mask` equals capacity - 1, so `index & mask`
 * wraps an index into [0, capacity). Read and write counters are free-
 * running totals; their difference is the number of buffered bytes.
 */
typedef struct {
    uint8_t *data;
    uint32_t mask;       /* capacity - 1, valid because capacity is 2^k. */
    uint32_t readCount;  /* Total bytes ever read. */
    uint32_t writeCount; /* Total bytes ever written. */
} RingBuffer;

/*
 * Round `n` up to the nearest power of two (minimum 1).
 * Returns the smallest 2^k >= n. Used to size the backing array.
 */
static uint32_t roundUpPow2(uint32_t n) {
    if (n < 2) {
        return 1;
    }
    /* Smear the highest set bit down across all lower bits, then +1. */
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/*
 * Initialize `rb` with capacity at least `requested` bytes.
 * Returns 1 on success, 0 on allocation failure. On success the buffer
 * owns a heap array that ringFree() must later release.
 */
int ringInit(RingBuffer *rb, uint32_t requested) {
    uint32_t capacity = roundUpPow2(requested);
    rb->data = (uint8_t *)malloc(capacity);
    if (rb->data == NULL) {
        return 0;
    }
    rb->mask = capacity - 1;
    rb->readCount = 0;
    rb->writeCount = 0;
    return 1;
}

/* Return the number of bytes currently available to read. */
uint32_t ringSize(const RingBuffer *rb) {
    return rb->writeCount - rb->readCount;
}

/* Return the total capacity in bytes. */
uint32_t ringCapacity(const RingBuffer *rb) {
    return rb->mask + 1;
}

/* Return the number of free byte slots. */
uint32_t ringFree(const RingBuffer *rb) {
    return ringCapacity(rb) - ringSize(rb);
}

/*
 * Write one byte, refusing to overrun unread data.
 * Returns 1 if stored, 0 if the buffer was full (byte dropped).
 */
int ringWriteStrict(RingBuffer *rb, uint8_t byte) {
    if (ringSize(rb) == ringCapacity(rb)) {
        return 0; /* Full: do not clobber unread bytes. */
    }
    rb->data[rb->writeCount & rb->mask] = byte;
    rb->writeCount++;
    return 1;
}

/*
 * Write one byte, overwriting the oldest unread byte if the buffer is
 * full. Always succeeds. When overwriting, the read cursor is advanced
 * so the reported size stays at capacity rather than appearing to grow.
 */
void ringWriteOverwrite(RingBuffer *rb, uint8_t byte) {
    if (ringSize(rb) == ringCapacity(rb)) {
        rb->readCount++; /* Discard the oldest byte to make room. */
    }
    rb->data[rb->writeCount & rb->mask] = byte;
    rb->writeCount++;
}

/*
 * Read one byte into `*out`.
 * Returns 1 on success, 0 if the buffer was empty.
 */
int ringRead(RingBuffer *rb, uint8_t *out) {
    if (ringSize(rb) == 0) {
        return 0;
    }
    *out = rb->data[rb->readCount & rb->mask];
    rb->readCount++;
    return 1;
}

/*
 * Copy up to `maxLen` bytes into `dst` without consuming them.
 * Returns the number of bytes actually copied (min of available and
 * maxLen). The read cursor is left unchanged.
 */
uint32_t ringPeekBlock(const RingBuffer *rb, uint8_t *dst, uint32_t maxLen) {
    uint32_t available = ringSize(rb);
    uint32_t toCopy = (maxLen < available) ? maxLen : available;
    for (uint32_t i = 0; i < toCopy; i++) {
        /* Offset the running read counter, then wrap with the mask. */
        dst[i] = rb->data[(rb->readCount + i) & rb->mask];
    }
    return toCopy;
}

/*
 * Discard all buffered data, keeping the allocation.
 * Resets the counters so the buffer is logically empty.
 */
void ringClear(RingBuffer *rb) {
    rb->readCount = 0;
    rb->writeCount = 0;
}

/*
 * Release the backing array. After this the buffer must be re-init'd
 * before reuse. Safe on a zeroed/NULL-data struct.
 */
void ringDestroy(RingBuffer *rb) {
    free(rb->data);
    rb->data = NULL;
    rb->mask = 0;
    rb->readCount = 0;
    rb->writeCount = 0;
}
