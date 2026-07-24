/* A varargs-FREE printf-style formatter: walks a format string and a
   parallel int array, writing into a fixed buffer. Exercises a char-by-char
   state machine + manual integer-to-decimal conversion. */
#include <stdio.h>
#include <string.h>

static int itoa10(int v, char *out) {
    char tmp[12];
    int n = 0, neg = v < 0;
    unsigned uv = neg ? (unsigned)(-(long)v) : (unsigned)v;
    do { tmp[n++] = (char)('0' + uv % 10); uv /= 10; } while (uv);
    int k = 0;
    if (neg) out[k++] = '-';
    while (n > 0) out[k++] = tmp[--n];
    return k;
}

/* %d consumes next arg; %% is literal percent. */
static int fmt(char *buf, const char *f, const int *args, int nargs) {
    int o = 0, ai = 0;
    for (const char *p = f; *p; ++p) {
        if (*p != '%') { buf[o++] = *p; continue; }
        ++p;
        if (*p == '%') { buf[o++] = '%'; }
        else if (*p == 'd' && ai < nargs) { o += itoa10(args[ai++], buf + o); }
        else { buf[o++] = '?'; }
    }
    buf[o] = '\0';
    return o;
}

int main(void) {
    char buf[128];
    int args[3] = { 7, -42, 1000 };
    int len = fmt(buf, "x=%d y=%d z=%d (100%%)", args, 3);
    printf("[%s] len=%d\n", buf, len);
    return 0;
}
