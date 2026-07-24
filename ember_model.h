// ember_model.h — the data behind the GUI views. Loads a binary (reusing
// ember.h's Mach-O readers for symbols/sections/bytes), runs `emberdragon` for
// pseudocode + disassembly, and derives the function list, strings, and the
// cross-reference (call) graph. No OpenGL here — pure data.
#ifndef EMBER_MODEL_H
#define EMBER_MODEL_H
#include "ember.h"
#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <cxxabi.h>      // demangle C++ symbols so func.addr maps for C++ binaries (mangled labels)
#if defined(_WIN32)
  #include <cstdlib>
  #ifndef popen
    #define popen _popen
    #define pclose _pclose
  #endif
#endif

namespace model {
using std::string; using std::vector;

struct Sym { uint64_t addr; string name; int kind; };           // kind 0 fn, 1 data
struct Str { uint64_t addr; string text; };
struct Func { string name, sig; int kind = 0; vector<string> lines; uint64_t addr = 0; vector<int> callees, callers; };

inline string shq(const string& s) {   // shell-quote a path/arg (spaces, quotes, $, …) for popen
#ifdef _WIN32
    string o = "\""; for (char c : s) { if (c == '"') o += "\\\""; else o += c; } return o + "\"";   // cmd.exe: double-quotes (single-quotes are literal there)
#else
    string o = "'"; for (char c : s) { if (c == '\'') o += "'\\''"; else o += c; } return o + "'";
#endif
}
inline string basename(const string& p) { size_t s = p.find_last_of('/'); return s == string::npos ? p : p.substr(s + 1); }
inline string trim(const string& s) { size_t a = s.find_first_not_of(" \t"); if (a == string::npos) return ""; size_t b = s.find_last_not_of(" \t"); return s.substr(a, b - a + 1); }

inline string runCmd(const string& cmd) {
    FILE* p = popen(cmd.c_str(), "r"); string out; if (!p) return out;
    char b[8192]; size_t n; while ((n = fread(b, 1, sizeof b, p)) > 0) { out.append(b, n); if (out.size() > 64u * 1024 * 1024) break; }  // cap runaway output
    pclose(p); return out;
}
// like runCmd but invokes onLine() per output line as it arrives (for live progress on a worker thread)
template <class F> inline void runCmdStream(const string& cmd, F onLine) {
    FILE* p = popen(cmd.c_str(), "r"); if (!p) return;
    char b[8192]; string acc;
    while (fgets(b, sizeof b, p)) { acc += b; if (!acc.empty() && acc.back() == '\n') { acc.pop_back(); onLine(acc); acc.clear(); } }
    if (!acc.empty()) onLine(acc);
    pclose(p);
}
inline string readText(const string& path) { FILE* f = fopen(path.c_str(), "rb"); if (!f) return ""; string s; char b[8192]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n); fclose(f); return s; }

struct Binary {
    string path, name, arch = "?"; bool loaded = false, cppOut = false;   // cppOut = the decompilation is C++ (classes/std::/cout) vs plain C
    vector<uint8_t> bytes;
    vector<Sym> syms;
    vector<Str> strings;
    vector<Func> funcs;
    string pseudo, disasm;
    vector<string> log;
    string dir;     // directory holding the emberdragon launcher
    uint64_t textVaddr = 0; size_t textOff = 0, textSize = 0;   // .text mapping: file offset = addr - textVaddr + textOff
    string disasmSource;        // if set, disassemble THIS file (a temp of the patched bytes) instead of the original
    // virtual address -> file byte offset (for asm<->bytes linkage); SIZE_MAX if outside .text
    size_t addrToOffset(uint64_t addr) const { if (textSize && addr >= textVaddr && addr < textVaddr + textSize) return textOff + (size_t)(addr - textVaddr); return SIZE_MAX; }

    void logmsg(const string& m) { log.push_back(m); }

    static string sniffArch(const vector<uint8_t>& h) {
        if (h.size() < 8) return "?";
        auto u32 = [&](int o) { return (unsigned)h[o] | (h[o+1] << 8) | (h[o+2] << 16) | ((unsigned)h[o+3] << 24); };
        unsigned m = u32(0), ct = u32(4);
        if (m == 0xfeedfacf || m == 0xfeedface) { if (ct == 0x01000007) return "x86-64"; if (ct == 0x0100000c) return "arm64"; if (ct == 7) return "x86"; if (ct == 12) return "arm"; return "mach-o"; }
        if (m == 0xcafebabe || m == 0xbebafeca) return "universal";
        if (h.size() > 1 && h[0] == 'M' && h[1] == 'Z') return "PE";
        if (h.size() > 3 && h[0] == 0x7f && h[1] == 'E' && h[2] == 'L' && h[3] == 'F') return "elf";
        return "?";
    }

    // split decompiler text into top-level brace blocks -> functions/classes/structs
    void parseFunctions(const string& text) {
        funcs.clear();
        vector<string> lines; { size_t i = 0; while (i < text.size()) { size_t e = text.find('\n', i); lines.push_back(text.substr(i, e == string::npos ? string::npos : e - i)); if (e == string::npos) break; i = e + 1; } }
        auto nameOf = [](const string& l) -> string { string t = trim(l);
            if (t.rfind("class ", 0) == 0 || t.rfind("struct ", 0) == 0) { size_t s = t.find(' ') + 1, e = t.find_first_of(" {:", s); return t.substr(s, e - s); }
            size_t par = t.find('('); if (par != string::npos) { size_t s = t.find_last_of(" *", par); return t.substr(s == string::npos ? 0 : s + 1, par - (s == string::npos ? 0 : s + 1)); }
            return t; };
        Func glob; glob.name = "(data / globals)"; glob.sig = "data & globals"; glob.kind = 3;
        int depth = 0; Func cur; bool in = false;
        for (auto& l : lines) { int op = 0, cl = 0; for (char c : l) { if (c == '{') op++; else if (c == '}') cl++; }
            if (!in && depth == 0 && op > cl && l.find('{') != string::npos) { cur = Func(); cur.name = nameOf(l); cur.sig = trim(l); if (!cur.sig.empty() && cur.sig.back() == '{') { cur.sig.pop_back(); cur.sig = trim(cur.sig); }
                string tl = trim(l); cur.kind = tl.rfind("class ", 0) == 0 ? 1 : tl.rfind("struct ", 0) == 0 ? 2 : 0; cur.lines.push_back(l); in = true; depth += op - cl; continue; }
            if (in) { cur.lines.push_back(l); depth += op - cl; if (depth <= 0) { funcs.push_back(cur); in = false; depth = 0; } continue; }
            if (!trim(l).empty()) glob.lines.push_back(l);
        }
        if (!glob.lines.empty()) funcs.insert(funcs.begin(), glob);
    }

    static bool identChar(char c) { return std::isalnum((unsigned char)c) || c == '_'; }
    // build call graph: a call site is an identifier immediately followed by '('
    void buildXrefs() {
        for (auto& f : funcs) { f.callees.clear(); f.callers.clear(); }
        for (size_t fi = 0; fi < funcs.size(); fi++) {
            for (auto& l : funcs[fi].lines) {
                for (size_t i = 0; i < l.size();) {
                    if (std::isalpha((unsigned char)l[i]) || l[i] == '_') { size_t j = i; while (j < l.size() && identChar(l[j])) j++;
                        // allow operator<< style names
                        while (j < l.size() && (l[j] == '<' || l[j] == '>' || l[j] == '=' || l[j] == '!' || l[j] == '+' || l[j] == '-')) j++;
                        if (j < l.size() && l[j] == '(') { string nm = l.substr(i, j - i);
                            for (size_t k = 0; k < funcs.size(); k++) if (k != fi && funcs[k].name == nm && funcs[k].kind == 0) {
                                if (std::find(funcs[fi].callees.begin(), funcs[fi].callees.end(), (int)k) == funcs[fi].callees.end()) funcs[fi].callees.push_back((int)k);
                                if (std::find(funcs[k].callers.begin(), funcs[k].callers.end(), (int)fi) == funcs[k].callers.end()) funcs[k].callers.push_back((int)fi);
                            }
                        }
                        i = j;
                    } else i++;
                }
            }
        }
    }

    void scanStrings() {
        strings.clear(); vector<nx::Section> secs; nx::machoSections(bytes.data(), bytes.size(), secs);
        for (auto& s : secs) {
            bool textseg = s.name.find("cstring") != string::npos || s.name.find("const") != string::npos || s.name.find("cfstring") != string::npos || s.name.find("data") != string::npos || s.name.find("__os_log") != string::npos;
            if (!textseg || s.fileoff == 0 || s.size == 0) continue;
            size_t end = std::min(s.fileoff + (size_t)s.size, bytes.size());
            string cur; uint64_t startVa = 0;
            for (size_t i = s.fileoff; i < end; i++) { unsigned char c = bytes[i];
                if (c >= 32 && c < 127) { if (cur.empty()) startVa = s.vaddr + (i - s.fileoff); cur += (char)c; }
                else { if (cur.size() >= 4) strings.push_back({ startVa, cur }); cur.clear(); }
            }
            if (cur.size() >= 4) strings.push_back({ startVa, cur });
        }
        std::sort(strings.begin(), strings.end(), [](const Str& a, const Str& b) { return a.addr < b.addr; });
    }

    void loadSymbols() {
        syms.clear(); std::unordered_map<uint64_t, string> m; nx::machoSymbols(bytes.data(), bytes.size(), m);
        for (auto& kv : m) syms.push_back({ kv.first, kv.second, 0 });
        std::sort(syms.begin(), syms.end(), [](const Sym& a, const Sym& b) { return a.addr < b.addr; });
    }

    // Link each decompiled function to its virtual address — the foundation for every address-based
    // feature (patch-from-pseudocode, the per-line asm/hex map, relocation-aware patching). Primary
    // source: the disassembly's `name:` labels (clean names, 1:1 with the pseudocode); fallback: the
    // symbol table (stripping the Mach-O leading '_'). Runs at load, while func names == original symbols.
    // demangle a (possibly C++-mangled) symbol to its bare function name: `_Z11installMainv` -> `installMain`,
    // `__Z10bruteForce...` -> `bruteForce`. Non-mangled names (C `main`) pass through unchanged.
    static string demangleBase(const string& sym) {
        string s = sym; if (s.size() > 2 && s[0] == '_' && s[1] == '_' && s[2] == 'Z') s = s.substr(1);   // Mach-O __Z.. -> _Z..
        int st = 0; char* d = abi::__cxa_demangle(s.c_str(), nullptr, nullptr, &st);
        string full = (st == 0 && d) ? string(d) : sym; if (d) free(d);
        size_t par = full.find('('); if (par != string::npos) full = full.substr(0, par);   // drop (args)
        while (!full.empty() && full.back() == ' ') full.pop_back();
        if (full.find('<') == string::npos) { size_t sp = full.rfind(' '); if (sp != string::npos) full = full.substr(sp + 1); }   // drop a return type
        size_t cc = full.rfind("::"); if (cc != string::npos) full = full.substr(cc + 2);    // drop namespace/class qualifier
        return full;
    }
    void mapFuncAddrs() {
        std::unordered_map<string, uint64_t> byName;
        auto add = [&](const string& nm, uint64_t a) { if (nm.empty()) return; if (!byName.count(nm)) byName[nm] = a;   // index by raw AND demangled-base (C++ symbols/labels are mangled)
            string db = demangleBase(nm); if (!db.empty() && db != nm && !byName.count(db)) byName[db] = a; };
        if (!trim(disasm).empty()) {                                  // (a) disasm labels: a "name:" line, then the next "<hex>:" line is its address
            string pend; size_t i = 0;
            while (i < disasm.size()) { size_t e = disasm.find('\n', i); string ln = disasm.substr(i, e == string::npos ? string::npos : e - i); i = (e == string::npos) ? disasm.size() : e + 1;
                string t = trim(ln); if (t.empty()) continue;
                size_t c = 0; while (c < t.size() && std::isxdigit((unsigned char)t[c])) c++;
                if (c > 0 && c < t.size() && t[c] == ':') { if (!pend.empty()) { add(pend, strtoull(t.substr(0, c).c_str(), nullptr, 16)); pend.clear(); } }   // addressed instruction
                else if (t.size() > 1 && t.back() == ':') pend = t.substr(0, t.size() - 1);   // a bare "name:" label
            }
        }
        for (auto& s : syms) { string n = s.name; if (!n.empty() && n[0] == '_') n = n.substr(1); add(n, s.addr); }   // (b) symbol-table fallback
        for (auto& f : funcs) { if (f.addr) continue; auto it = byName.find(f.name); if (it != byName.end()) f.addr = it->second; }
    }

    // Load from the on-disk export (the app runs `emberdragon --export --out <decompDir>` first).
    // Symbols/strings/hex come from the source binary; pseudocode/disasm come from the generated files.
    void load(const string& launcherDir, const string& sourcePath, const string& decompDir) {
        *this = Binary(); dir = launcherDir; path = sourcePath; name = basename(sourcePath);
        if (!nx::readFile(sourcePath.c_str(), bytes)) { logmsg("error: cannot read " + sourcePath); return; }
        arch = sniffArch(bytes);
        logmsg("loaded " + name + "  (" + arch + ", " + std::to_string(bytes.size()) + " bytes)");
        { size_t toff = 0, tsz = 0; uint64_t tva = 0;   // .text mapping for asm<->bytes linkage
          if (nx::machoText(bytes.data(), bytes.size(), toff, tsz, tva) || nx::peText(bytes.data(), bytes.size(), toff, tsz, tva)) { textOff = toff; textSize = tsz; textVaddr = tva; } }
        loadSymbols(); logmsg(std::to_string(syms.size()) + " symbols");
        scanStrings();  logmsg(std::to_string(strings.size()) + " strings");
        pseudo = readText(decompDir + "/cpp/" + name + ".cpp"); cppOut = !trim(pseudo).empty();   // prefer the C++ output (classes/std::/cout); fall back to plain C
        if (!cppOut) pseudo = readText(decompDir + "/c/" + name + ".c");
        disasm = readText(decompDir + "/asm/" + name + ".s");
        if (trim(pseudo).empty()) pseudo = runCmd(shq(dir + "emberdragon") + " " + shq(sourcePath) + " --ai none 2>/dev/null");  // fallback if export missing
        parseFunctions(pseudo); buildXrefs(); mapFuncAddrs();
        logmsg(std::to_string(funcs.size()) + " functions decompiled");
        loaded = true;
    }
    void runDisasm() {
        if (!trim(disasm).empty()) return;
        disasm = runCmd(shq(dir + "emberdragon") + " " + shq(disasmSource.empty() ? path : disasmSource) + " --asm 2>/dev/null");
        if (trim(disasm).empty()) disasm = "; (disassembly listing unavailable for this target)\n";
    }
};
} // namespace model
#endif
