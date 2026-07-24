/* goto-based state loop: a lexer-style FSM scanning a fixed string into tokens. */
#include <stdio.h>
#include <ctype.h>

int main(void) {
    const char *s = "ab12 + 34cd";
    const char *p = s;
    int idents = 0, numbers = 0, ops = 0;
    char ch;

state_start:
    ch = *p;
    if (ch == '\0') goto done;
    if (isspace((unsigned char)ch)) { p++; goto state_start; }
    if (isalpha((unsigned char)ch)) goto state_ident;
    if (isdigit((unsigned char)ch)) goto state_number;
    /* operator */
    ops++; p++;
    goto state_start;

state_ident:
    idents++;
    while (isalnum((unsigned char)*p)) p++;
    goto state_start;

state_number:
    numbers++;
    while (isdigit((unsigned char)*p)) p++;
    /* a trailing alpha run starts a new ident on the next pass */
    goto state_start;

done:
    printf("input: \"%s\"\n", s);
    printf("idents=%d numbers=%d ops=%d\n", idents, numbers, ops);
    return 0;
}
