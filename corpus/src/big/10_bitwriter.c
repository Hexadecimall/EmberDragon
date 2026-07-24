/*
 * MSB-first bit stream writer and reader.
 *
 * Entropy coders need to emit codes that are not byte-aligned, so this module
 * packs individual bits and small bit-fields into a byte buffer (and reads them
 * back) most-significant-bit first. The writer tracks a partial byte until it
 * fills, mirroring the bit ordering used by DEFLATE's Huffman blocks.
 */

#include <stdint.h>
#include <stddef.h>

/*
 * Output bit cursor over a caller-provided byte buffer.
 *
 * `bitPos` counts bits already written; `byteCap` bounds the buffer so the
 * writer can signal overflow instead of scribbling past the end.
 */
typedef struct {
    uint8_t *buffer;
    size_t   byteCap;
    size_t   bitPos;
} BitWriter;

/*
 * Input bit cursor over a byte buffer, the mirror image of BitWriter.
 */
typedef struct {
    const uint8_t *buffer;
    size_t         bitCount; /* total readable bits */
    size_t         bitPos;
} BitReader;

/*
 * Initialise a BitWriter over buffer with the given capacity in bytes.
 * The buffer is assumed pre-zeroed by the caller for clean unused tail bits.
 */
void bitWriterInit(BitWriter *w, uint8_t *buffer, size_t byteCap) {
    w->buffer = buffer;
    w->byteCap = byteCap;
    w->bitPos = 0;
}

/*
 * Append a single bit (the low bit of `bit`) to the stream.
 *
 * Bits land most-significant first within each byte: the first bit written to
 * a byte occupies value 0x80. Returns 1 on success, 0 if the buffer is full.
 * O(1).
 */
int bitWriterPut(BitWriter *w, uint32_t bit) {
    size_t byteIndex = w->bitPos >> 3;
    if (byteIndex >= w->byteCap) {
        return 0; /* No room for another bit. */
    }
    if (bit & 1u) {
        /* Set the bit at position 7,6,...,0 within the current byte. */
        unsigned shift = 7u - (unsigned)(w->bitPos & 7u);
        w->buffer[byteIndex] |= (uint8_t)(1u << shift);
    }
    w->bitPos++;
    return 1;
}

/*
 * Append the low `count` bits of `value`, most-significant of those first.
 *
 * count must be in 0..32. Emitting high-to-low keeps multi-bit codes in the
 * same order a reader will consume them. Returns 1 on success, 0 if any bit
 * could not be written (buffer full). O(count).
 */
int bitWriterPutBits(BitWriter *w, uint32_t value, unsigned count) {
    for (unsigned i = 0; i < count; i++) {
        unsigned shift = count - 1u - i; /* take the i-th bit from the top */
        if (!bitWriterPut(w, (value >> shift) & 1u)) {
            return 0;
        }
    }
    return 1;
}

/*
 * Number of whole bytes needed to hold everything written so far.
 *
 * Rounds the bit count up, since a trailing partial byte still occupies a full
 * byte of storage. O(1).
 */
size_t bitWriterByteLength(const BitWriter *w) {
    return (w->bitPos + 7) >> 3;
}

/*
 * Initialise a BitReader over buffer holding `bitCount` valid bits.
 */
void bitReaderInit(BitReader *r, const uint8_t *buffer, size_t bitCount) {
    r->buffer = buffer;
    r->bitCount = bitCount;
    r->bitPos = 0;
}

/*
 * Read the next bit, returning 0 or 1 in *outBit.
 *
 * Returns 1 on success, 0 once the stream is exhausted (in which case *outBit
 * is left untouched). O(1).
 */
int bitReaderGet(BitReader *r, uint32_t *outBit) {
    if (r->bitPos >= r->bitCount) {
        return 0; /* End of stream. */
    }
    size_t byteIndex = r->bitPos >> 3;
    unsigned shift = 7u - (unsigned)(r->bitPos & 7u);
    *outBit = (r->buffer[byteIndex] >> shift) & 1u;
    r->bitPos++;
    return 1;
}

/*
 * Read `count` bits (0..32) and assemble them into *outValue, MSB first.
 *
 * This is the inverse of bitWriterPutBits(). Returns 1 on success, 0 if the
 * stream ends before `count` bits are available. O(count).
 */
int bitReaderGetBits(BitReader *r, unsigned count, uint32_t *outValue) {
    uint32_t acc = 0;
    for (unsigned i = 0; i < count; i++) {
        uint32_t bit;
        if (!bitReaderGet(r, &bit)) {
            return 0;
        }
        /* Shift the accumulator up and OR the new low bit in. */
        acc = (acc << 1) | bit;
    }
    *outValue = acc;
    return 1;
}
