#include <cstdio>
#include <cstring>

void caesar(long s, int a);
void xorpass(long data, long n2, char a);


static const char str[] = "\n";

int main(int argc, char** argv) {
    char decoded[64];
    char v128;
    long len;
    long i;
    // neon:  ldr    value0, [v153, #0x0]  // = "Attack at Dawn!"
    __strcpy_chk(decoded, &v128, 64);
    caesar(decoded, 3);
    printf("caesar+3: %s\n", decoded);
    caesar(decoded, -3);
    printf("decoded : %s\n", decoded);
    len = strlen(decoded);
    xorpass(decoded, len, 90);
    printf("xor bytes:");
    i = 0;
    while (i < len) {
        printf(" %02X", decoded[i]);
        i++;
    }
    printf(str);
    xorpass(decoded, len, 90);
    printf("xor back: %s\n", decoded);
    printf("roundtrip %s\n", (strcmp(decoded, &v128) == 0 ? "OK" : "FAIL"));
    return 0;
}

void caesar(long s, int a) {
    long i;
    char v7;
    i = 0;
    while (!(s[i] == 0)) {
        v7 = s[i];
        if (v7 >= 97) {
            if (v7 <= 122) {
                s[i] = v7 - 97 + a + 26 - (v7 - 97 + a + 26) / 26 * 26 + 97;
                goto L1;
            }
        } else {
            if (v7 >= 65) {
                if (v7 <= 90) {
                    s[i] = v7 - 65 + a + 26 - (v7 - 65 + a + 26) / 26 * 26 + 65;
                }
            }
        }
        L1:
        i++;
    }
    return;
}

void xorpass(long data, long n2, char a) {
    long n;
    long i;
    n = n2;
    i = 0;
    while (i < n) {
        data[i] ^= a;
        i++;
    }
    return;
}

