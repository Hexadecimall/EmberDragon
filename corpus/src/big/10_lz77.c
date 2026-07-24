/*
 * Minimal LZ77 sliding-window compressor.
 *
 * The encoder finds the longest match for the upcoming bytes inside a fixed
 * window of already-seen data and emits tokens of the form (offset, length,
 * nextLiteral). Decoding replays each token by copying from the reconstructed
 * output, which is why overlapping copies (offset < length) are valid and
 * intentionally supported.
 */

#include <stdint.h>
#include <stdlib.h>

/* How far back a match may reference and how long a single match may be. */
#define LZ_WINDOW 4096
#define LZ_MAX_MATCH 18
#define LZ_MIN_MATCH 3

/*
 * A single LZ77 token: copy `length` bytes from `offset` back, then output the
 * literal byte. When no useful match exists, length is 0 and offset is unused.
 */
typedef struct {
    uint16_t offset;
    uint16_t length;
    uint8_t  literal;
} LzToken;

/*
 * Find the longest match for src[pos..] within the preceding window.
 *
 * Scans every candidate start position from the window boundary up to pos,
 * measuring how many bytes match (capped at LZ_MAX_MATCH and the remaining
 * input). The best offset/length are written through the out-parameters.
 *
 * Returns the best match length found, or 0 if nothing reached LZ_MIN_MATCH.
 * Runs in O(window * maxMatch) per call.
 */
static uint16_t findLongestMatch(const uint8_t *src, size_t srcLen, size_t pos,
                                 uint16_t *bestOffset) {
    uint16_t bestLen = 0;
    *bestOffset = 0;
    /* The window cannot start before the buffer's beginning. */
    size_t start = pos > LZ_WINDOW ? pos - LZ_WINDOW : 0;
    for (size_t cand = start; cand < pos; cand++) {
        size_t len = 0;
        /* Compare forward; cand+len may pass pos for overlapping runs. */
        while (len < LZ_MAX_MATCH &&
               pos + len < srcLen &&
               src[cand + len] == src[pos + len]) {
            len++;
        }
        if (len > bestLen) {
            bestLen = (uint16_t)len;
            *bestOffset = (uint16_t)(pos - cand);
        }
    }
    /* Short matches cost more to encode than they save, so reject them. */
    return bestLen >= LZ_MIN_MATCH ? bestLen : 0;
}

/*
 * Compress src into a caller-allocated array of tokens.
 *
 * tokens   : output array; tokenCap must be at least srcLen tokens to be safe.
 * Returns the number of tokens written, or (size_t)-1 if tokenCap is too small.
 * Overall complexity is O(n * window) due to the per-position search.
 */
size_t lzCompress(const uint8_t *src, size_t srcLen,
                  LzToken *tokens, size_t tokenCap) {
    size_t pos = 0;
    size_t count = 0;
    while (pos < srcLen) {
        if (count >= tokenCap) {
            return (size_t)-1; /* Defensive: never write past the array. */
        }
        uint16_t offset;
        uint16_t len = findLongestMatch(src, srcLen, pos, &offset);
        LzToken t;
        t.offset = offset;
        t.length = len;
        /* Advance past the match, then attach the following byte as a literal. */
        pos += len;
        t.literal = (pos < srcLen) ? src[pos] : 0;
        pos += 1;
        tokens[count++] = t;
    }
    return count;
}

/*
 * Reconstruct the original bytes from a token stream.
 *
 * dst/dstCap describe the output buffer. Each token first copies `length`
 * bytes from `offset` back in the already-written output, then appends its
 * literal. Decoding stops early if the buffer would overflow.
 *
 * Returns the number of bytes produced. O(m) in the decoded length.
 */
size_t lzDecompress(const LzToken *tokens, size_t tokenCount,
                    uint8_t *dst, size_t dstCap) {
    size_t out = 0;
    for (size_t i = 0; i < tokenCount; i++) {
        LzToken t = tokens[i];
        for (uint16_t k = 0; k < t.length; k++) {
            if (out >= dstCap || t.offset > out) {
                return out; /* Malformed or full: stop without corrupting memory. */
            }
            /* Byte-by-byte copy so overlapping runs replicate correctly. */
            dst[out] = dst[out - t.offset];
            out++;
        }
        /* The final token of a buffer may carry a padding literal; keep it. */
        if (out < dstCap) {
            dst[out++] = t.literal;
        }
    }
    return out;
}
