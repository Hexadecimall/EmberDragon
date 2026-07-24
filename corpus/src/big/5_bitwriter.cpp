/*
 * bitwriter.cpp — MSB-first bit writer that packs fields into a byte vector.
 *
 * The encoding counterpart to a bit reader: callers push individual bits or
 * multi-bit fields and the writer accumulates them most-significant-bit first
 * into a growable buffer, flushing a completed byte whenever eight bits have
 * been gathered. A final pad flushes any partial trailing byte with zeros.
 */

#include <cstdint>
#include <vector>

/*
 * Accumulates bits into `bytes`. `pending` holds bits already collected for
 * the byte currently under construction, left-justified so the first bit
 * written sits in the high position. `pending_count` is how many of its 8
 * slots are filled (0..7); it never reaches 8 because the byte is flushed.
 */
struct BitWriter {
    std::vector<uint8_t> bytes; /* completed bytes */
    uint8_t pending = 0;        /* partially filled current byte */
    int pending_count = 0;      /* number of valid high bits in `pending` */
};

/*
 * Append a single bit (any nonzero `bit` counts as 1). The bit is shifted
 * into the next free position of the pending byte; once eight bits are
 * present the byte is committed to the buffer and the accumulator resets.
 */
void bitwriter_put_bit(BitWriter &w, int bit) {
    /* Make room at the bottom, then OR the new bit into position 0. */
    w.pending = (uint8_t)((w.pending << 1) | (bit ? 1u : 0u));
    w.pending_count++;
    if (w.pending_count == 8) {
        w.bytes.push_back(w.pending);
        w.pending = 0;
        w.pending_count = 0;
    }
}

/*
 * Write the low `count` bits of `value` (0..32 bits), most-significant bit
 * first, by delegating to bitwriter_put_bit. Out-of-range counts are clamped
 * to [0, 32] so the loop cannot read undefined high bits. Returns the number
 * of bits actually written.
 */
int bitwriter_put_bits(BitWriter &w, uint32_t value, int count) {
    if (count < 0) count = 0;
    if (count > 32) count = 32;
    for (int i = count - 1; i >= 0; i--) {
        /* Emit bit i, the (i+1)-th most significant of the requested field. */
        int bit = (int)((value >> i) & 1u);
        bitwriter_put_bit(w, bit);
    }
    return count;
}

/*
 * Total number of bits written so far, including any in the pending byte.
 * Useful for computing how much padding a flush will add.
 */
size_t bitwriter_bit_length(const BitWriter &w) {
    return w.bytes.size() * 8 + (size_t)w.pending_count;
}

/*
 * Flush the partially filled byte (if any) by padding the remaining low bits
 * with zeros and committing it. After this call the writer is byte-aligned
 * and `bytes` holds the complete output. Returns the number of pad bits added
 * (0 if already aligned).
 */
int bitwriter_flush(BitWriter &w) {
    if (w.pending_count == 0)
        return 0; /* nothing pending, already aligned */
    int pad = 8 - w.pending_count;
    /* Shift the collected bits up to the top of the byte; the freed low bits
     * are implicitly zero, which is exactly the desired padding. */
    w.pending = (uint8_t)(w.pending << pad);
    w.bytes.push_back(w.pending);
    w.pending = 0;
    w.pending_count = 0;
    return pad;
}

/*
 * Write a whole byte's worth of bits (8 bits, MSB first). Equivalent to
 * bitwriter_put_bits(w, value, 8) but named for clarity at call sites that
 * deal in bytes. Returns the number of bits written (always 8).
 */
int bitwriter_put_byte(BitWriter &w, uint8_t value) {
    return bitwriter_put_bits(w, value, 8);
}
