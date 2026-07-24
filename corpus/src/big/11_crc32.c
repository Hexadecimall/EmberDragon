/*
 * crc32.c — table-driven CRC-32 (the IEEE 802.3 / zip / PNG polynomial).
 *
 * A cyclic redundancy check treats the message as the coefficients of a
 * polynomial over GF(2) and computes its remainder modulo a fixed generator
 * polynomial; the remainder is the checksum. This implementation uses the
 * reflected (bit-reversed) CRC-32 convention shared by zlib, PNG, and zip, and
 * builds its 256-entry lookup table lazily on first use for byte-at-a-time speed.
 */

#include <stdint.h>
#include <stddef.h>

/* The reflected form of the standard CRC-32 generator polynomial 0x04C11DB7.
 * Reflection lets us process bits from least-significant upward, which matches
 * how serial hardware and the zlib convention shift data. */
#define CRC32_POLY_REFLECTED 0xEDB88320u

/* The precomputed table maps each possible trailing byte to the 32-bit value it
 * contributes. It is filled in once by crc32_build_table. */
static uint32_t crc32_table[256];
static int crc32_table_ready = 0;

/*
 * Populate crc32_table by simulating eight single-bit CRC steps per byte value.
 *
 * For each byte n we feed it through the bit-reflected division: if the low bit
 * is set we shift right and XOR the polynomial, otherwise we just shift. The
 * result is the CRC contribution of that byte, cached for the table-driven loop.
 * Idempotent and O(256*8); runs at most once.
 */
static void crc32_build_table(void) {
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t crc = n;
        for (int k = 0; k < 8; k++) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ CRC32_POLY_REFLECTED;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[n] = crc;
    }
    crc32_table_ready = 1;
}

/*
 * Continue a CRC-32 computation over another buffer.
 *
 * The running value follows the zlib convention: the CRC register is stored
 * inverted between calls, so seed a fresh computation with 0 (this function
 * applies the inversion internally) and feed the previous return value back in
 * for subsequent chunks.
 *
 * crc:  running CRC from a previous call, or 0 to start.
 * data: next chunk of bytes.
 * len:  length of that chunk.
 * Returns the updated CRC. O(len). Builds the table on first call.
 */
uint32_t crc32_update(uint32_t crc, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    if (!crc32_table_ready) crc32_build_table();

    crc = ~crc; /* undo the final inversion so we resume the raw register */
    while (len--) {
        /* XOR the next byte into the low end, then look up the combined effect
         * of dividing out those eight bits. */
        crc = crc32_table[(crc ^ *p++) & 0xFFu] ^ (crc >> 8);
    }
    return ~crc; /* re-apply the inversion that callers expect */
}

/*
 * Compute the CRC-32 of a buffer in one shot.
 *
 * data/len: the bytes to checksum. Returns the standard CRC-32 (the value zip
 * and PNG store). Equivalent to crc32_update(0, data, len). O(len).
 */
uint32_t crc32(const void *data, size_t len) {
    return crc32_update(0u, data, len);
}

/*
 * Verify a buffer against an expected CRC-32.
 *
 * data/len: the bytes. expected: the checksum to compare against.
 * Returns 1 if the recomputed CRC matches, 0 otherwise. O(len).
 */
int crc32_verify(const void *data, size_t len, uint32_t expected) {
    return crc32(data, len) == expected;
}
