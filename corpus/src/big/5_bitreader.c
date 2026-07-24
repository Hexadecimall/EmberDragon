/*
 * bitreader.c — MSB-first bit reader over a byte buffer.
 *
 * Many binary formats (JPEG, Deflate's Huffman tables, network protocols)
 * pack fields that are not byte-aligned. This reader walks a buffer one bit
 * at a time from the most significant bit of each byte, and can extract
 * multi-bit fields, align to the next byte, and report exhaustion.
 */

#include <stddef.h>
#include <stdint.h>

/*
 * Cursor over an immutable byte buffer. `bit_pos` is the absolute bit offset
 * from the start of the buffer; byte index is bit_pos/8 and the in-byte shift
 * is 7 - (bit_pos % 8) because bits are consumed most-significant first.
 */
typedef struct {
    const uint8_t *data; /* buffer being read (not owned) */
    size_t size;         /* buffer length in bytes */
    size_t bit_pos;      /* absolute position of the next bit to read */
} BitReader;

/*
 * Initialize a reader over `data` of `size` bytes, positioned at the very
 * first bit. The buffer is borrowed, not copied, and must outlive the reader.
 */
void bitreader_init(BitReader *r, const uint8_t *data, size_t size) {
    r->data = data;
    r->size = size;
    r->bit_pos = 0;
}

/*
 * Return the number of bits not yet consumed. Useful to check before a read
 * that might run past the end of the buffer.
 */
size_t bitreader_remaining(const BitReader *r) {
    size_t total_bits = r->size * 8;
    /* Guard against a cursor that was advanced past the end. */
    return (r->bit_pos >= total_bits) ? 0 : total_bits - r->bit_pos;
}

/*
 * Read a single bit and advance the cursor. Returns the bit value (0 or 1),
 * or -1 if the reader is already exhausted. Bits come out MSB-first within
 * each byte, matching big-endian bit packing.
 */
int bitreader_read_bit(BitReader *r) {
    if (r->bit_pos >= r->size * 8)
        return -1; /* no bits left */
    size_t byte_index = r->bit_pos >> 3;
    /* Bit 0 of bit_pos addresses the high bit of the byte, hence 7 - offset. */
    int shift = 7 - (int)(r->bit_pos & 7);
    int bit = (r->data[byte_index] >> shift) & 1;
    r->bit_pos++;
    return bit;
}

/*
 * Read `count` bits (0..32) as an unsigned integer, MSB-first, and store the
 * result in *out. Returns 1 on success, 0 if there are not enough bits left
 * or `count` exceeds 32. The first bit read becomes the most significant bit
 * of the result.
 */
int bitreader_read_bits(BitReader *r, int count, uint32_t *out) {
    if (count < 0 || count > 32)
        return 0;
    if (bitreader_remaining(r) < (size_t)count)
        return 0;

    uint32_t value = 0;
    for (int i = 0; i < count; i++) {
        int bit = bitreader_read_bit(r);
        /* Shift the accumulator up and drop the new bit into position 0. */
        value = (value << 1) | (uint32_t)bit;
    }
    *out = value;
    return 1;
}

/*
 * Advance the cursor to the next byte boundary, discarding 0..7 partial bits.
 * If already byte-aligned this is a no-op. Returns the number of bits skipped.
 */
size_t bitreader_align_to_byte(BitReader *r) {
    size_t offset = r->bit_pos & 7;
    if (offset == 0)
        return 0; /* already aligned */
    size_t skip = 8 - offset;
    r->bit_pos += skip;
    return skip;
}

/*
 * Read a whole byte starting from the current (not necessarily aligned)
 * position into *out. Returns 1 on success, 0 if fewer than 8 bits remain.
 * This is a thin wrapper that reuses the general multi-bit reader.
 */
int bitreader_read_byte(BitReader *r, uint8_t *out) {
    uint32_t value;
    if (!bitreader_read_bits(r, 8, &value))
        return 0;
    *out = (uint8_t)value;
    return 1;
}
