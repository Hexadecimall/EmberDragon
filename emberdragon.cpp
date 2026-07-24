// emberdragon — native launcher (replaces the old bash script so it runs on Windows too).
// Auto-selects ember-arm64 (arm64) / ember-lift (x86-64) by Mach-O cputype, runs the
// lift -> collapse -> (optional AI) -> export pipeline, writes decomp/{c,cpp,asm,headers}/ +
// manifest.json. ALL file I/O is native C++ (no cat/cp/mkdir/python shellouts), so it is portable.
//   emberdragon <binary> [--ai none|api|local] [--collapse] [--asm] [--export [--out dir]]
// build:  clang++ -std=c++17 -O2 emberdragon.cpp -o emberdragon                (mac)
//         pullio --gcc cxx Win64 -i emberdragon.cpp -o emberdragon.exe         (Windows)
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#ifdef _WIN32
  #include <direct.h>
  #include <windows.h>
  #ifndef popen
    #define popen _popen
    #define pclose _pclose
  #endif
  static const char PSEP = '\\';
#else
  #include <unistd.h>
  #include <cstdlib>
  static const char PSEP = '/';
#endif
#ifdef __APPLE__
  #include <mach-o/dyld.h>     // _NSGetExecutablePath
#endif
using std::string;

static string shq(const string& s) {
#ifdef _WIN32
    string o = "\""; for (char c : s) { if (c == '"') o += "\\\""; else o += c; } return o + "\"";   // cmd.exe
#else
    string o = "'"; for (char c : s) { if (c == '\'') o += "'\\''"; else o += c; } return o + "'";
#endif
}
static string readFile(const string& p) { FILE* f = fopen(p.c_str(), "rb"); if (!f) return ""; string s; char b[65536]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n); fclose(f); return s; }
static bool writeFile(const string& p, const string& s) { FILE* f = fopen(p.c_str(), "wb"); if (!f) return false; fwrite(s.data(), 1, s.size(), f); fclose(f); return true; }
static void mkdir1(const string& p) {
#ifdef _WIN32
    _mkdir(p.c_str());
#else
    mkdir(p.c_str(), 0755);
#endif
}
static void mkdirs(const string& p) { string c; for (char ch : p) { c += ch; if ((ch == '/' || ch == '\\') && c.size() > 1) mkdir1(c); } mkdir1(p); }
static string runCapture(const string& cmd) { FILE* f = popen(cmd.c_str(), "r"); if (!f) return ""; string s; char b[65536]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n); pclose(f); return s; }
static string baseName(const string& p) { size_t s = p.find_last_of("/\\"); return s == string::npos ? p : p.substr(s + 1); }
static string dirName(const string& p) { size_t s = p.find_last_of("/\\"); return s == string::npos ? string(".") : p.substr(0, s); }
static bool isExecFile(const string& p) { struct stat st; return stat(p.c_str(), &st) == 0 && (st.st_mode & S_IFREG); }

// Resolve the directory that holds emberdragon's sibling engines (ember-arm64, …).
// argv[0] alone breaks when launched via PATH (no slash -> "."). So: prefer the real
// executable path; if argv[0] has a slash use its dir; else search $PATH for the engine.
static string engineDir(const char* a0) {
    string self;
#if defined(__APPLE__)
    char buf[4096]; uint32_t sz = sizeof buf;
    if (_NSGetExecutablePath(buf, &sz) == 0) { char rp[4096]; self = realpath(buf, rp) ? rp : buf; }
#elif defined(_WIN32)
    char buf[4096]; if (GetModuleFileNameA(NULL, buf, sizeof buf)) self = buf;
#elif defined(__linux__)
    char buf[4096]; ssize_t n = readlink("/proc/self/exe", buf, sizeof buf - 1); if (n > 0) { buf[n] = 0; self = buf; }
#endif
    if (!self.empty()) { string d = dirName(self); if (!d.empty() && d.back() != '/' && d.back() != '\\') d += PSEP; return d; }
    string a = a0 ? a0 : "";
    if (a.find_first_of("/\\") != string::npos) { string d = dirName(a); if (!d.empty() && d.back() != '/' && d.back() != '\\') d += PSEP; return d; }
#ifndef _WIN32
    if (const char* path = getenv("PATH")) { string P = path, seg; for (size_t i = 0; i <= P.size(); i++) {   // bare name via PATH: find the dir that holds an engine
        if (i == P.size() || P[i] == ':') { if (!seg.empty()) { string c = seg + "/ember-arm64"; if (isExecFile(c)) return seg + "/"; } seg.clear(); }
        else seg += P[i]; } }
#endif
    return string(".") + PSEP;
}

// prototypes from the decompiled C (replaces the old python step): `<type> name(...) {` -> `<type> name(...);`
static string genHeaders(const string& c, const string& name) {
    string h = "// " + name + ".h — recovered prototypes\n#pragma once\n\n"; size_t i = 0;
    while (i < c.size()) { size_t e = c.find('\n', i); string ln = c.substr(i, e == string::npos ? string::npos : e - i); i = (e == string::npos) ? c.size() : e + 1;
        if (!ln.empty() && (isalpha((unsigned char)ln[0]) || ln[0] == '_') && ln.find('(') != string::npos && ln.size() > 2 && ln.compare(ln.size() - 3, 3, ") {") == 0)
            h += ln.substr(0, ln.size() - 2) + ";\n"; }
    return h;
}

int main(int argc, char** argv) {
    string bin, mode = "none", out_dir; bool want_asm = false, want_collapse = false, want_export = false;
    for (int i = 1; i < argc; i++) { string a = argv[i];
        if (a == "--ai" && i + 1 < argc) mode = argv[++i];
        else if (a.rfind("--ai=", 0) == 0) mode = a.substr(5);
        else if (a == "--asm" || a == "--disasm") want_asm = true;
        else if (a == "--collapse") want_collapse = true;
        else if (a == "--export") want_export = true;
        else if (a == "--out" && i + 1 < argc) out_dir = argv[++i];
        else if (a == "--struct-widths") {                          // opt-in: precise struct-array field widths (best-effort, warns)
#ifdef _WIN32
            _putenv_s("EMBER_STRUCT_WIDTHS", "1");
#else
            setenv("EMBER_STRUCT_WIDTHS", "1", 1);                  // inherited by the ember-collapse child
#endif
            fprintf(stderr, "ember: --struct-widths enabled (experimental: struct field types are inferred from observed accesses; verify before relying on them).\n");
        }
        else if (!a.empty() && a[0] != '-') bin = a;
    }
    if (bin.empty()) { fprintf(stderr, "usage: emberdragon <binary> [--ai none|api|local] [--collapse] [--asm] [--struct-widths] [--export [--out dir]]\n"); return 2; }
    string here = engineDir(argv[0]);   // robust: real exe path / argv[0] dir / $PATH search (works when run as a global command)
    // arch from Mach-O cputype (bytes 4..7, little-endian): arm64 = 0x0100000c, else treat as x86-64
    string head = readFile(bin); uint32_t cpu = 0; if (head.size() >= 8) cpu = (uint8_t)head[4] | ((uint8_t)head[5] << 8) | ((uint8_t)head[6] << 16) | ((uint32_t)(uint8_t)head[7] << 24);
    bool arm = (cpu == 0x0100000c);
    // fat/universal Mach-O (magic ca fe ba be|bf, big-endian): scan slices, prefer the arm64 one when present.
    if (head.size() >= 8 && (uint8_t)head[0]==0xca && (uint8_t)head[1]==0xfe && (uint8_t)head[2]==0xba && ((uint8_t)head[3]==0xbe || (uint8_t)head[3]==0xbf)) {
        bool fat64 = (uint8_t)head[3]==0xbf; uint32_t n = ((uint8_t)head[4]<<24)|((uint8_t)head[5]<<16)|((uint8_t)head[6]<<8)|(uint8_t)head[7];
        size_t entSz = fat64?32:20, p=8; bool hasArm=false;
        for (uint32_t i=0;i<n && p+entSz<=head.size(); i++, p+=entSz) {
            uint32_t c = ((uint8_t)head[p]<<24)|((uint8_t)head[p+1]<<16)|((uint8_t)head[p+2]<<8)|(uint8_t)head[p+3];
            if (c==0x0100000c) hasArm=true; }
        arm = hasArm; }
    string engine = here + (arm ? "ember-arm64" : "ember-lift"), disengine = here + (arm ? "ember-arm64" : "ember-dis"), disflag = arm ? " --disasm" : "", collapse = here + "ember-collapse", name = baseName(bin);

    if (want_export) {
        if (out_dir.empty()) out_dir = "decomp";
        for (const char* sub : { "/c", "/cpp", "/asm", "/headers" }) mkdirs(out_dir + sub);
        string rawf = out_dir + "/c/" + name + ".c.raw"; writeFile(rawf, runCapture(shq(engine) + " " + shq(bin)));   // stage 0: lift (progress -> our stderr)
        string finalc = readFile(rawf);
        if (want_collapse) { string s = runCapture(shq(collapse) + " " + shq(rawf)); if (!s.empty()) finalc = s; }    // stage 1: de-bloat (file arg = shell-free, works on NXRT)
        if (mode == "api" || mode == "local") {                                                    // stage 2: AI pass — decompiled code passes THROUGH the AI
            string sf = out_dir + "/c/" + name + ".c.stage"; writeFile(sf, finalc);
            string cmd = (mode == "api")
                ? (shq(here + "ember-ai") + " < " + shq(sf))                  // Claude via API (premium; needs ANTHROPIC_API_KEY)
                : (shq(here + "ember-apple") + " --rewrite < " + shq(sf));    // Apple on-device foundation model (local, free, private)
            string s = runCapture(cmd); if (!s.empty()) finalc = s; }
        writeFile(out_dir + "/c/" + name + ".c", finalc);
        writeFile(out_dir + "/cpp/" + name + ".cpp", finalc);
        writeFile(out_dir + "/asm/" + name + ".s", runCapture(shq(disengine) + disflag + " " + shq(bin)));
        writeFile(out_dir + "/headers/" + name + ".h", genHeaders(finalc, name));
        writeFile(out_dir + "/manifest.json", "{\n  \"name\": \"" + name + "\",\n  \"arch\": \"" + (arm ? "arm64" : "x86-64") + "\"\n}\n");
        fprintf(stderr, "exported %s [%s] -> %s\n", name.c_str(), arm ? "arm64" : "x86-64", out_dir.c_str());
        return 0;
    }
    string out = runCapture(shq(engine) + " " + shq(bin));
    if (want_asm) out = runCapture(shq(disengine) + disflag + " " + shq(bin));
    else if (want_collapse) { string rf = dirName(bin) + "/.ed_raw"; writeFile(rf, out); string s = runCapture(shq(collapse) + " " + shq(rf)); remove(rf.c_str()); if (!s.empty()) out = s; }
    fputs(out.c_str(), stdout);
    return 0;
}
