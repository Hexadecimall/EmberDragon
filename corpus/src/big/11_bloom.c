/*
 * bloom.c — a compact, fixed-size Bloom filter.
 *
 * A Bloom filter is a probabilistic set: it answers "definitely not present" or
 * "possibly present" using a bit array and several hash functions, trading a
 * tunable false-positive rate for very small memory and constant-time
 * operations. It never produces false negatives, so it is ideal as a cheap
 * pre-filter in front of an expensive lookup (disk, network, large table).
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* A Bloom filter over a bit array of bit_count bits using hash_count probes. */
typedef struct BloomFilter {
    uint8_t *bits;        /* packed bit array, 8 bits per byte */
    size_t   bit_count;   /* total number of addressable bits */
    unsigned hash_count;  /* how many bit positions each key sets/tests */
} BloomFilter;

/*
 * 64-bit FNV-1a over a buffer; used as the base hash for double hashing.
 *
 * data/len: bytes to hash. Returns a 64-bit digest. We split this into two
 * 32-bit halves to synthesize many independent hash functions cheaply.
 */
static uint64_t bloom_fnv1a(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint64_t h = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 0x00000100000001B3ULL;
    }
    return h;
}

/*
 * Allocate a Bloom filter with the requested geometry.
 *
 * bit_count:  number of bits in the array; rounded up to a whole byte.
 * hash_count: number of hash probes per key; more probes lower the false-
 *             positive rate up to a point, then raise it.
 * Returns a zeroed (empty) filter, or NULL on bad arguments or OOM. Caller
 * frees it with bloom_free.
 */
BloomFilter *bloom_create(size_t bit_count, unsigned hash_count) {
    if (bit_count == 0 || hash_count == 0) return NULL;
    BloomFilter *bf = (BloomFilter *)malloc(sizeof(BloomFilter));
    if (!bf) return NULL;
    size_t byte_count = (bit_count + 7) / 8; /* round up to full bytes */
    bf->bits = (uint8_t *)calloc(byte_count, 1);
    if (!bf->bits) {
        free(bf);
        return NULL;
    }
    bf->bit_count = bit_count;
    bf->hash_count = hash_count;
    return bf;
}

/*
 * Derive the i-th bit position for a key via Kirsch-Mitzenmacher double hashing.
 *
 * Instead of computing k independent hashes, we combine two halves of one
 * 64-bit hash as h1 + i*h2. This gives k well-distributed positions with only
 * one real hashing pass, with no measurable loss in false-positive rate.
 *
 * h1/h2: the two 32-bit halves of the base hash.
 * i:     which probe, 0-based.
 * bit_count: array size, used to fold the position into range.
 * Returns a bit index in [0, bit_count).
 */
static size_t bloom_nth_position(uint32_t h1, uint32_t h2, unsigned i,
                                 size_t bit_count) {
    uint64_t combined = (uint64_t)h1 + (uint64_t)i * (uint64_t)h2;
    return (size_t)(combined % bit_count);
}

/*
 * Insert a key into the filter by setting all of its probe bits.
 *
 * bf:  target filter.
 * key/len: the key bytes. Idempotent: adding the same key twice is harmless.
 * Runs in O(hash_count).
 */
void bloom_add(BloomFilter *bf, const void *key, size_t len) {
    uint64_t base = bloom_fnv1a(key, len);
    uint32_t h1 = (uint32_t)(base & 0xFFFFFFFF);
    uint32_t h2 = (uint32_t)(base >> 32);
    /* h2 must be nonzero or every probe collapses onto the same bit. */
    if (h2 == 0) h2 = 0x9E3779B1u; /* fractional bits of the golden ratio */

    for (unsigned i = 0; i < bf->hash_count; i++) {
        size_t pos = bloom_nth_position(h1, h2, i, bf->bit_count);
        bf->bits[pos / 8] |= (uint8_t)(1u << (pos % 8)); /* set the bit */
    }
}

/*
 * Test whether a key might be in the set.
 *
 * Returns 0 if the key is definitely absent (some probe bit is clear) and 1 if
 * it is possibly present (all probe bits are set, which may be a false
 * positive). Never returns 0 for a key that was actually added. O(hash_count).
 */
int bloom_maybe_contains(const BloomFilter *bf, const void *key, size_t len) {
    uint64_t base = bloom_fnv1a(key, len);
    uint32_t h1 = (uint32_t)(base & 0xFFFFFFFF);
    uint32_t h2 = (uint32_t)(base >> 32);
    if (h2 == 0) h2 = 0x9E3779B1u;

    for (unsigned i = 0; i < bf->hash_count; i++) {
        size_t pos = bloom_nth_position(h1, h2, i, bf->bit_count);
        if ((bf->bits[pos / 8] & (uint8_t)(1u << (pos % 8))) == 0) {
            return 0; /* a clear bit proves the key was never inserted */
        }
    }
    return 1; /* all bits set: possibly present */
}

/*
 * Count how many bits are currently set in the filter.
 *
 * This estimates how "full" the filter is; as it approaches bit_count the
 * false-positive rate climbs. Returns the population count in O(bit_count).
 */
size_t bloom_popcount(const BloomFilter *bf) {
    size_t count = 0;
    size_t byte_count = (bf->bit_count + 7) / 8;
    for (size_t i = 0; i < byte_count; i++) {
        uint8_t b = bf->bits[i];
        while (b) {            /* Brian Kernighan's bit-clearing popcount */
            b &= (uint8_t)(b - 1);
            count++;
        }
    }
    return count;
}

/*
 * Clear the filter back to empty without freeing it.
 *
 * After this every bloom_maybe_contains returns 0 again. O(bit_count/8).
 */
void bloom_clear(BloomFilter *bf) {
    size_t byte_count = (bf->bit_count + 7) / 8;
    memset(bf->bits, 0, byte_count);
}

/*
 * Release a Bloom filter and its bit array. Safe to call with NULL.
 */
void bloom_free(BloomFilter *bf) {
    if (!bf) return;
    free(bf->bits);
    free(bf);
}
