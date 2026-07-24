/*
 * leb128.cpp — LEB128 variable-length integer encoding (unsigned + signed).
 *
 * LEB128 stores integers in a sequence of bytes, seven payload bits each,
 * with the high bit acting as a continuation flag. It is the wire format used
 * by DWARF, WebAssembly, and Protocol Buffers' base varints. This module
 * encodes into and decodes from a growable byte stream object.
 */

#include <cstdint>
#include <vector>

/*
 * A simple append-and-read byte buffer used as the LEB128 transport. Writes
 * push onto the back; reads advance an internal cursor. Bounds are checked on
 * read so a truncated stream is reported rather than read past the end.
 */
struct ByteStream {
    std::vector<uint8_t> bytes; /* backing storage */
    size_t cursor = 0;          /* next byte to be read */
};

/* Mask selecting the 7 payload bits of a LEB128 byte. */
static const uint8_t PAYLOAD_MASK = 0x7F;
/* High bit set means "more bytes follow". */
static const uint8_t CONTINUE_BIT = 0x80;

/*
 * Encode an unsigned 64-bit integer into the stream as unsigned LEB128.
 * Emits 7 bits per byte, setting the continuation bit on every byte except
 * the last. Returns the number of bytes appended (1..10).
 */
size_t encode_uleb128(ByteStream &stream, uint64_t value) {
    size_t written = 0;
    do {
        uint8_t byte = (uint8_t)(value & PAYLOAD_MASK);
        value >>= 7;
        /* If anything remains, mark continuation and keep going. */
        if (value != 0)
            byte |= CONTINUE_BIT;
        stream.bytes.push_back(byte);
        written++;
    } while (value != 0);
    return written;
}

/*
 * Encode a signed 64-bit integer as signed LEB128. Uses arithmetic right
 * shift so the sign bit is replicated, and terminates only when the
 * remaining bits are all-zero (non-negative) or all-one (negative) AND the
 * sign bit of the last payload byte already carries the correct sign.
 * Returns the number of bytes appended.
 */
size_t encode_sleb128(ByteStream &stream, int64_t value) {
    size_t written = 0;
    bool more = true;
    while (more) {
        uint8_t byte = (uint8_t)(value & PAYLOAD_MASK);
        /* Arithmetic shift keeps the sign for negative numbers. */
        value >>= 7;
        /* The sixth bit (0x40) of the byte is the sign bit of this chunk. */
        int sign_bit_set = (byte & 0x40) != 0;
        /* We are done when the leftover value matches the sign already
         * encoded in this byte: 0 for positive, -1 for negative. */
        if ((value == 0 && !sign_bit_set) ||
            (value == -1 && sign_bit_set)) {
            more = false;
        } else {
            byte |= CONTINUE_BIT;
        }
        stream.bytes.push_back(byte);
        written++;
    }
    return written;
}

/*
 * Decode an unsigned LEB128 value from the stream into *out. Returns true on
 * success and advances the cursor past the value. Returns false (leaving the
 * cursor where it stalled) if the stream ends before the terminating byte.
 */
bool decode_uleb128(ByteStream &stream, uint64_t *out) {
    uint64_t result = 0;
    int shift = 0;
    while (stream.cursor < stream.bytes.size()) {
        uint8_t byte = stream.bytes[stream.cursor++];
        /* OR in the 7 payload bits at their proper position. */
        result |= (uint64_t)(byte & PAYLOAD_MASK) << shift;
        if ((byte & CONTINUE_BIT) == 0) {
            *out = result;
            return true; /* last byte reached */
        }
        shift += 7;
    }
    return false; /* ran out of bytes mid-value */
}

/*
 * Decode a signed LEB128 value from the stream into *out. Returns true on
 * success. After the final byte, if the chunk's sign bit is set and we have
 * not consumed all 64 bits, the high bits are filled with ones so the
 * negative value is sign-extended correctly.
 */
bool decode_sleb128(ByteStream &stream, int64_t *out) {
    uint64_t result = 0;
    int shift = 0;
    uint8_t byte = 0;
    while (stream.cursor < stream.bytes.size()) {
        byte = stream.bytes[stream.cursor++];
        result |= (uint64_t)(byte & PAYLOAD_MASK) << shift;
        shift += 7;
        if ((byte & CONTINUE_BIT) == 0) {
            /* Sign-extend if the value is negative and there are spare bits. */
            if (shift < 64 && (byte & 0x40))
                result |= ~(uint64_t)0 << shift;
            *out = (int64_t)result;
            return true;
        }
    }
    return false; /* truncated stream */
}
