/* crackme08  --  level 8 / 10
 * concept: the binary refuses to run under a debugger (PT_DENY_ATTACH on
 *          macOS, PTRACE_TRACEME on linux). the real check is easy once you
 *          neutralize the anti-debug: NOP the ptrace call / patch the branch,
 *          then deobfuscate the key.
 */
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#  include <sys/ptrace.h>
#  ifndef PT_DENY_ATTACH
#    define PT_DENY_ATTACH 31
#  endif
#elif defined(__linux__)
#  include <sys/ptrace.h>
#endif

static void guard(void) {
#ifdef __APPLE__
    ptrace(PT_DENY_ATTACH, 0, 0, 0);
#elif defined(__linux__)
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
        puts("[-] no debuggers allowed");
        _Exit(1);
    }
#endif
}

static const unsigned char enc[] = { 83, 4, 85, 66, 80, 104, 90, 82, 104, 89, 7, 67 };

int main(void) {
    guard();

    char buf[64], key[sizeof enc + 1];
    for (size_t i = 0; i < sizeof enc; i++) key[i] = enc[i] ^ 0x37;
    key[sizeof enc] = 0;

    printf("crackme08 :: password: ");
    if (!fgets(buf, sizeof buf, stdin)) return 1;
    buf[strcspn(buf, "\n")] = 0;

    if (strcmp(buf, key) == 0)
        puts("[+] granted -- NXRT{l08_p4tch_0ut_th3_guard}");
    else
        puts("[-] denied");
    return 0;
}
