/*
 * crc32.c — Standard CRC-32 (IEEE 802.3 / zlib) checksum implementation.
 *
 * Computes the same 32-bit cyclic redundancy check used by zlib, gzip, and
 * PNG. Uses the reflected polynomial 0xEDB88320 with a lazily-built 256-entry
 * lookup table, and supports streaming a checksum across multiple buffers.
 */

#include <stddef.h>
#include <stdint.h>

/*
 * Reflected form of the CRC-32 polynomial 0x04C11DB7. "Reflected" means the
 * bit order is mirrored, which matches the LSB-first processing used here.
 */
#define CRC32_POLY 0xEDB88320u

/*
 * Lookup table mapping each possible byte value to its partial CRC remainder.
 * Marked built so the table is computed exactly once across calls.
 */
static uint32_t crc_table[256];
static int crc_table_built = 0;

/*
 * Populate crc_table. For each byte value we simulate feeding that byte
 * through the CRC shift register: shift right, and XOR the polynomial in
 * whenever a 1 bit falls off the bottom. Idempotent — a guard prevents
 * recomputation on later calls. O(256 * 8) on first call, O(1) afterward.
 */
static void crc32_build_table(void) {
    if (crc_table_built) return;
    for (uint32_t byte = 0; byte < 256; byte++) {
        uint32_t remainder = byte;
        for (int bit = 0; bit < 8; bit++) {
            /* If the low bit is set, the polynomial divides in this step. */
            if (remainder & 1u)
                remainder = (remainder >> 1) ^ CRC32_POLY;
            else
                remainder >>= 1;
        }
        crc_table[byte] = remainder;
    }
    crc_table_built = 1;
}

/*
 * Begin a new CRC-32 computation. Returns the initial running value, which
 * is the conventional all-ones seed (0xFFFFFFFF). Feed this into
 * crc32_update(), then pass the final running value to crc32_finalize().
 */
uint32_t crc32_init(void) {
    crc32_build_table();
    return 0xFFFFFFFFu;
}

/*
 * Fold `length` bytes from `data` into the running CRC value `crc`.
 * Returns the updated running value. May be called repeatedly to checksum
 * a stream in chunks. Complexity is O(length) — one table lookup per byte.
 */
uint32_t crc32_update(uint32_t crc, const void *data, size_t length) {
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < length; i++) {
        /* Index the table by the low byte XORed with the next input byte,
         * then shift the consumed byte out of the register. */
        uint8_t index = (uint8_t)(crc ^ p[i]);
        crc = (crc >> 8) ^ crc_table[index];
    }
    return crc;
}

/*
 * Produce the final checksum from a running value. The running CRC is
 * inverted (XOR with all ones) to match the IEEE convention. Returns the
 * 32-bit checksum suitable for storing or comparing.
 */
uint32_t crc32_finalize(uint32_t crc) {
    return crc ^ 0xFFFFFFFFu;
}

/*
 * Convenience one-shot: checksum a single buffer end to end. Equivalent to
 * init -> update -> finalize. Returns the IEEE CRC-32 of `data[0..length)`.
 */
uint32_t crc32_compute(const void *data, size_t length) {
    uint32_t crc = crc32_init();
    crc = crc32_update(crc, data, length);
    return crc32_finalize(crc);
}

/*
 * Verify that `data` matches a previously recorded checksum `expected`.
 * Returns 1 if the recomputed CRC equals `expected`, 0 otherwise.
 */
int crc32_verify(const void *data, size_t length, uint32_t expected) {
    return crc32_compute(data, length) == expected;
}
