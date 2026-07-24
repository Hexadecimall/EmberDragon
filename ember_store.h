// ember_store.h — on-disk project storage under macOS Application Support.
// Layout (confirmed):
//   ~/Library/Application Support/EmberDragon/
//     recent.json · last-session.json
//     projects/<binary>/
//       project.json                       (source path, view, timestamps)
//       decomp/{c,cpp,asm,headers}/  manifest.json
//       optimized/{c,cpp,asm,headers}/ manifest.json   (written by the optimize pass)
#ifndef EMBER_STORE_H
#define EMBER_STORE_H
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <sys/stat.h>
#include <dirent.h>
#ifdef _WIN32
  #include <direct.h>
#endif

namespace store {
using std::string; using std::vector;

inline string home() {
#ifdef _WIN32
    const char* h = getenv("USERPROFILE"); if (h && *h) return h;
    const char* d = getenv("HOMEDRIVE"), * p = getenv("HOMEPATH"); if (d && p) return string(d) + p;
    return ".";
#else
    const char* h = getenv("HOME"); return h ? h : ".";
#endif
}
inline string root() {
#ifdef _WIN32
    const char* a = getenv("APPDATA"); return (a && *a ? string(a) : home()) + "/EmberDragon";   // %APPDATA%\EmberDragon
#else
    return home() + "/Library/Application Support/EmberDragon";
#endif
}
inline string projectsRoot() { return root() + "/projects"; }
inline string base(const string& p) { size_t s = p.find_last_of("/\\"); return s == string::npos ? p : p.substr(s + 1); }   // split on / AND \ (Windows paths)
// macOS treats ANY folder ending in a bundle extension (.app/.framework/.bundle/...) as an application,
// so a project dir named e.g. "Terminal.app" shows up as a phantom app in Launchpad/Spotlight. Replace the
// extension's leading dot with '_' so the on-disk folder is never itself a bundle (display name is unaffected).
inline string projFolder(const string& name) {
    static const char* EXT[] = { ".app", ".framework", ".bundle", ".xpc", ".plugin", ".kext", ".appex", ".prefPane", ".qlgenerator", ".component" };
    string n = name; for (auto e : EXT) { string es = e; if (n.size() >= es.size() && n.compare(n.size() - es.size(), es.size(), es) == 0) { n[n.size() - es.size()] = '_'; break; } }
    return n;
}
inline string projectDir(const string& name) { return projectsRoot() + "/" + projFolder(name); }
inline string decompDir(const string& name) { return projectDir(name) + "/decomp"; }
inline string optimizedDir(const string& name) { return projectDir(name) + "/optimized"; }

inline string shq(const string& s) {
#ifdef _WIN32
    string o = "\""; for (char c : s) { if (c == '"') o += "\\\""; else o += c; } return o + "\"";   // cmd.exe: double-quote
#else
    string o = "'"; for (char c : s) { if (c == '\'') o += "'\\''"; else o += c; } return o + "'";
#endif
}
inline void mkdirs(const string& p) {
#ifdef _WIN32
    string cur; for (size_t i = 0; i <= p.size(); i++) { if (i == p.size() || p[i] == '/' || p[i] == '\\') { if (cur.size() > 1 && !(cur.size() == 2 && cur[1] == ':')) _mkdir(cur.c_str()); } if (i < p.size()) cur += p[i]; }   // recursive _mkdir (no `mkdir -p` on cmd.exe)
#else
    if (system(("mkdir -p " + shq(p)).c_str())) {}
#endif
}
inline bool exists(const string& p) { struct stat st; return stat(p.c_str(), &st) == 0; }
inline bool isDir(const string& p) { struct stat st; return stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode); }
inline long fileSize(const string& p) { struct stat st; return stat(p.c_str(), &st) == 0 ? (long)st.st_size : -1; }
inline string readFile(const string& p) { std::ifstream f(p, std::ios::binary); return string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()); }
inline void writeFile(const string& p, const string& c) { std::ofstream f(p, std::ios::binary); f << c; }

// ── minimal JSON (we only emit/read flat string fields and string arrays) ────
inline string esc(const string& s) { string o; for (char c : s) { if (c == '"' || c == '\\') { o += '\\'; o += c; } else if (c == '\n') o += "\\n"; else o += c; } return o; }
// value of  "key": "value"   (first match; handles simple escapes)
inline string jsonStr(const string& js, const string& key) {
    size_t k = js.find("\"" + key + "\""); if (k == string::npos) return "";
    size_t c = js.find(':', k); if (c == string::npos) return ""; size_t q = js.find('"', c); if (q == string::npos) return "";
    string out; for (size_t i = q + 1; i < js.size(); i++) { char ch = js[i]; if (ch == '\\' && i + 1 < js.size()) { char n = js[++i]; out += (n == 'n') ? '\n' : n; } else if (ch == '"') break; else out += ch; }
    return out;
}
inline vector<string> jsonStrArray(const string& js) { vector<string> v; size_t i = js.find('['); if (i == string::npos) return v;
    for (i++; i < js.size() && js[i] != ']'; ) { if (js[i] == '"') { string s; for (i++; i < js.size(); i++) { if (js[i] == '\\' && i + 1 < js.size()) { s += js[++i]; } else if (js[i] == '"') break; else s += js[i]; } v.push_back(s); } i++; } return v; }

// ── recent + last session ────────────────────────────────────────────────────
inline vector<string> loadRecent() { return jsonStrArray(readFile(root() + "/recent.json")); }
inline void saveRecent(const vector<string>& paths) { mkdirs(root()); string j = "[\n"; for (size_t i = 0; i < paths.size() && i < 15; i++) j += "  \"" + esc(paths[i]) + "\"" + (i + 1 < paths.size() && i + 1 < 15 ? "," : "") + "\n"; j += "]\n"; writeFile(root() + "/recent.json", j); }
inline void addRecent(const string& path) { auto r = loadRecent(); r.erase(std::remove(r.begin(), r.end(), path), r.end()); r.insert(r.begin(), path); saveRecent(r); }
inline void saveLastSession(const string& name, const string& source) { mkdirs(root()); writeFile(root() + "/last-session.json", "{\n  \"project\": \"" + esc(name) + "\",\n  \"source\": \"" + esc(source) + "\"\n}\n"); }
// returns {projectName, sourcePath} or empty
inline std::pair<string, string> loadLastSession() { string js = readFile(root() + "/last-session.json"); return { jsonStr(js, "project"), jsonStr(js, "source") }; }

// settings (currently just the AI opt-in). AI is OFF unless the user explicitly enables it.
inline bool loadAiOptIn() { return jsonStr(readFile(root() + "/settings.json"), "aiOptIn") == "true"; }
inline void saveAiOptIn(bool on) { mkdirs(root()); writeFile(root() + "/settings.json", string("{\n  \"aiOptIn\": \"") + (on ? "true" : "false") + "\"\n}\n"); }

inline void writeProject(const string& name, const string& source, const string& arch, const string& view) {
    mkdirs(projectDir(name));
    writeFile(projectDir(name) + "/project.json",
        "{\n  \"name\": \"" + esc(name) + "\",\n  \"source\": \"" + esc(source) + "\",\n  \"arch\": \"" + esc(arch) + "\",\n  \"lastView\": \"" + esc(view) + "\"\n}\n");
}

// ── file tree (for the explorer) ─────────────────────────────────────────────
struct Node { string name, path; bool dir = false; long size = 0; vector<Node> children; bool expanded = true; };
inline void scanInto(Node& n, int depth) {
    if (!n.dir || depth > 8) return; DIR* d = opendir(n.path.c_str()); if (!d) return;
    vector<Node> dirs, files; struct dirent* e;
    while ((e = readdir(d))) { string nm = e->d_name; if (nm == "." || nm == "..") continue; string p = n.path + "/" + nm; Node c; c.name = nm; c.path = p; c.dir = isDir(p); c.size = c.dir ? 0 : fileSize(p); (c.dir ? dirs : files).push_back(c); }
    closedir(d);
    auto byName = [](const Node& a, const Node& b) { return a.name < b.name; };
    std::sort(dirs.begin(), dirs.end(), byName); std::sort(files.begin(), files.end(), byName);
    for (auto& c : dirs) { scanInto(c, depth + 1); n.children.push_back(c); }
    for (auto& c : files) n.children.push_back(c);
}
inline Node scanTree(const string& path, const string& label) { Node n; n.name = label; n.path = path; n.dir = isDir(path); scanInto(n, 0); return n; }
} // namespace store
#endif
