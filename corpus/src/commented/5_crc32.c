/*
 * crc32.c - CRC-32 (IEEE 802.3) checksum using a runtime-generated lookup
 * table, plus a simpler additive Adler-32 checksum for comparison.
 *
 * The CRC uses the standard reflected polynomial 0xEDB88320 and the usual
 * pre/post inversion of the running value with 0xFFFFFFFF.
 */

#include <stdint.h>
#include <stddef.h>

/* Reflected form of the IEEE CRC-32 polynomial. */
#define CRC32_POLY 0xEDB88320u

/*
 * 256-entry table mapping each possible input byte to its CRC contribution.
 * Initialized lazily by crc32_init_table on first use.
 */
static uint32_t g_crcTable[256];
static int g_tableReady = 0;

/*
 * Populate the global CRC lookup table. Each entry holds the CRC of a single
 * byte value processed through the polynomial. Idempotent: repeated calls
 * after the first are no-ops. O(256 * 8) on the first call.
 */
void crc32_init_table(void) {
    if (g_tableReady)
        return;
    for (uint32_t byte = 0; byte < 256; byte++) {
        uint32_t crc = byte;
        /* Fold the byte through the polynomial one bit at a time. */
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1u)
                crc = (crc >> 1) ^ CRC32_POLY;
            else
                crc >>= 1;
        }
        g_crcTable[byte] = crc;
    }
    g_tableReady = 1;
}

/*
 * Compute the CRC-32 checksum of 'length' bytes at 'data'. Initializes the
 * lookup table on demand. Returns the final inverted CRC value. O(length).
 */
uint32_t crc32_compute(const uint8_t *data, size_t length) {
    crc32_init_table();
    /* Standard pre-conditioning: start with all ones. */
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < length; i++) {
        /* XOR the next byte into the low 8 bits, then table-shift. */
        uint8_t index = (uint8_t)(crc ^ data[i]);
        crc = (crc >> 8) ^ g_crcTable[index];
    }
    /* Post-conditioning: invert all bits to produce the reported checksum. */
    return crc ^ 0xFFFFFFFFu;
}

/*
 * Continue a CRC-32 across multiple buffers. Pass 0 as 'seed' for the first
 * chunk and the previous return value for each subsequent chunk. Returns the
 * running (already inverted) CRC so it can be fed back in directly. O(length).
 */
uint32_t crc32_update(uint32_t seed, const uint8_t *data, size_t length) {
    crc32_init_table();
    /* Undo the previous post-inversion so we can keep accumulating. */
    uint32_t crc = seed ^ 0xFFFFFFFFu;
    for (size_t i = 0; i < length; i++) {
        uint8_t index = (uint8_t)(crc ^ data[i]);
        crc = (crc >> 8) ^ g_crcTable[index];
    }
    return crc ^ 0xFFFFFFFFu;
}

/* Largest prime below 65536, used as the Adler-32 modulus. */
#define ADLER_MODULUS 65521u

/*
 * Compute the Adler-32 checksum of 'length' bytes at 'data'. The result packs
 * a running sum (low half) and a sum-of-sums (high half), each reduced modulo
 * the largest prime under 2^16. Returns the combined 32-bit value. O(length).
 */
uint32_t adler32_compute(const uint8_t *data, size_t length) {
    uint32_t low = 1;   /* sum of bytes, seeded to 1 by the spec */
    uint32_t high = 0;  /* sum of the running 'low' values        */
    for (size_t i = 0; i < length; i++) {
        low = (low + data[i]) % ADLER_MODULUS;
        high = (high + low) % ADLER_MODULUS;
    }
    /* High half occupies the upper 16 bits, low half the lower 16. */
    return (high << 16) | low;
}
