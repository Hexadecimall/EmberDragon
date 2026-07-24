/* crackme05  --  level 5 / 10
 * concept: a real keygenme. the serial is a function of the username.
 *          recover the algorithm (a djb2 variant) and write a keygen that
 *          works for ANY name.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static uint32_t h_name(const char *s) {
    uint32_t h = 5381;
    for (; *s; s++) h = h * 33u + (unsigned char)*s;
    return h;
}

int main(void) {
    char name[64], ser[64];
    printf("crackme05 :: name:   ");
    if (!fgets(name, sizeof name, stdin)) return 1;
    name[strcspn(name, "\n")] = 0;
    printf("crackme05 :: serial: ");
    if (!fgets(ser, sizeof ser, stdin)) return 1;

    uint32_t want = h_name(name) & 0xFFFFFFu;
    uint32_t got  = (uint32_t)strtoul(ser, NULL, 10);

    if (name[0] && got == want)
        puts("[+] granted -- NXRT{l05_n4me_t0_s3r1al_k3yg3n}");
    else
        puts("[-] denied");
    return 0;
}
