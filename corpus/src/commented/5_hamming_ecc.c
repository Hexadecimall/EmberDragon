/*
 * hamming_ecc.c - Hamming(7,4) single-error-correcting code. Encodes a 4-bit
 * nibble into a 7-bit codeword with three parity bits, and decodes a codeword
 * back to its nibble while detecting and correcting any single bit flip.
 *
 * Bit layout of a codeword (positions 1..7, 1-based as in the classic scheme):
 *   p1 p2 d1 p4 d2 d3 d4
 * Parity positions are powers of two (1, 2, 4); data occupies the rest.
 */

#include <stdint.h>

/*
 * Compute even parity over the bits of 'value' that are selected by 'mask'.
 * Returns 0 if an even number of selected bits are set, 1 if odd. O(set bits)
 * thanks to the clear-lowest-bit loop.
 */
static int parity_of(uint8_t value, uint8_t mask) {
    uint8_t bits = value & mask;
    int parity = 0;
    while (bits != 0) {
        bits &= bits - 1; /* clear lowest set bit, toggling parity once */
        parity ^= 1;
    }
    return parity;
}

/*
 * Encode a 4-bit data nibble (only the low 4 bits are used) into a 7-bit
 * Hamming codeword. Returns the codeword in the low 7 bits of the result.
 * The three parity bits are chosen so each covers a fixed subset of positions.
 */
uint8_t hamming_encode(uint8_t nibble) {
    /* Extract the four data bits. */
    int d1 = (nibble >> 0) & 1;
    int d2 = (nibble >> 1) & 1;
    int d3 = (nibble >> 2) & 1;
    int d4 = (nibble >> 3) & 1;

    /* Each parity bit covers the data bits whose position index shares the
     * corresponding power-of-two bit. */
    int p1 = d1 ^ d2 ^ d4; /* covers positions 1,3,5,7 */
    int p2 = d1 ^ d3 ^ d4; /* covers positions 2,3,6,7 */
    int p4 = d2 ^ d3 ^ d4; /* covers positions 4,5,6,7 */

    /* Assemble the codeword, bit 0 = position 1, up to bit 6 = position 7. */
    uint8_t code = 0;
    code |= (uint8_t)(p1 << 0);
    code |= (uint8_t)(p2 << 1);
    code |= (uint8_t)(d1 << 2);
    code |= (uint8_t)(p4 << 3);
    code |= (uint8_t)(d2 << 4);
    code |= (uint8_t)(d3 << 5);
    code |= (uint8_t)(d4 << 6);
    return code;
}

/*
 * Decode a 7-bit Hamming codeword back into its 4-bit nibble, correcting any
 * single-bit error. The recovered nibble is stored in '*nibble' (low 4 bits).
 * Returns the 1-based position of the corrected bit (1..7), or 0 if the
 * codeword was already valid.
 */
int hamming_decode(uint8_t code, uint8_t *nibble) {
    /* Recompute each parity check against the received bits. The three check
     * results, read as a binary number (s4 s2 s1), give the syndrome: the
     * 1-based position of the flipped bit, or zero if none. */
    int s1 = parity_of(code, (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6));
    int s2 = parity_of(code, (1 << 1) | (1 << 2) | (1 << 5) | (1 << 6));
    int s4 = parity_of(code, (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
    int syndrome = (s4 << 2) | (s2 << 1) | s1;

    int corrected = 0;
    if (syndrome != 0) {
        /* The syndrome names a 1-based position; flip that bit to repair it. */
        code ^= (uint8_t)(1 << (syndrome - 1));
        corrected = syndrome;
    }

    /* Pull the data bits back out of their (now corrected) positions. */
    int d1 = (code >> 2) & 1;
    int d2 = (code >> 4) & 1;
    int d3 = (code >> 5) & 1;
    int d4 = (code >> 6) & 1;
    *nibble = (uint8_t)((d4 << 3) | (d3 << 2) | (d2 << 1) | d1);
    return corrected;
}

/*
 * Encode a full byte by splitting it into two nibbles and Hamming-encoding
 * each. The two 7-bit codewords are packed into a 16-bit result: the low
 * nibble's codeword in bits 0..6 and the high nibble's in bits 7..13.
 * Returns the packed 16-bit value.
 */
uint16_t hamming_encode_byte(uint8_t byte) {
    uint8_t lowCode = hamming_encode(byte & 0x0F);
    uint8_t highCode = hamming_encode((byte >> 4) & 0x0F);
    return (uint16_t)(lowCode | (highCode << 7));
}

/*
 * Decode a packed 16-bit pair of codewords back into the original byte,
 * correcting up to one bit error in each half. The byte is written to
 * '*byte'. Returns the total number of bits corrected across both halves
 * (0, 1, or 2).
 */
int hamming_decode_byte(uint16_t packed, uint8_t *byte) {
    uint8_t lowNibble = 0;
    uint8_t highNibble = 0;
    int lowFix = hamming_decode((uint8_t)(packed & 0x7F), &lowNibble);
    int highFix = hamming_decode((uint8_t)((packed >> 7) & 0x7F), &highNibble);
    *byte = (uint8_t)((highNibble << 4) | lowNibble);
    /* Each half reports a position; count it as one correction if nonzero. */
    return (lowFix != 0 ? 1 : 0) + (highFix != 0 ? 1 : 0);
}
