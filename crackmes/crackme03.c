/* crackme03  --  level 3 / 10
 * concept: numeric serial. one arithmetic relation must hold.
 *          invert the math to recover the serial (your first keygen).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
    char buf[64];
    printf("crackme03 :: serial: ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;

    uint32_t x = (uint32_t)strtoul(buf, NULL, 10);
    if (((x ^ 0xABCDu) * 0x1Fu) == 0x0051BF6Cu)
        puts("[+] granted -- NXRT{l03_inv3rt_th3_4r1thm3t1c}");
    else
        puts("[-] denied");
    return 0;
}
