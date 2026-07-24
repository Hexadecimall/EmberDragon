/*
 * LZW dictionary-based compressor.
 *
 * LZW builds a dictionary of byte strings on the fly: it starts with all 256
 * single-byte entries and adds a new multi-byte entry every time it emits a
 * code. Because the decoder rebuilds the identical dictionary from the code
 * stream alone, no table is transmitted. This implementation uses a simple
 * hash map keyed by (prefix code, next byte) for the encoder.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Codes 0..255 are the literal bytes; new strings start at 256. */
static const int kFirstFreeCode = 256;
/* Hash table size; a prime keeps probe sequences well spread. */
static const int kHashSize = 9973;

/*
 * One encoder dictionary slot mapping a (prefix, suffix) pair to its code.
 *
 * An empty slot has code == -1. Collisions are resolved by linear probing.
 */
struct LzwEntry {
    int     prefix;
    uint8_t suffix;
    int     code;
};

/*
 * The encoder's open-addressed hash table plus the next code to assign.
 */
struct LzwDict {
    LzwEntry slots[kHashSize];
    int      nextCode;
};

/*
 * Reset the dictionary so only the 256 literal codes are implicitly present.
 *
 * The single-byte codes are never stored explicitly; they equal the byte
 * value, so the table only holds multi-byte strings. O(kHashSize).
 */
static void lzwDictReset(LzwDict *dict) {
    for (int i = 0; i < kHashSize; i++) {
        dict->slots[i].code = -1; /* mark empty */
    }
    dict->nextCode = kFirstFreeCode;
}

/*
 * Hash a (prefix, suffix) pair into a starting bucket index.
 *
 * Mixes the prefix code with the suffix byte; the caller linear-probes from
 * the returned index. O(1).
 */
static int lzwHash(int prefix, uint8_t suffix) {
    unsigned h = (unsigned)prefix * 257u + (unsigned)suffix + 1u;
    return (int)(h % (unsigned)kHashSize);
}

/*
 * Look up the code for (prefix, suffix), or -1 if the string is not present.
 *
 * Linear-probes until it finds a match or an empty slot. Returns the stored
 * code, or -1 when absent. O(1) amortised for a lightly loaded table.
 */
static int lzwLookup(const LzwDict *dict, int prefix, uint8_t suffix) {
    int idx = lzwHash(prefix, suffix);
    while (dict->slots[idx].code != -1) {
        if (dict->slots[idx].prefix == prefix &&
            dict->slots[idx].suffix == suffix) {
            return dict->slots[idx].code;
        }
        idx = (idx + 1) % kHashSize; /* probe the next bucket */
    }
    return -1;
}

/*
 * Insert (prefix, suffix) with the next available code.
 *
 * Assumes the string is absent (caller checked via lzwLookup). Does nothing if
 * the table is effectively full, capping dictionary growth. O(1) amortised.
 */
static void lzwInsert(LzwDict *dict, int prefix, uint8_t suffix) {
    if (dict->nextCode >= kHashSize) {
        return; /* Stop growing rather than overflow the fixed table. */
    }
    int idx = lzwHash(prefix, suffix);
    while (dict->slots[idx].code != -1) {
        idx = (idx + 1) % kHashSize;
    }
    dict->slots[idx].prefix = prefix;
    dict->slots[idx].suffix = suffix;
    dict->slots[idx].code = dict->nextCode++;
}

/*
 * Compress src into an array of integer codes.
 *
 * src/srcLen : input bytes.
 * codes      : output code array, sized by the caller (codeCap entries).
 * Returns the number of codes emitted, or (size_t)-1 if codeCap is exceeded.
 * Runs in O(n) amortised thanks to the hash dictionary.
 */
size_t lzwCompress(const uint8_t *src, size_t srcLen,
                   int *codes, size_t codeCap) {
    if (srcLen == 0) {
        return 0;
    }
    LzwDict *dict = (LzwDict *)malloc(sizeof(LzwDict));
    if (!dict) {
        return (size_t)-1;
    }
    lzwDictReset(dict);

    size_t out = 0;
    int current = src[0]; /* the running string, seeded with the first byte */
    for (size_t i = 1; i < srcLen; i++) {
        uint8_t next = src[i];
        int combined = lzwLookup(dict, current, next);
        if (combined != -1) {
            /* The extended string already exists; keep growing it. */
            current = combined;
        } else {
            /* Emit the longest known string, then learn the extension. */
            if (out >= codeCap) {
                free(dict);
                return (size_t)-1;
            }
            codes[out++] = current;
            lzwInsert(dict, current, next);
            current = next;
        }
    }
    /* Flush the final pending string. */
    if (out >= codeCap) {
        free(dict);
        return (size_t)-1;
    }
    codes[out++] = current;
    free(dict);
    return out;
}

/*
 * Decompress a code stream produced by lzwCompress() back into bytes.
 *
 * The decoder rebuilds the dictionary as an array of (prefix, suffix) entries
 * and reconstructs each string by walking prefixes backward. It handles the
 * classic KwKwK case where a code refers to the entry being defined this step.
 *
 * codes/codeCount : the compressed codes.
 * dst/dstCap      : output buffer.
 * Returns the number of bytes written, or (size_t)-1 on allocation failure or
 * a malformed code. O(total output length).
 */
size_t lzwDecompress(const int *codes, size_t codeCount,
                     uint8_t *dst, size_t dstCap) {
    if (codeCount == 0) {
        return 0;
    }
    /* Parallel arrays form the decoder dictionary indexed by code. */
    int    *prefixOf = (int *)malloc(sizeof(int) * (size_t)kHashSize);
    uint8_t *suffixOf = (uint8_t *)malloc((size_t)kHashSize);
    uint8_t *scratch  = (uint8_t *)malloc((size_t)kHashSize);
    if (!prefixOf || !suffixOf || !scratch) {
        free(prefixOf); free(suffixOf); free(scratch);
        return (size_t)-1;
    }
    int nextCode = kFirstFreeCode;

    size_t out = 0;
    int previous = codes[0];
    if (previous < 0 || previous > 255 || out >= dstCap) {
        free(prefixOf); free(suffixOf); free(scratch);
        return (size_t)-1;
    }
    dst[out++] = (uint8_t)previous; /* first code is always a literal byte */

    for (size_t i = 1; i < codeCount; i++) {
        int code = codes[i];
        int walk;
        uint8_t firstByte;

        if (code < kFirstFreeCode || (code < nextCode)) {
            /* Known code: expand it directly. */
            walk = code;
        } else if (code == nextCode) {
            /* KwKwK: the code is the one we are about to define; its string is
             * the previous string plus that string's own first byte. */
            walk = previous;
        } else {
            free(prefixOf); free(suffixOf); free(scratch);
            return (size_t)-1; /* code from the future is corrupt */
        }

        /* Decode `walk` into scratch in reverse, since we follow prefixes. */
        size_t len = 0;
        int node = walk;
        while (node >= kFirstFreeCode) {
            scratch[len++] = suffixOf[node];
            node = prefixOf[node];
        }
        scratch[len++] = (uint8_t)node; /* the terminal literal byte */
        firstByte = (uint8_t)node;

        /* Emit the string in forward order. */
        for (size_t k = len; k > 0; k--) {
            if (out >= dstCap) {
                free(prefixOf); free(suffixOf); free(scratch);
                return out;
            }
            dst[out++] = scratch[k - 1];
        }
        /* For the KwKwK case the emitted string ends with its own first byte. */
        if (code == nextCode) {
            /* already handled: walk==previous, firstByte is previous's first */
        }

        /* Define the new dictionary entry: previous string + firstByte. */
        if (nextCode < kHashSize) {
            prefixOf[nextCode] = previous;
            suffixOf[nextCode] = firstByte;
            nextCode++;
        }
        previous = code;
    }

    free(prefixOf); free(suffixOf); free(scratch);
    return out;
}
