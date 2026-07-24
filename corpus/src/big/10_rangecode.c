/*
 * Static order-0 range coder.
 *
 * A range coder narrows an integer interval [low, low+range) according to each
 * symbol's cumulative frequency, emitting the high bytes of `low` as the range
 * shrinks. With an accurate frequency model it approaches the entropy bound
 * more closely than Huffman coding because it is not limited to whole-bit code
 * lengths. This implementation uses 32-bit state with byte-wise renormalisation.
 */

#include <stdint.h>
#include <stddef.h>

/* Renormalise whenever the range drops below 2^24 so a byte can be shipped. */
#define RC_TOP    0x01000000u
#define RC_BOTTOM 0x00010000u

/*
 * Encoder state plus its bounded output buffer.
 *
 * `low` is the 64-bit interval base (extra bits absorb carries); `range` is the
 * current interval width. `out`/`cap`/`pos` track emitted bytes.
 */
typedef struct {
    uint64_t low;
    uint32_t range;
    uint8_t *out;
    size_t   cap;
    size_t   pos;
} RangeEncoder;

/*
 * Initialise an encoder over a caller-provided output buffer.
 *
 * The range starts at the full 32-bit span; low starts at zero. O(1).
 */
void rangeEncoderInit(RangeEncoder *rc, uint8_t *out, size_t cap) {
    rc->low = 0;
    rc->range = 0xFFFFFFFFu;
    rc->out = out;
    rc->cap = cap;
    rc->pos = 0;
}

/*
 * Append one byte to the encoder output, ignoring writes past capacity.
 *
 * Overflow is silently dropped so a too-small buffer cannot corrupt memory; the
 * caller detects truncation by comparing pos against cap. O(1).
 */
static void rangeEmit(RangeEncoder *rc, uint8_t byte) {
    if (rc->pos < rc->cap) {
        rc->out[rc->pos] = byte;
    }
    rc->pos++;
}

/*
 * Push out settled high bytes once the interval is narrow enough.
 *
 * While the range is below RC_TOP, the top byte of `low` is fixed, so it is
 * emitted and both low and range are shifted up by eight bits. O(1) amortised.
 */
static void rangeEncodeRenorm(RangeEncoder *rc) {
    while (rc->range < RC_TOP) {
        rangeEmit(rc, (uint8_t)(rc->low >> 32)); /* high byte beyond 32 bits */
        rc->low = (rc->low << 8) & 0xFFFFFFFFFFull;
        rc->range <<= 8;
    }
}

/*
 * Encode one symbol given its cumulative-frequency interval.
 *
 * cumFreq  : sum of frequencies of all symbols before this one.
 * freq     : this symbol's frequency (must be > 0).
 * totFreq  : total of all frequencies (the model's denominator).
 *
 * The current range is partitioned in proportion to the model; the symbol's
 * slice becomes the new interval, then the coder renormalises. O(1) plus the
 * amortised renormalisation.
 */
void rangeEncode(RangeEncoder *rc, uint32_t cumFreq, uint32_t freq,
                 uint32_t totFreq) {
    /* Scale the interval down to one unit of total frequency. */
    uint32_t step = rc->range / totFreq;
    rc->low += (uint64_t)step * cumFreq;
    rc->range = step * freq;
    rangeEncodeRenorm(rc);
}

/*
 * Flush the remaining interval bytes, finishing the stream.
 *
 * Four bytes of `low` are emitted so the decoder can read a full word. Must be
 * called exactly once after the last symbol. Returns the total byte length the
 * encoder tried to write (which may exceed cap if it overflowed). O(1).
 */
size_t rangeEncoderFinish(RangeEncoder *rc) {
    for (int i = 0; i < 4; i++) {
        rangeEmit(rc, (uint8_t)(rc->low >> 32));
        rc->low = (rc->low << 8) & 0xFFFFFFFFFFull;
    }
    return rc->pos;
}

/*
 * Decoder state mirroring the encoder.
 *
 * `code` holds the portion of the encoded value currently under inspection.
 */
typedef struct {
    uint32_t range;
    uint32_t code;
    const uint8_t *in;
    size_t   len;
    size_t   pos;
} RangeDecoder;

/*
 * Read the next input byte, or 0 once the buffer is exhausted.
 *
 * Returning zero padding past the end matches the encoder's trailing flush and
 * keeps the arithmetic well defined. O(1).
 */
static uint8_t rangeReadByte(RangeDecoder *rc) {
    return rc->pos < rc->len ? rc->in[rc->pos++] : 0;
}

/*
 * Initialise a decoder, priming `code` with the first four input bytes.
 *
 * The four-byte prime mirrors rangeEncoderFinish()'s flush. O(1).
 */
void rangeDecoderInit(RangeDecoder *rc, const uint8_t *in, size_t len) {
    rc->range = 0xFFFFFFFFu;
    rc->in = in;
    rc->len = len;
    rc->pos = 0;
    rc->code = 0;
    for (int i = 0; i < 4; i++) {
        rc->code = (rc->code << 8) | rangeReadByte(rc);
    }
}

/*
 * Determine which symbol's interval the current code falls into.
 *
 * Returns a value in [0, totFreq) that the caller maps to a symbol by scanning
 * cumulative frequencies. This must be followed by rangeDecodeUpdate() with the
 * chosen symbol's interval. O(1).
 */
uint32_t rangeDecodeGetFreq(RangeDecoder *rc, uint32_t totFreq) {
    uint32_t step = rc->range / totFreq;
    uint32_t value = rc->code / step;
    /* Clamp: rounding can push value to totFreq, which has no symbol. */
    return value >= totFreq ? totFreq - 1 : value;
}

/*
 * Consume the selected symbol and renormalise the decoder.
 *
 * cumFreq/freq/totFreq match the values the encoder used for this symbol.
 * Mirrors the encoder's interval narrowing and byte intake. O(1) amortised.
 */
void rangeDecodeUpdate(RangeDecoder *rc, uint32_t cumFreq, uint32_t freq,
                       uint32_t totFreq) {
    uint32_t step = rc->range / totFreq;
    rc->code -= step * cumFreq;
    rc->range = step * freq;
    /* Pull in fresh bytes while the range is too narrow to distinguish symbols. */
    while (rc->range < RC_TOP) {
        rc->code = (rc->code << 8) | rangeReadByte(rc);
        rc->range <<= 8;
    }
}
