/*
 * fnv1a.c — FNV-1a non-cryptographic hash (32-bit and 64-bit variants).
 *
 * FNV-1a is a fast, simple hash with good avalanche behavior for short keys,
 * making it a popular choice for hash-table bucketing and quick content
 * fingerprints. Each input byte is XORed into the running hash and then the
 * hash is multiplied by a fixed prime; doing the XOR before the multiply (the
 * "1a" ordering) gives noticeably better dispersion than the original FNV-1.
 */

#include <stdint.h>
#include <stddef.h>

/* The canonical FNV constants. The offset basis seeds the hash and the prime
 * is the per-byte multiplier; both are mandated by the FNV specification. */
#define FNV32_OFFSET_BASIS 0x811C9DC5u
#define FNV32_PRIME        0x01000193u
#define FNV64_OFFSET_BASIS 0xCBF29CE484222325ULL
#define FNV64_PRIME        0x00000100000001B3ULL

/*
 * Hash a byte buffer with the 32-bit FNV-1a algorithm.
 *
 * data: pointer to the bytes to hash (may be NULL only if len == 0).
 * len:  number of bytes to consume.
 * Returns the 32-bit hash. Runs in O(len) with no allocation.
 */
uint32_t fnv1a_32(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint32_t hash = FNV32_OFFSET_BASIS;
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];          /* fold the byte in first ("1a" ordering) */
        hash *= FNV32_PRIME;   /* unsigned wraparound is intentional and defined */
    }
    return hash;
}

/*
 * Hash a byte buffer with the 64-bit FNV-1a algorithm.
 *
 * Same contract as fnv1a_32 but yields a wider digest, useful when collision
 * resistance over large key spaces matters. O(len), no allocation.
 */
uint64_t fnv1a_64(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint64_t hash = FNV64_OFFSET_BASIS;
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];
        hash *= FNV64_PRIME;
    }
    return hash;
}

/*
 * Hash a NUL-terminated C string with 32-bit FNV-1a.
 *
 * str: NUL-terminated string; the terminating NUL is NOT included in the hash.
 * Returns the 32-bit hash. Convenience wrapper that walks until '\0'.
 */
uint32_t fnv1a_32_str(const char *str) {
    uint32_t hash = FNV32_OFFSET_BASIS;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= FNV32_PRIME;
    }
    return hash;
}

/*
 * Incrementally extend a running 32-bit FNV-1a hash with more bytes.
 *
 * This lets a caller hash a stream in chunks: seed the first call with
 * FNV32_OFFSET_BASIS, then thread the returned value into each subsequent call.
 * Because FNV-1a processes one byte at a time with no internal state beyond the
 * accumulator, splitting the input has no effect on the final result.
 *
 * hash: the accumulator carried over from previous chunks.
 * data: next chunk of bytes.
 * len:  length of that chunk.
 * Returns the updated accumulator.
 */
uint32_t fnv1a_32_update(uint32_t hash, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        hash ^= p[i];
        hash *= FNV32_PRIME;
    }
    return hash;
}

/*
 * Reduce a full hash into a table index using a power-of-two mask.
 *
 * hash:      a value produced by one of the fnv1a_* functions.
 * table_bits: log2 of the table size; the table must have 2^table_bits slots.
 * Returns an index in [0, 2^table_bits). Masking is valid here only because
 * FNV-1a mixes the low bits well; for non-power-of-two tables use modulo.
 */
uint32_t fnv1a_index(uint32_t hash, unsigned table_bits) {
    uint32_t mask = (table_bits >= 32) ? 0xFFFFFFFFu : ((1u << table_bits) - 1u);
    return hash & mask;
}
