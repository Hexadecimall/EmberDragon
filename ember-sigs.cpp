// ember-sigs — FLIRT-style library function identification for EmberDragon.
// Builds a "fat" signature database from known libraries, then identifies those
// functions inside a target binary by their byte-pattern — so stripped / statically
// linked code gets real names (memcpy, std::string::append, …) instead of sub_<addr>.
//
// A signature is the function's masked machine-code hash + length: position-dependent
// immediates (external BL targets, ADRP/ADR page refs, literal loads) are ZEROED so the
// same function fingerprints identically across binaries; the rest (the actual logic —
// arithmetic, internal control flow, constants) is hashed verbatim.
//
//   ember-sigs gen   <db> <macho…>     append signatures from each library into <db>
//   ember-sigs match <db> <macho>      print `<hex addr> <name>` for each identified fn
//
// build: clang++ -std=c++17 -O2 ember-sigs.cpp -o ember-sigs
#include "ember.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <cxxabi.h>
using std::string; using std::vector;

static int64_t sext(uint64_t v, int bits) { uint64_t m = 1ull << (bits - 1); return (int64_t)((v ^ m) - m); }

// zero the position-dependent immediate of one arm64 instruction so the same function
// hashes identically wherever it's linked. Keeps internal branches + real constants.
static uint32_t maskInsn(uint32_t w, uint64_t addr, uint64_t fstart, uint64_t fend) {
    if ((w & 0xFC000000u) == 0x94000000u) {                                   // BL <imm26> — a call; mask if it leaves this function
        uint64_t tgt = addr + ((uint64_t)(sext(w & 0x3ffffff, 26) << 2)); if (tgt < fstart || tgt >= fend) return w & 0xFC000000u; return w; }
    if ((w & 0xFC000000u) == 0x14000000u) {                                   // B <imm26> — keep if internal (control flow), mask external tail-call
        uint64_t tgt = addr + ((uint64_t)(sext(w & 0x3ffffff, 26) << 2)); if (tgt < fstart || tgt >= fend) return w & 0xFC000000u; return w; }
    if ((w & 0x1F000000u) == 0x10000000u) return w & 0x9F00001Fu;             // ADR / ADRP — page-relative data ref: keep op+Rd, zero immlo/immhi
    if ((w & 0x3B000000u) == 0x18000000u) return w & 0xFF0000FFu;             // LDR (literal) — zero the 19-bit literal offset
    return w;
}

struct Sig { uint64_t crc; uint32_t len; };
static Sig signFn(const uint8_t* code, uint64_t fstart, size_t len) {
    uint64_t h = 1469598103934665603ull;                                      // FNV-1a over the masked instruction stream
    for (size_t o = 0; o + 4 <= len; o += 4) {
        uint32_t w = code[o] | (code[o+1] << 8) | (code[o+2] << 16) | ((uint32_t)code[o+3] << 24);
        w = maskInsn(w, fstart + o, fstart, fstart + len);
        for (int b = 0; b < 4; b++) { h ^= (w >> (b * 8)) & 0xff; h *= 1099511628211ull; }
    }
    return { h, (uint32_t)len };
}

// a clean, valid-identifier function name from a (possibly C++-mangled, Mach-O `_`-prefixed) symbol.
// `_Z..`/`__Z..` -> demangled base (drop args/templates/namespace); `_fnv1a` -> `fnv1a`. "" if unusable.
static string cleanName(const string& s) {
    string full;
    for (const string& c : { s, (s.size() > 1 && s[0] == '_') ? s.substr(1) : string() }) {
        if (c.empty()) continue; int st = 0; char* d = abi::__cxa_demangle(c.c_str(), 0, 0, &st);
        if (st == 0 && d) { full = d; free(d); break; } if (d) free(d); }
    if (full.empty()) full = (s.size() > 1 && s[0] == '_') ? s.substr(1) : s;        // C name: drop the Mach-O _
    size_t par = full.find('('); if (par != string::npos) full = full.substr(0, par);  // drop (args)
    { string r; int d = 0; for (char c : full) { if (c == '<') d++; else if (c == '>') { if (d > 0) d--; } else if (d == 0) r += c; } full = r; }  // strip <templates>
    while (!full.empty() && full.back() == ' ') full.pop_back();
    if (full.find(' ') != string::npos) full = full.substr(full.rfind(' ') + 1);       // drop a return type
    size_t cc = full.rfind("::"); if (cc != string::npos) full = full.substr(cc + 2);  // base name (last :: segment)
    string id; for (char c : full) { if (isalnum((unsigned char)c) || c == '_') id += c; else if (c == '~') id += "dtor_"; }
    if (id.empty() || isdigit((unsigned char)id[0])) return "";
    return id;
}

// the in-order (addr,name) functions of a Mach-O whose code lives in __text, plus the __text bytes.
static bool funcsOf(const string& path, vector<std::pair<uint64_t,string>>& fns, vector<uint8_t>& bytes, size_t& toff, uint64_t& tva, size_t& tsz) {
    if (!nx::readFile(path.c_str(), bytes)) return false;
    if (!nx::machoText(bytes.data(), bytes.size(), toff, tsz, tva)) return false;
    std::unordered_map<uint64_t,string> syms; nx::machoSymbols(bytes.data(), bytes.size(), syms);
    for (auto& kv : syms) if (kv.first >= tva && kv.first < tva + tsz) fns.push_back(kv);
    std::sort(fns.begin(), fns.end());
    return !fns.empty();
}

static int gen(const string& db, vector<string> libs, const string& namesFile = "") {
    // load the signatures ALREADY in the DB so we NEVER override them — Analyze only ADDS what's new (so the
    // accumulated knowledge from past decompiles + other binaries is never clobbered by a fresh, weaker name).
    std::set<std::pair<uint64_t,uint32_t>> have;
    { FILE* in = fopen(db.c_str(), "r"); if (in) { unsigned long long c; unsigned l; char nm[512];
        while (fscanf(in, "%llx %u %511s", &c, &l, nm) == 3) have.insert({ (uint64_t)c, (uint32_t)l }); fclose(in); } }
    // optional `<addr> <name>` list (the GUI passes EVERY function's address+current name — incl. your renames —
    // so signatures use YOUR names, and stripped binaries with no symbols still contribute).
    std::map<uint64_t,string> nameList;
    if (!namesFile.empty()) { FILE* nf = fopen(namesFile.c_str(), "r"); if (nf) { unsigned long long a; char nm[512];
        while (fscanf(nf, "%llx %511s", &a, nm) == 2) nameList[(uint64_t)a] = nm; fclose(nf); } }
    FILE* out = fopen(db.c_str(), "a"); if (!out) { fprintf(stderr, "ember-sigs: cannot open %s\n", db.c_str()); return 1; }
    long total = 0;
    for (auto& lib : libs) {
        vector<uint8_t> bytes; size_t toff, tsz; uint64_t tva;
        if (!nx::readFile(lib.c_str(), bytes)) { fprintf(stderr, "  (skip %s)\n", lib.c_str()); continue; }
        nx::machoSelectSlice(bytes, 0x0100000c);                                 // fat -> arm64 slice (thin/x86 = no-op)
        if (!nx::machoText(bytes.data(), bytes.size(), toff, tsz, tva) && !nx::peText(bytes.data(), bytes.size(), toff, tsz, tva)) { fprintf(stderr, "  (skip %s — no .text)\n", lib.c_str()); continue; }   // Mach-O or PE
        vector<std::pair<uint64_t,string>> fns;
        if (!nameList.empty()) { for (auto& kv : nameList) if (kv.first >= tva && kv.first < tva + tsz) fns.push_back(kv); }
        else { std::unordered_map<uint64_t,string> syms; nx::machoSymbols(bytes.data(), bytes.size(), syms);
               for (auto& kv : syms) if (kv.first >= tva && kv.first < tva + tsz) fns.push_back(kv); }
        std::sort(fns.begin(), fns.end());
        for (size_t i = 0; i < fns.size(); i++) {
            uint64_t a = fns[i].first, e = (i + 1 < fns.size()) ? fns[i + 1].first : tva + tsz;
            if (e <= a || e - a < 8 || e - a > 1u << 20) continue;             // skip empty / runaway
            const uint8_t* code = bytes.data() + toff + (a - tva);
            Sig s = signFn(code, a, (size_t)(e - a)); string nm = cleanName(fns[i].second);
            if (nm.empty() || nm == "mh_execute_header" || nm.rfind("sub_", 0) == 0) continue;   // never sign a still-placeholder name
            if (have.count({ s.crc, s.len })) continue;                        // ALREADY KNOWN -> don't override
            fprintf(out, "%016llx %u %s\n", (unsigned long long)s.crc, s.len, nm.c_str()); have.insert({ s.crc, s.len }); total++;
        }
        fprintf(stderr, "  + %s\n", lib.c_str());
    }
    fclose(out); fprintf(stderr, "ember-sigs: added %ld NEW signatures -> %s\n", total, db.c_str());
    return 0;
}

static int match(const string& db, const string& target, const string& addrFile) {
    // load DB: crc:len -> name (drop ambiguous crc:len that map to >1 distinct name)
    std::map<std::pair<uint64_t,uint32_t>, string> sig2name; std::map<std::pair<uint64_t,uint32_t>, bool> amb;
    FILE* f = fopen(db.c_str(), "r"); if (!f) { fprintf(stderr, "ember-sigs: no db %s\n", db.c_str()); return 1; }
    char line[4096]; while (fgets(line, sizeof line, f)) { unsigned long long crc; unsigned len; char nm[4000];
        if (sscanf(line, "%llx %u %3999[^\n]", &crc, &len, nm) == 3) { auto k = std::make_pair((uint64_t)crc, (uint32_t)len);
            auto it = sig2name.find(k); if (it == sig2name.end()) sig2name[k] = nm; else if (it->second != nm) amb[k] = true; } }
    fclose(f);
    vector<uint8_t> bytes; if (!nx::readFile(target.c_str(), bytes)) { fprintf(stderr, "ember-sigs: cannot read %s\n", target.c_str()); return 1; }
    size_t toff, tsz; uint64_t tva; if (!nx::machoText(bytes.data(), bytes.size(), toff, tsz, tva)) { fprintf(stderr, "ember-sigs: no __text\n"); return 1; }
    // function starts: the decompiler's detected addresses (works on STRIPPED binaries) or the symbol table
    vector<uint64_t> starts;
    if (!addrFile.empty()) { FILE* af = fopen(addrFile.c_str(), "r"); if (af) { char l[64]; while (fgets(l, sizeof l, af)) { uint64_t a = strtoull(l, nullptr, 16); if (a >= tva && a < tva + tsz) starts.push_back(a); } fclose(af); } }
    else { std::unordered_map<uint64_t,string> syms; nx::machoSymbols(bytes.data(), bytes.size(), syms); for (auto& kv : syms) if (kv.first >= tva && kv.first < tva + tsz) starts.push_back(kv.first); }
    std::sort(starts.begin(), starts.end()); starts.erase(std::unique(starts.begin(), starts.end()), starts.end());
    int hits = 0;
    for (size_t i = 0; i < starts.size(); i++) {
        uint64_t a = starts[i], e = (i + 1 < starts.size()) ? starts[i + 1] : tva + tsz;
        if (e <= a || e - a < 8) continue;
        Sig s = signFn(bytes.data() + toff + (a - tva), a, (size_t)(e - a));
        auto k = std::make_pair(s.crc, s.len); auto it = sig2name.find(k);
        if (it != sig2name.end() && !amb.count(k)) { printf("%llx %s\n", (unsigned long long)a, it->second.c_str()); hits++; }
    }
    fprintf(stderr, "ember-sigs: identified %d function(s)\n", hits);
    return 0;
}

int main(int argc, char** argv) {
    if (argc >= 4 && string(argv[1]) == "gen") { vector<string> libs; string names;
        for (int i = 3; i < argc; i++) { string a = argv[i]; if (a == "--names" && i + 1 < argc) names = argv[++i]; else libs.push_back(a); }
        return gen(argv[2], libs, names); }
    if (argc >= 4 && string(argv[1]) == "match") return match(argv[2], argv[3], argc >= 5 ? argv[4] : "");
    fprintf(stderr, "usage:\n  ember-sigs gen   <db> <macho…>\n  ember-sigs match <db> <macho> [addrs.txt]\n");
    return 2;
}
