/* crackme04  --  level 4 / 10
 * concept: each character is checked against a table after an index-dependent
 *          transform. recover char i with: enc[i] ^ (i*7+3).
 */
#include <stdio.h>
#include <string.h>

static const unsigned char enc[] = { 91, 58, 99, 71, 46, 72, 73, 7, 67, 113, 45, 113 };

int main(void) {
    char buf[64];
    printf("crackme04 :: key: ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;
    buf[strcspn(buf, "\n")] = 0;

    if (strlen(buf) != sizeof enc) { puts("[-] denied"); return 0; }
    int ok = 1;
    for (size_t i = 0; i < sizeof enc; i++)
        if (((unsigned char)buf[i] ^ ((i * 7 + 3) & 0xFF)) != enc[i]) ok = 0;

    puts(ok ? "[+] granted -- NXRT{l04_1nd3x3d_xor_l00p}" : "[-] denied");
    return 0;
}
