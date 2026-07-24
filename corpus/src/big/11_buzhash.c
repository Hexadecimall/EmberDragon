/*
 * buzhash.c — a cyclic-polynomial rolling hash (buzhash) and content-defined
 * chunking.
 *
 * Buzhash builds a rolling hash from a fixed table of random 32-bit values, one
 * per possible byte. Adding a byte rotates the accumulator and XORs in that
 * byte's table value; removing the oldest byte XORs in its value pre-rotated by
 * the window length. Because it relies only on rotates and XORs it is very fast,
 * and its uniform low bits make it ideal for content-defined chunking, where a
 * data stream is split at byte boundaries whose hash matches a mask.
 */

#include <stdint.h>
#include <stddef.h>

/* Word size of the hash, used for the rotate amount wraparound. */
#define BUZ_BITS 32

/* A 256-entry table of pseudo-random words, one per byte value. It is seeded
 * deterministically so that the same data always hashes the same way. */
static uint32_t buz_table[256];
static int buz_table_ready = 0;

/* Rolling hash state for a window of fixed length over a byte stream. */
typedef struct BuzHash {
    uint32_t hash;       /* current accumulator over the window */
    size_t   window_len; /* number of bytes the window covers */
    uint32_t rot;        /* window_len mod 32: the cumulative rotation */
} BuzHash;

/*
 * Rotate a 32-bit value left by r bits.
 *
 * r is taken modulo 32 so any shift amount is safe (a shift of 32 would be
 * undefined). Returns the rotated value. O(1).
 */
static uint32_t buz_rotl(uint32_t x, uint32_t r) {
    r &= (BUZ_BITS - 1);
    if (r == 0) return x; /* avoid the undefined 32-bit shift in the else path */
    return (x << r) | (x >> (BUZ_BITS - r));
}

/*
 * Fill buz_table with deterministic pseudo-random words on first use.
 *
 * We use a small xorshift32 generator seeded with a fixed constant so the table
 * is reproducible across runs and machines. Idempotent; O(256).
 */
static void buz_init_table(void) {
    uint32_t state = 0x1F2D3C4Bu; /* fixed seed for reproducibility */
    for (int i = 0; i < 256; i++) {
        /* xorshift32: cheap, full-period generator, good enough for table fill */
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        buz_table[i] = state;
    }
    buz_table_ready = 1;
}

/*
 * Initialize a buzhash over the first window_len bytes of data.
 *
 * bh:         state to populate.
 * data:       buffer of at least window_len bytes.
 * window_len: fixed window width (>= 1).
 * Each byte contributes its table value rotated by its age, so the most recent
 * byte is unrotated and the oldest is rotated by window_len-1. O(window_len).
 */
void buzhash_init(BuzHash *bh, const uint8_t *data, size_t window_len) {
    if (!buz_table_ready) buz_init_table();
    bh->window_len = window_len;
    bh->rot = (uint32_t)(window_len % BUZ_BITS);

    uint32_t h = 0;
    for (size_t i = 0; i < window_len; i++) {
        /* The byte at position i has (window_len-1-i) newer bytes after it,
         * so it must be rotated left that many times. */
        h ^= buz_rotl(buz_table[data[i]], (uint32_t)(window_len - 1 - i));
    }
    bh->hash = h;
}

/*
 * Slide the window forward by one byte.
 *
 * Removes old_byte (leaving on the left) and appends new_byte (entering on the
 * right) in O(1). The whole accumulator is rotated left by one — aging every
 * retained byte — and the leaving byte's contribution, pre-rotated by the full
 * window length, is XORed out.
 *
 * bh:       state from buzhash_init.
 * old_byte: byte exiting the window.
 * new_byte: byte entering the window.
 */
void buzhash_roll(BuzHash *bh, uint8_t old_byte, uint8_t new_byte) {
    /* The exiting byte was rotated by (window_len-1) when it last entered; after
     * the upcoming whole-accumulator rotate it would be at window_len, so cancel
     * it at exactly that rotation. */
    uint32_t leaving = buz_rotl(buz_table[old_byte], bh->rot);
    bh->hash = buz_rotl(bh->hash, 1) ^ leaving ^ buz_table[new_byte];
}

/*
 * Return the current hash of the window. O(1).
 */
uint32_t buzhash_value(const BuzHash *bh) {
    return bh->hash;
}

/*
 * Find the next content-defined chunk boundary in a byte stream.
 *
 * Starting just past a previous boundary, this slides a window forward until
 * the rolling hash has its low `mask_bits` bits all zero, which happens on
 * average every 2^mask_bits bytes regardless of how the data is shifted — the
 * key property that makes deduplication robust to insertions.
 *
 * data:       the stream.
 * len:        total length of the stream.
 * start:      offset to begin searching from.
 * window_len: rolling-window width.
 * mask_bits:  number of low bits that must be zero to declare a boundary.
 * Returns the offset just after the byte that triggered the boundary, or len if
 * no boundary is found before the end of the stream. O(len - start).
 */
size_t buzhash_next_boundary(const uint8_t *data, size_t len, size_t start,
                             size_t window_len, unsigned mask_bits) {
    if (start + window_len > len) return len; /* not enough bytes for a window */

    uint32_t mask = (mask_bits >= 32) ? 0xFFFFFFFFu : ((1u << mask_bits) - 1u);

    BuzHash bh;
    buzhash_init(&bh, data + start, window_len);

    size_t pos = start + window_len; /* index of the next byte to feed in */
    if ((bh.hash & mask) == 0) {
        return pos; /* boundary already at the end of the very first window */
    }
    while (pos < len) {
        buzhash_roll(&bh, data[pos - window_len], data[pos]);
        pos++;
        if ((bh.hash & mask) == 0) return pos; /* cut point found */
    }
    return len; /* stream ended without another boundary */
}
