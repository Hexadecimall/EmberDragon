/*
 * csv_field.cpp -- Quote and escape a single field for CSV (RFC 4180) output.
 *
 * Decides whether a string needs to be wrapped in double quotes and, if so,
 * doubles any embedded quotes per the CSV convention. This is the per-cell
 * primitive a CSV writer applies before joining fields with commas. Pure
 * character logic with no allocation beyond the caller's buffer.
 */

#include <cstddef>

namespace csv {

/*
 * needsQuoting -- Determine whether a field must be quoted.
 *
 * @field: the NUL-terminated cell value to inspect.
 * Returns true if the field contains a comma, double quote, carriage return,
 * or newline -- the four characters that force quoting under RFC 4180.
 * O(length of the field).
 */
static bool needsQuoting(const char *field) {
    for (std::size_t i = 0; field[i] != '\0'; i++) {
        char c = field[i];
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            return true;
        }
    }
    return false;
}

/*
 * rawLength -- Length of a NUL-terminated string.
 *
 * @s: the string.
 * Returns the count of bytes before the terminator. O(n). Kept local so the
 * file needs no <cstring>.
 */
static std::size_t rawLength(const char *s) {
    std::size_t n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

/*
 * encodeField -- Write the CSV-encoded form of `field` into `out`.
 *
 * @out:   destination buffer for the NUL-terminated encoded field.
 * @cap:   capacity of `out` in bytes.
 * @field: the cell value to encode.
 *
 * Returns the number of characters written, or 0 if the encoded form does not
 * fit in `cap`. When quoting is required the field is wrapped in double quotes
 * and every interior double quote is doubled ("" ). Fields that need no
 * special handling are copied verbatim. O(length of the field).
 */
std::size_t encodeField(char *out, std::size_t cap, const char *field) {
    if (cap == 0) {
        return 0;
    }

    bool quote = needsQuoting(field);

    if (!quote) {
        /* Fast path: copy as-is if it fits, including the terminator. */
        std::size_t len = rawLength(field);
        if (len + 1 > cap) {
            out[0] = '\0';
            return 0;
        }
        for (std::size_t i = 0; i < len; i++) {
            out[i] = field[i];
        }
        out[len] = '\0';
        return len;
    }

    /* Quoted path. First measure the encoded size so we can refuse to start
     * writing a result that cannot fit -- avoids producing a truncated cell.
     * Size = 2 surrounding quotes + each char + 1 extra per interior quote. */
    std::size_t encoded = 2;
    for (std::size_t i = 0; field[i] != '\0'; i++) {
        encoded += (field[i] == '"') ? 2u : 1u;  /* a quote becomes "" */
    }
    if (encoded + 1 > cap) {
        out[0] = '\0';
        return 0;
    }

    std::size_t idx = 0;
    out[idx++] = '"';                 /* opening quote */
    for (std::size_t i = 0; field[i] != '\0'; i++) {
        char c = field[i];
        if (c == '"') {
            out[idx++] = '"';         /* escape by doubling */
        }
        out[idx++] = c;
    }
    out[idx++] = '"';                 /* closing quote */
    out[idx] = '\0';
    return idx;
}

} // namespace csv
