/* crackme01  --  level 1 / 10
 * concept: nothing hidden. the password is a literal in the binary.
 * try:     strings crackme01
 */
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[64];
    printf("crackme01 :: password: ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;
    buf[strcspn(buf, "\n")] = 0;

    if (strcmp(buf, "sunshine_42") == 0)
        puts("[+] granted -- NXRT{l01_str1ngs_4re_y0ur_fr1end}");
    else
        puts("[-] denied");
    return 0;
}
