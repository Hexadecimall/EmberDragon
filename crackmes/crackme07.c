/* crackme07  --  level 7 / 10
 * concept: a custom 32-bit hash of a 5-char key must equal a target.
 *          not directly invertible -- but the last char only affects the low
 *          byte. fix the first 4, then solve the 5th. (partial brute + solve)
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void) {
    char buf[64];
    printf("crackme07 :: key (5 chars): ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;
    buf[strcspn(buf, "\n")] = 0;
    if (strlen(buf) != 5) { puts("[-] denied"); return 0; }

    uint32_t h = 0;
    for (int i = 0; i < 5; i++)
        h = (((h << 4) + h) ^ (unsigned char)buf[i]);

    if (h == 0x0063533Eu)
        puts("[+] granted -- NXRT{l07_h4sh_pr31m4g3_g4me}");
    else
        puts("[-] denied");
    return 0;
}
