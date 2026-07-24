/*
 * varint.c - LEB128 variable-length integer encoding, as used by Protocol
 * Buffers and DWARF. Each byte carries 7 payload bits plus a continuation
 * flag in the high bit; signed values are mapped through ZigZag so small
 * magnitudes stay short.
 */

#include <stdint.h>
#include <stddef.h>

/* High bit of each byte marks "more bytes follow". */
#define CONTINUATION_BIT 0x80
/* Low 7 bits of each byte carry payload. */
#define PAYLOAD_MASK     0x7F

/*
 * Encode an unsigned 64-bit value as unsigned LEB128 into 'buffer'.
 * 'bufferSize' is the available capacity. Returns the number of bytes written,
 * or -1 if the buffer is too small (a 64-bit value needs up to 10 bytes).
 */
int varint_encode_unsigned(uint64_t value, uint8_t *buffer, size_t bufferSize) {
    int written = 0;
    do {
        /* Take the low 7 bits as this byte's payload. */
        uint8_t byte = value & PAYLOAD_MASK;
        value >>= 7;
        /* If more nonzero bits remain, mark continuation on this byte. */
        if (value != 0)
            byte |= CONTINUATION_BIT;
        if ((size_t)written >= bufferSize)
            return -1;
        buffer[written++] = byte;
    } while (value != 0);
    return written;
}

/*
 * Decode an unsigned LEB128 value from 'buffer' of length 'bufferSize' into
 * '*value'. Returns the number of bytes consumed on success, or -1 if the
 * sequence runs past the end of the buffer or would shift beyond 64 bits.
 */
int varint_decode_unsigned(const uint8_t *buffer, size_t bufferSize, uint64_t *value) {
    uint64_t result = 0;
    int shift = 0;
    int consumed = 0;
    while ((size_t)consumed < bufferSize) {
        uint8_t byte = buffer[consumed++];
        /* A shift of 64 or more would drop bits; treat as malformed. */
        if (shift >= 64)
            return -1;
        result |= (uint64_t)(byte & PAYLOAD_MASK) << shift;
        /* Clear continuation bit means this is the final byte. */
        if ((byte & CONTINUATION_BIT) == 0) {
            *value = result;
            return consumed;
        }
        shift += 7;
    }
    /* Ran out of input while continuation was still expected. */
    return -1;
}

/*
 * Map a signed value to an unsigned one via ZigZag, so that small negative
 * numbers encode compactly. -1 -> 1, 1 -> 2, -2 -> 3, and so on. Returns the
 * unsigned encoding.
 */
uint64_t zigzag_encode(int64_t value) {
    /* Arithmetic right shift produces the sign mask; XOR folds negatives in. */
    return ((uint64_t)value << 1) ^ (uint64_t)(value >> 63);
}

/*
 * Invert zigzag_encode, recovering the original signed value from its
 * unsigned ZigZag form. Returns the signed value.
 */
int64_t zigzag_decode(uint64_t value) {
    /* Low bit holds the sign; recombine the shifted magnitude with it. */
    return (int64_t)(value >> 1) ^ -(int64_t)(value & 1);
}

/*
 * Encode a signed 64-bit value as a varint by ZigZag-mapping it first.
 * Returns the number of bytes written, or -1 if 'buffer' is too small.
 */
int varint_encode_signed(int64_t value, uint8_t *buffer, size_t bufferSize) {
    return varint_encode_unsigned(zigzag_encode(value), buffer, bufferSize);
}

/*
 * Decode a signed varint, undoing the ZigZag mapping after the raw decode.
 * Returns the number of bytes consumed, or -1 on a malformed or truncated
 * sequence.
 */
int varint_decode_signed(const uint8_t *buffer, size_t bufferSize, int64_t *value) {
    uint64_t raw = 0;
    int consumed = varint_decode_unsigned(buffer, bufferSize, &raw);
    if (consumed < 0)
        return -1;
    *value = zigzag_decode(raw);
    return consumed;
}
