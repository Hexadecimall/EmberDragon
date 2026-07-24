/* crackme06  --  level 6 / 10
 * concept: no single comparison -- a SYSTEM of constraints over 6 bytes.
 *          the system is underdetermined: many valid keys exist. solve it by
 *          hand, or feed the constraints to an SMT solver (z3).
 */
#include <stdio.h>
#include <string.h>

int main(void) {
    unsigned char k[64];
    printf("crackme06 :: key (6 chars): ");
    if (!fgets((char*)k, sizeof k, stdin)) return 1;
    k[strcspn((char*)k, "\n")] = 0;
    if (strlen((char*)k) != 6) { puts("[-] denied"); return 0; }

    int ok = 1;
    if (((k[0] + k[5]) & 0xFF) != 0xA6) ok = 0;
    if ((k[1] ^ k[4])         != 0x24) ok = 0;
    if (((k[3] - k[2]) & 0xFF) != 0x0A) ok = 0;
    if ((k[0] ^ k[2])         != 0x1C) ok = 0;
    if (((k[1] + k[3]) & 0xFF) != 0xCE) ok = 0;
    if ((k[0]+k[1]+k[2]+k[3]+k[4]+k[5]) != 532) ok = 0;

    puts(ok ? "[+] granted -- NXRT{l06_c0nstr41nt_s0lv1ng}" : "[-] denied");
    return 0;
}
