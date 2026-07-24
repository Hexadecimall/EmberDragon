# NXRT crackme ladder

Ten crackmes, each introducing one new reverse-engineering idea. Build, then
practice on the **binary** -- don't read the source until you've solved it (or
you want to check your work).

```sh
make            # build all, -O2
make strip      # strip symbols -> more realistic target
# solve one:
./crackme01
```

On macOS you'll get arm64 Mach-O targets; tools that pair well: `lldb`,
`otool -tV`, `nm`, `strings`, Hopper / Ghidra / your own Binary Ninja clone.
`crackme08` is the anti-debug one -- attaching `lldb` is the point.

## the ladder

| # | concept | technique to learn |
|---|---------|--------------------|
| 01 | plaintext compare | `strings`, find the literal |
| 02 | xor-obfuscated string | spot the decode loop, undo the xor |
| 03 | numeric serial, arithmetic | invert the math (first keygen) |
| 04 | per-char indexed transform | read the loop, recover char-by-char |
| 05 | name -> serial keygenme | reverse the hash, write a real keygen |
| 06 | constraint system (6 bytes) | solve by hand or with z3 |
| 07 | custom 32-bit hash | partial brute-force + solve last byte |
| 08 | anti-debug + obfuscated key | patch out the guard, then deobfuscate |
| 09 | bytecode VM | disassemble the embedded program |
| 10 | keygen + anti-patch + integrity | the flag is locked by the key -- patching won't reveal it; you must keygen |

## the level-10 twist

Levels 1-9 can be beaten by patching a branch if you just want the "granted"
line. Level 10 derives the success message from your key and from a checksum of
an embedded table, so a forced branch or a tampered binary prints garbage. The
only way to see the real flag is to recover the actual key and run the
untouched binary. That's the jump from *patcher* to *keygenner*.

Each solved level prints an `NXRT{...}` token. Stuck? Ask and I'll hand over
hints, a walkthrough, or reference keygens for any level.
