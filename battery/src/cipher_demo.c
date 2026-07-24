/* Caesar + XOR cipher over a fixed string, with round-trip verification. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static void caesar(char *s, int shift) {
    for (size_t i = 0; s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'a' && c <= 'z')
            s[i] = (char)('a' + ((c - 'a' + shift + 26) % 26));
        else if (c >= 'A' && c <= 'Z')
            s[i] = (char)('A' + ((c - 'A' + shift + 26) % 26));
    }
}

static void xorpass(char *s, size_t n, uint8_t key) {
    for (size_t i = 0; i < n; i++)
        s[i] = (char)((uint8_t)s[i] ^ key);
}

int main(void) {
    char msg[] = "Attack at Dawn!";
    char work[64];
    strcpy(work, msg);

    caesar(work, 3);
    printf("caesar+3: %s\n", work);
    caesar(work, -3);
    printf("decoded : %s\n", work);

    size_t n = strlen(work);
    uint8_t key = 0x5A;
    xorpass(work, n, key);
    printf("xor bytes:");
    for (size_t i = 0; i < n; i++)
        printf(" %02X", (unsigned char)work[i]);
    printf("\n");

    xorpass(work, n, key);
    printf("xor back: %s\n", work);

    printf("roundtrip %s\n", strcmp(work, msg) == 0 ? "OK" : "FAIL");
    return 0;
}
