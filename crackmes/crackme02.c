/* crackme02  --  level 2 / 10
 * concept: the password isn't stored in clear -- it's xor'd with a constant
 *          and decoded at runtime. strings won't show it; trace the loop.
 */
#include <stdio.h>
#include <string.h>

static const unsigned char enc[] = { 50, 110, 57, 49, 5, 46, 50, 63, 5, 42, 54, 110, 52, 63, 46 };

int main(void) {
    char buf[64], key[sizeof enc + 1];
    for (size_t i = 0; i < sizeof enc; i++) key[i] = enc[i] ^ 0x5A;
    key[sizeof enc] = 0;

    printf("crackme02 :: password: ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;
    buf[strcspn(buf, "\n")] = 0;

    if (strcmp(buf, key) == 0)
        puts("[+] granted -- NXRT{l02_x0r_1s_n0t_encrypt10n}");
    else
        puts("[-] denied");
    return 0;
}
