/*
 * ZigZag variable-length integer encoding.
 *
 * ZigZag maps signed integers onto unsigned ones so that small-magnitude
 * values (positive or negative) become small unsigned values, which then pack
 * efficiently into a base-128 varint. This is the same scheme Protocol Buffers
 * use for sint32/sint64 fields, reproduced here in plain integer arithmetic.
 */

#include <stdint.h>
#include <stddef.h>

/*
 * Fold a signed 64-bit value into an unsigned one with ZigZag.
 *
 * The transform interleaves positive and negative numbers: 0->0, -1->1, 1->2,
 * -2->3, ... so the magnitude of the result tracks the magnitude of the input.
 * The arithmetic right shift relies on sign extension, which is the intended
 * (and portable in practice) behaviour for this idiom.
 *
 * Returns the ZigZag-encoded unsigned value. O(1).
 */
uint64_t zigzagEncode(int64_t value) {
    return ((uint64_t)value << 1) ^ (uint64_t)(value >> 63);
}

/*
 * Invert zigzagEncode(), recovering the original signed value.
 *
 * The low bit selects the sign; shifting right undoes the interleave. O(1).
 */
int64_t zigzagDecode(uint64_t encoded) {
    return (int64_t)(encoded >> 1) ^ -(int64_t)(encoded & 1);
}

/*
 * Write an unsigned value as a base-128 varint into out.
 *
 * Seven bits are emitted per byte, low group first; the high bit of each byte
 * is a continuation flag set on every byte except the last. The caller must
 * provide at least 10 bytes of space (the worst case for a 64-bit value).
 *
 * Returns the number of bytes written, between 1 and 10. O(log value).
 */
size_t varintWrite(uint64_t value, uint8_t *out) {
    size_t n = 0;
    /* Peel off 7 bits at a time until no significant bits remain. */
    while (value >= 0x80) {
        out[n++] = (uint8_t)(value | 0x80); /* Continuation bit set. */
        value >>= 7;
    }
    out[n++] = (uint8_t)value; /* Final byte: high bit clear. */
    return n;
}

/*
 * Read a base-128 varint from in, reversing varintWrite().
 *
 * in/inLen  : source buffer and its length.
 * value     : out-parameter receiving the decoded unsigned value.
 * Returns the number of bytes consumed, or 0 if the buffer ends mid-varint or
 * the value exceeds 64 bits (malformed input). O(log value).
 */
size_t varintRead(const uint8_t *in, size_t inLen, uint64_t *value) {
    uint64_t result = 0;
    int shift = 0;
    size_t i = 0;
    while (i < inLen) {
        uint8_t byte = in[i++];
        /* Accumulate the low 7 bits at the current position. */
        result |= (uint64_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            *value = result;
            return i; /* Continuation bit clear marks the end. */
        }
        shift += 7;
        if (shift >= 64) {
            return 0; /* More than ten groups cannot fit in 64 bits. */
        }
    }
    return 0; /* Ran out of bytes before the terminating byte. */
}

/*
 * Encode an array of signed integers as a stream of ZigZag varints.
 *
 * values/count : the integers to encode.
 * out/outCap   : destination buffer and capacity.
 * Returns the number of bytes written, or (size_t)-1 if out runs out of space.
 * O(count) varint conversions.
 */
size_t encodeSignedStream(const int64_t *values, size_t count,
                          uint8_t *out, size_t outCap) {
    size_t pos = 0;
    for (size_t i = 0; i < count; i++) {
        /* Each value needs at most 10 bytes; verify before writing. */
        if (pos + 10 > outCap) {
            return (size_t)-1;
        }
        pos += varintWrite(zigzagEncode(values[i]), out + pos);
    }
    return pos;
}
