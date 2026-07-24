/* crackme10  --  level 10 / 10  :: FINAL
 * concept: everything at once, plus the trick that defeats lazy cracking --
 *          the success message is ENCRYPTED with a keystream derived from your
 *          key (and from an integrity checksum of an embedded table).
 *
 *          forcing the branch with a patch does NOT reveal the flag: a wrong
 *          key -> wrong keystream -> garbage output. tampering with the table
 *          changes the integrity byte -> also garbage. the only winning move
 *          is to recover the real key (keygen the invertible transform) and
 *          run the UNMODIFIED binary.
 */
#include <stdio.h>
#include <string.h>

static const unsigned char req[] = { 20, 63, 35, 35, 211, 213, 248, 214 };
static const unsigned char table[] = { 200, 243, 150, 185, 92, 7, 42, 205, 240, 155, 190, 97, 4, 47, 210, 245 };
static const unsigned char cipher[] = { 56, 63, 59, 43, 10, 57, 119, 8, 17, 52, 68, 94, 94, 72, 96, 63, 100, 114, 120, 155, 146, 186, 134, 201, 170, 172, 185, 190, 199, 218, 225, 183, 166, 183, 237, 253, 232, 16, 27 };

int main(void) {
    unsigned char k[64];
    printf("crackme10 :: key (8 chars): ");
    if (!fgets((char*)k, sizeof k, stdin)) return 1;
    k[strcspn((char*)k, "\n")] = 0;

    /* integrity: checksum of the embedded validation table */
    unsigned char integ = 0;
    for (size_t i = 0; i < sizeof table; i++) integ = (integ * 31 + table[i]) & 0xFF;

    /* stage 1: invertible per-byte transform must match the required table */
    int ok = (strlen((char*)k) == sizeof req);
    if (ok)
        for (size_t i = 0; i < sizeof req; i++)
            if ((((k[i] + (int)i * 13) ^ 0x5A) & 0xFF) != req[i]) ok = 0;

    /* stage 2: decrypt the flag with a keystream built from the key + integrity.
       note this runs regardless -- the lock is in the math, not the branch. */
    char out[sizeof cipher + 1];
    for (size_t j = 0; j < sizeof cipher; j++)
        out[j] = cipher[j] ^ (k[j % sizeof req] ^ integ ^ (((int)j * 7) & 0xFF));
    out[sizeof cipher] = 0;

    if (ok) {
        printf("[+] granted -- %s\n", out);
    } else {
        puts("[-] denied");
    }
    return 0;
}
