/*
 * Byte-oriented run-length encoding (RLE).
 *
 * This module compresses a buffer by collapsing consecutive identical bytes
 * into (count, value) pairs. Runs longer than 255 are split across multiple
 * pairs so every count fits in a single byte, keeping the format simple and
 * self-describing. It is intended for data with long uniform stretches such
 * as bitmap scanlines or sparse sensor logs.
 */

#include <stdint.h>
#include <stdlib.h>

/* Largest run length that a one-byte count field can express. */
#define RLE_MAX_RUN 255

/*
 * Compute the exact number of bytes the encoded stream will occupy.
 *
 * Walks the input identifying maximal runs and counts the 2-byte pairs that
 * runEncode() will emit, including the extra pairs needed to split runs that
 * exceed RLE_MAX_RUN.
 *
 * src      : input bytes (may be NULL only when srcLen is 0).
 * srcLen   : number of input bytes.
 * Returns the encoded length in bytes. O(n) over the input.
 */
size_t runEncodedSize(const uint8_t *src, size_t srcLen) {
    size_t pairs = 0;
    size_t i = 0;
    while (i < srcLen) {
        uint8_t value = src[i];
        size_t run = 1;
        /* Extend the run while the byte repeats. */
        while (i + run < srcLen && src[i + run] == value) {
            run++;
        }
        /* Each chunk of up to RLE_MAX_RUN bytes becomes one (count,value) pair. */
        pairs += (run + RLE_MAX_RUN - 1) / RLE_MAX_RUN;
        i += run;
    }
    return pairs * 2;
}

/*
 * Encode src into dst as a sequence of (count, value) byte pairs.
 *
 * The caller must size dst using runEncodedSize(); this function performs no
 * bounds checking on dst. A run longer than RLE_MAX_RUN is emitted as several
 * consecutive pairs sharing the same value.
 *
 * Returns the number of bytes written to dst. O(n) over the input.
 */
size_t runEncode(const uint8_t *src, size_t srcLen, uint8_t *dst) {
    size_t out = 0;
    size_t i = 0;
    while (i < srcLen) {
        uint8_t value = src[i];
        size_t run = 1;
        while (i + run < srcLen && src[i + run] == value) {
            run++;
        }
        i += run;
        /* Emit the run in 255-byte slices so each count fits in one byte. */
        while (run > 0) {
            size_t chunk = run > RLE_MAX_RUN ? RLE_MAX_RUN : run;
            dst[out++] = (uint8_t)chunk;
            dst[out++] = value;
            run -= chunk;
        }
    }
    return out;
}

/*
 * Decode a (count, value) pair stream back into raw bytes.
 *
 * src      : encoded pairs; srcLen must be even for well-formed input.
 * dst      : output buffer the caller has sized to the decoded length.
 * dstCap   : capacity of dst; decoding stops early if it would overflow.
 * Returns the number of bytes written, or 0 if srcLen is odd (malformed).
 * O(m) where m is the decoded length.
 */
size_t runDecode(const uint8_t *src, size_t srcLen, uint8_t *dst, size_t dstCap) {
    if (srcLen % 2 != 0) {
        return 0; /* Pairs come two bytes at a time; an odd length is corrupt. */
    }
    size_t out = 0;
    for (size_t i = 0; i < srcLen; i += 2) {
        uint8_t count = src[i];
        uint8_t value = src[i + 1];
        for (uint8_t j = 0; j < count; j++) {
            if (out >= dstCap) {
                return out; /* Refuse to write past the caller's buffer. */
            }
            dst[out++] = value;
        }
    }
    return out;
}
