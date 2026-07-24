/*
 * Move-to-front (MTF) transform over a 256-symbol alphabet.
 *
 * MTF replaces each byte with its current index in a list, then moves that
 * byte to the front. Data with local repetition (such as the output of a
 * Burrows-Wheeler transform) produces lots of small indices, which an entropy
 * coder compresses well. The transform is fully reversible and stateless
 * across calls once the alphabet is reset.
 */

#include <stdint.h>
#include <stddef.h>

/* The alphabet covers every possible byte value. */
#define MTF_ALPHABET 256

/*
 * The mutable symbol ordering used by both encode and decode.
 *
 * `order[i]` is the byte value currently at rank i; rank 0 is the front.
 */
typedef struct {
    uint8_t order[MTF_ALPHABET];
} MtfTable;

/*
 * Reset the table to the identity ordering 0,1,2,...,255.
 *
 * Both encoder and decoder must start from the same state, so this is called
 * before each independent buffer. O(alphabet).
 */
void mtfInit(MtfTable *table) {
    for (int i = 0; i < MTF_ALPHABET; i++) {
        table->order[i] = (uint8_t)i;
    }
}

/*
 * Find the current rank of a byte value in the ordering.
 *
 * Returns the index 0..255 at which `value` currently sits. Always succeeds
 * because every byte value is present exactly once. O(alphabet).
 */
static int mtfRankOf(const MtfTable *table, uint8_t value) {
    int rank = 0;
    while (table->order[rank] != value) {
        rank++;
    }
    return rank;
}

/*
 * Slide the byte at `rank` to the front, shifting the rest down by one.
 *
 * This is the shared "promote" step of the transform. O(rank).
 */
static void mtfPromote(MtfTable *table, int rank) {
    uint8_t value = table->order[rank];
    /* Shift everything ahead of `rank` back to open slot 0. */
    for (int i = rank; i > 0; i--) {
        table->order[i] = table->order[i - 1];
    }
    table->order[0] = value;
}

/*
 * Encode src into dst as a sequence of MTF ranks.
 *
 * table : pre-initialised via mtfInit(); its state advances as bytes process.
 * dst   : output of length srcLen (one rank byte per input byte).
 * Returns srcLen. Each byte costs an O(alphabet) lookup, so O(n * alphabet).
 */
size_t mtfEncode(MtfTable *table, const uint8_t *src, size_t srcLen,
                 uint8_t *dst) {
    for (size_t i = 0; i < srcLen; i++) {
        int rank = mtfRankOf(table, src[i]);
        dst[i] = (uint8_t)rank;     /* Emit the rank, not the byte. */
        mtfPromote(table, rank);    /* Then move the byte to the front. */
    }
    return srcLen;
}

/*
 * Decode a stream of MTF ranks back into the original bytes.
 *
 * table : pre-initialised with mtfInit() to the identity ordering.
 * src   : the rank bytes produced by mtfEncode().
 * dst   : output of length srcLen.
 * Returns srcLen. O(n * alphabet) for the same reason as encoding.
 */
size_t mtfDecode(MtfTable *table, const uint8_t *src, size_t srcLen,
                 uint8_t *dst) {
    for (size_t i = 0; i < srcLen; i++) {
        int rank = src[i];
        uint8_t value = table->order[rank]; /* Rank -> current byte at that rank. */
        dst[i] = value;
        mtfPromote(table, rank);            /* Mirror the encoder's promotion. */
    }
    return srcLen;
}

/*
 * Count how many encoded ranks equal zero.
 *
 * A high zero-count signals strong local repetition, a quick heuristic for
 * estimating how compressible the MTF output will be. O(n).
 */
size_t mtfCountZeroRuns(const uint8_t *ranks, size_t len) {
    size_t zeros = 0;
    for (size_t i = 0; i < len; i++) {
        if (ranks[i] == 0) {
            zeros++;
        }
    }
    return zeros;
}
