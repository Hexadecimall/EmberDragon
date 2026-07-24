/* crackme09  --  level 9 / 10
 * concept: the validator is a tiny bytecode VM interpreting an embedded
 *          program. you must disassemble the program (opcode table is in the
 *          interpreter) to learn what each input byte must be.
 *
 *   ops:  01 i  LOAD acc=input[i]      02 n  XOR acc^=n
 *         03 n  ADD acc+=n             04 n  ROL acc=rol8(acc,n)
 *         05 n  CMP fail if acc!=n     FF    HALT
 */
#include <stdio.h>
#include <string.h>

static const unsigned char program[] = { 1, 0, 2, 109, 3, 17, 4, 3, 5, 98, 1, 1, 2, 109, 3, 17, 4, 3, 5, 137, 1, 2, 2, 109, 3, 17, 4, 3, 5, 144, 1, 3, 2, 109, 3, 17, 4, 3, 5, 83, 1, 4, 2, 109, 3, 17, 4, 3, 5, 160, 1, 5, 2, 109, 3, 17, 4, 3, 5, 208, 1, 6, 2, 109, 3, 17, 4, 3, 5, 234, 255 };

static unsigned char rol8(unsigned char v, int n) {
    n &= 7; return (unsigned char)((v << n) | (v >> (8 - n)));
}

int main(void) {
    unsigned char in[64];
    printf("crackme09 :: key: ");
    if (!fgets((char*)in, sizeof in, stdin)) return 1;
    in[strcspn((char*)in, "\n")] = 0;
    if (strlen((char*)in) != 7) { puts("[-] denied"); return 0; }

    unsigned char acc = 0;
    int fail = 0;
    for (size_t pc = 0; pc < sizeof program; ) {
        unsigned char op = program[pc++];
        if (op == 0xFF) break;
        unsigned char arg = program[pc++];
        switch (op) {
            case 0x01: acc = in[arg];            break;
            case 0x02: acc ^= arg;               break;
            case 0x03: acc = (acc + arg) & 0xFF; break;
            case 0x04: acc = rol8(acc, arg);     break;
            case 0x05: if (acc != arg) fail = 1; break;
            default:   fail = 1;                 break;
        }
    }

    puts(!fail ? "[+] granted -- NXRT{l09_vm_byt3c0d3_r3v3rs1ng}" : "[-] denied");
    return 0;
}
