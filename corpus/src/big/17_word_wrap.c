/*
 * word_wrap.c -- Greedy word wrapping for plain text.
 *
 * Breaks a paragraph into lines no wider than a given column limit by packing
 * as many whitespace-delimited words onto each line as will fit (the classic
 * greedy algorithm used by terminals and text editors). Words longer than the
 * limit are placed on a line of their own rather than being split.
 */

#include <stddef.h>

/*
 * isSpace -- Test whether a byte is an ASCII whitespace separator.
 *
 * @c: the character to classify.
 * Returns nonzero for space, tab, newline, carriage return, and form feed;
 * zero otherwise. Used to find word boundaries.
 */
static int isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

/*
 * nextWord -- Locate the next word starting at or after `*pos`.
 *
 * @text:  the source string.
 * @pos:   in/out cursor; on entry points anywhere, on return points just past
 *         the returned word so the next call resumes correctly.
 * @start: out parameter set to the index where the word begins.
 * @len:   out parameter set to the word's length in characters.
 * Returns 1 if a word was found, 0 if only trailing whitespace remained.
 *
 * Leading whitespace is skipped before measuring. O(word length) per call.
 */
static int nextWord(const char *text, size_t *pos, size_t *start, size_t *len) {
    size_t i = *pos;

    /* Skip any run of separators before the word. */
    while (text[i] != '\0' && isSpace(text[i])) {
        i++;
    }
    if (text[i] == '\0') {
        return 0;               /* nothing but whitespace left */
    }

    *start = i;
    /* Advance to the first separator (or end) to delimit the word. */
    while (text[i] != '\0' && !isSpace(text[i])) {
        i++;
    }
    *len = i - *start;
    *pos = i;
    return 1;
}

/*
 * wrapText -- Greedily wrap `text` into `out`, inserting newlines.
 *
 * @out:   destination buffer for the wrapped, NUL-terminated result.
 * @cap:   capacity of `out` in bytes including the terminator.
 * @text:  source paragraph (internal whitespace runs collapse to one space).
 * @width: maximum line width in characters; treated as 1 if zero is passed.
 *
 * Returns the number of lines produced. Words are separated by a single space
 * on the same line; when the next word would exceed `width`, a newline starts
 * a fresh line. A word wider than `width` occupies its own line uncut. Writing
 * stops early if `cap` is reached, but the result is always terminated.
 * O(n) in the length of the text.
 */
size_t wrapText(char *out, size_t cap, const char *text, size_t width) {
    if (cap == 0) {
        return 0;
    }
    if (width == 0) {
        width = 1;              /* a zero limit would loop forever */
    }

    size_t pos = 0, start = 0, len = 0;
    size_t outIdx = 0;          /* write cursor into `out`            */
    size_t lineLen = 0;         /* visible chars on the current line  */
    size_t lines = 0;           /* completed-or-current line count    */
    int    haveLine = 0;        /* has the current line any content?  */

    while (nextWord(text, &pos, &start, &len)) {
        /* Decide whether this word fits after the existing line content.
         * A non-empty line needs one space of separation first. */
        size_t needed = (haveLine ? lineLen + 1 + len : len);

        if (haveLine && needed > width) {
            /* Break before this word: emit a newline and reset the line. */
            if (outIdx + 1 >= cap) break;
            out[outIdx++] = '\n';
            lineLen = 0;
            haveLine = 0;
        }

        if (haveLine) {
            /* Separate from the previous word with a single space. */
            if (outIdx + 1 >= cap) break;
            out[outIdx++] = ' ';
            lineLen++;
        } else {
            lines++;            /* starting a brand-new line */
        }

        /* Copy the word's characters, respecting the buffer capacity. */
        size_t copied = 0;
        while (copied < len && outIdx + 1 < cap) {
            out[outIdx++] = text[start + copied];
            copied++;
        }
        lineLen += copied;
        haveLine = 1;

        if (outIdx + 1 >= cap) break;   /* buffer exhausted */
    }

    out[outIdx] = '\0';
    return lines;
}
