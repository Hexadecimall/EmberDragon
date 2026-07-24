#include <stdio.h>

// From-scratch reimplementations of standard string primitives.
static int my_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (int)(p - s);
}

static int my_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static char *my_strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++)) ;
    return dst;
}

int main(void) {
    const char *words[] = {"alpha", "beta", "gamma", "alpha"};
    char buf[32];
    for (int i = 0; i < 4; i++) {
        my_strcpy(buf, words[i]);
        int len = my_strlen(buf);
        int cmp = my_strcmp(buf, "alpha");
        printf("%-6s len=%d cmp_alpha=%d\n", buf, len, cmp);
    }
    return 0;
}
