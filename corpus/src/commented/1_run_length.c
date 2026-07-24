/*
 * run_length.c
 *
 * Run-length encoding and decoding for byte strings. Each run is emitted as a
 * one-byte ASCII count (capped at 9 so a single digit always suffices)
 * followed by the repeated character, e.g. "aaaabbb" -> "4a3b". The decoder
 * reverses that exact format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The longest run that a single decimal digit can represent. */
#define MAX_RUN_LENGTH 9

/*
 * Encode `input` into newly allocated heap memory using run-length encoding.
 * Returns a NUL-terminated string the caller must free, or NULL on allocation
 * failure. Runs longer than MAX_RUN_LENGTH are split across several count
 * pairs. The worst case (no repeats) doubles the size, so the buffer is sized
 * for 2*len + 1 bytes. O(n) time.
 */
char *runLengthEncode(const char *input) {
    size_t inputLen = strlen(input);

    /* Two output bytes per input byte is the absolute worst case. */
    char *output = malloc(inputLen * 2 + 1);
    if (output == NULL) {
        return NULL;
    }

    size_t outPos = 0;
    size_t i = 0;
    while (i < inputLen) {
        char current = input[i];
        int  runLength = 0;

        /* Extend the run while the same byte repeats and we stay under the
         * single-digit cap; the cap forces a split into a fresh pair. */
        while (i < inputLen && input[i] == current && runLength < MAX_RUN_LENGTH) {
            runLength++;
            i++;
        }

        output[outPos++] = (char)('0' + runLength);  /* digit '1'..'9' */
        output[outPos++] = current;
    }

    output[outPos] = '\0';
    return output;
}

/*
 * Decode a run-length string produced by runLengthEncode back into the
 * original byte string. Returns a NUL-terminated heap string the caller must
 * free, or NULL on allocation failure or malformed input (odd length, or a
 * count byte that is not '1'..'9'). Performs two passes: one to size the
 * output, one to fill it. O(n) where n is the decoded length.
 */
char *runLengthDecode(const char *input) {
    size_t inputLen = strlen(input);

    /* Encoded data always comes in (count, char) pairs. */
    if (inputLen % 2 != 0) {
        return NULL;
    }

    /* First pass: validate the format and total the decoded length. */
    size_t decodedLen = 0;
    for (size_t i = 0; i < inputLen; i += 2) {
        char countByte = input[i];
        if (countByte < '1' || countByte > '9') {
            return NULL;  /* not a legal single-digit run count */
        }
        decodedLen += (size_t)(countByte - '0');
    }

    char *output = malloc(decodedLen + 1);
    if (output == NULL) {
        return NULL;
    }

    /* Second pass: expand each pair into its repeated characters. */
    size_t outPos = 0;
    for (size_t i = 0; i < inputLen; i += 2) {
        int  count = input[i] - '0';
        char value = input[i + 1];
        for (int k = 0; k < count; k++) {
            output[outPos++] = value;
        }
    }

    output[outPos] = '\0';
    return output;
}

/*
 * Compute how many bytes runLengthEncode would emit for `input` without
 * allocating the encoded buffer. Useful for deciding whether encoding is even
 * worthwhile. Returns the encoded length in bytes (excluding the terminator).
 * O(n) time, O(1) space.
 */
size_t encodedLength(const char *input) {
    size_t inputLen = strlen(input);
    size_t total = 0;
    size_t i = 0;

    while (i < inputLen) {
        char current = input[i];
        int  runLength = 0;
        while (i < inputLen && input[i] == current && runLength < MAX_RUN_LENGTH) {
            runLength++;
            i++;
        }
        total += 2;  /* every run contributes exactly one count+char pair */
    }
    return total;
}
