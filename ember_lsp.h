// ember_lsp.h — a minimal clangd LSP client for EmberDragon's editor: spawns the bundled clangd over
// pipes, speaks LSP JSON-RPC (Content-Length framing), and surfaces live diagnostics (errors/warnings)
// for the file in the editor. POSIX only (the LSP needs a real subprocess); a no-op stub on Windows so
// the cross-compile still builds. Single-header, dep-free (manual JSON build + a targeted diag parser).
#ifndef EMBER_LSP_H
#define EMBER_LSP_H
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#ifndef _WIN32
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <spawn.h>
extern char** environ;
#endif

namespace lsp {
using std::string; using std::vector;
struct TextEdit { int line = 0, sc = 0, eline = 0, ec = 0; string newText; };       // a replace [start..end] -> newText
struct Diag { int line = 0, col = 0, severity = 1; string msg; string fixTitle; vector<TextEdit> fix; };   // fix = clangd's inline quick-fix (empty if none)
struct Compl { string label, insert, detail; int kind = 0; };       // completion item (kind: 3=fn 5=field 6=var 7=class 21=const …)
struct Loc   { string path; int line = 0, col = 0; bool ok = false; };   // go-to-definition target

#ifndef _WIN32
struct Client {
    std::atomic<bool> alive{false};
    int inFd = -1, outFd = -1; pid_t pid = -1;
    std::thread reader;
    std::mutex mx;
    std::map<string, vector<Diag>> diags;          // uri -> diagnostics
    std::atomic<int> version{0}, epoch{0};         // epoch bumps when diagnostics change (UI repaints)
    string flags;                                   // compile flags for clangd's fallback
    std::atomic<int> reqSeq{100}, complEpoch{0}, defEpoch{0}, renameEpoch{0};
    std::mutex cmx; vector<Compl> complResult; int complAwaitId = -1; Loc defResult; int defAwaitId = -1;
    std::map<string, vector<TextEdit>> renameResult; int renameAwaitId = -1;
    bool isAlive() { return alive.load(); }

    static string jstr(const string& s) { string o = "\""; for (char c : s) {
        if (c == '"' || c == '\\') { o += '\\'; o += c; }
        else if (c == '\n') o += "\\n"; else if (c == '\t') o += "\\t"; else if (c == '\r') {}
        else if ((unsigned char)c < 0x20) {} else o += c; } return o + "\""; }
    static string uriOf(const string& path) { string u = "file://"; for (char c : path) {
        if (isalnum((unsigned char)c) || strchr("/-_.~", c)) u += c; else { char b[4]; snprintf(b, 4, "%%%02X", (unsigned char)c); u += b; } } return u; }

    void send(const string& body) { if (inFd < 0) return; string h = "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
        string m = h + body; ssize_t n = ::write(inFd, m.data(), m.size()); (void)n; }
    void notify(const string& method, const string& params) { send("{\"jsonrpc\":\"2.0\",\"method\":\"" + method + "\",\"params\":" + params + "}"); }
    void request(int id, const string& method, const string& params) { send("{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(id) + ",\"method\":\"" + method + "\",\"params\":" + params + "}"); }

    // start clangd (path) with compile fallback flags (e.g. "-std=c++17 -isysroot <sdk>")
    bool start(const string& clangd, const string& fallbackFlags) {
        if (alive.load()) return true; flags = fallbackFlags;
        int in[2], out[2]; if (pipe(in) || pipe(out)) return false;
        posix_spawn_file_actions_t fa; posix_spawn_file_actions_init(&fa);
        posix_spawn_file_actions_adddup2(&fa, in[0], 0);                       // clangd stdin  <- in
        posix_spawn_file_actions_adddup2(&fa, out[1], 1);                      // clangd stdout -> out
        posix_spawn_file_actions_addclose(&fa, in[1]); posix_spawn_file_actions_addclose(&fa, out[0]);
        const char* argv[] = { clangd.c_str(), "--log=error", "--background-index=false", "--limit-results=50", nullptr };
        int rc = posix_spawn(&pid, clangd.c_str(), &fa, nullptr, (char* const*)argv, environ);
        posix_spawn_file_actions_destroy(&fa); ::close(in[0]); ::close(out[1]);
        if (rc != 0) { ::close(in[1]); ::close(out[0]); return false; }
        inFd = in[1]; outFd = out[0]; alive = true;
        reader = std::thread([this] { readLoop(); });
        // LSP handshake: initialize -> initialized, with our fallback flags so clangd can parse standalone files
        string fb; { string f = flags; size_t p = 0; while (p < f.size()) { size_t q = f.find(' ', p); string tok = f.substr(p, q == string::npos ? string::npos : q - p);
            if (!tok.empty()) { if (!fb.empty()) fb += ","; fb += jstr(tok); } p = q == string::npos ? f.size() : q + 1; } }
        request(1, "initialize", "{\"processId\":null,\"rootUri\":null,\"capabilities\":{\"textDocument\":{\"publishDiagnostics\":{\"codeActionsInline\":true},\"completion\":{\"completionItem\":{\"snippetSupport\":false}}}},"
                                 "\"initializationOptions\":{\"fallbackFlags\":[" + fb + "]}}");
        notify("initialized", "{}");
        return true;
    }
    void stop() { if (!alive.exchange(false)) return; if (pid > 0) kill(pid, SIGTERM); if (inFd >= 0) ::close(inFd); inFd = -1;
        if (reader.joinable()) reader.detach(); if (pid > 0) { int st; waitpid(pid, &st, WNOHANG); } }
    // CRITICAL: without this, destroying the global `client` at exit with a still-joinable reader thread
    // makes std::thread's destructor call std::terminate() -> abort (the quit crash).
    ~Client() { alive = false; if (inFd >= 0) ::close(inFd); inFd = -1; if (reader.joinable()) reader.detach(); }

    void didOpen(const string& path, const string& text) { if (!alive.load()) return; string u = uriOf(path);
        notify("textDocument/didOpen", "{\"textDocument\":{\"uri\":" + jstr(u) + ",\"languageId\":\"cpp\",\"version\":1,\"text\":" + jstr(text) + "}}"); }
    void didChange(const string& path, const string& text) { if (!alive.load()) return; string u = uriOf(path); int v = ++version + 1;
        notify("textDocument/didChange", "{\"textDocument\":{\"uri\":" + jstr(u) + ",\"version\":" + std::to_string(v) + "},"
               "\"contentChanges\":[{\"text\":" + jstr(text) + "}]}"); }
    vector<Diag> diagnostics(const string& path) { std::lock_guard<std::mutex> lk(mx); auto it = diags.find(uriOf(path)); return it == diags.end() ? vector<Diag>{} : it->second; }
    void clearDiagnostics(const string& path) { std::lock_guard<std::mutex> lk(mx); diags.erase(uriOf(path)); }   // after applying a fix -> hide the stale tag until clangd re-publishes (no double-apply)
    // request autocompletion at (line,col). buffer is synced first so clangd sees the current edits.
    void requestCompletion(const string& path, const string& text, int line, int col) { if (!alive.load()) return; didChange(path, text);
        int id = ++reqSeq; { std::lock_guard<std::mutex> lk(cmx); complAwaitId = id; }
        request(id, "textDocument/completion", "{\"textDocument\":{\"uri\":" + jstr(uriOf(path)) + "},\"position\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "}}"); }
    vector<Compl> completions() { std::lock_guard<std::mutex> lk(cmx); return complResult; }
    void requestDefinition(const string& path, int line, int col) { if (!alive.load()) return; int id = ++reqSeq; { std::lock_guard<std::mutex> lk(cmx); defAwaitId = id; }
        request(id, "textDocument/definition", "{\"textDocument\":{\"uri\":" + jstr(uriOf(path)) + "},\"position\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "}}"); }
    Loc definition() { std::lock_guard<std::mutex> lk(cmx); Loc l = defResult; defResult = Loc{}; return l; }
    // semantic rename: clangd renames the symbol at (line,col) -> a WorkspaceEdit (per-file text edits).
    void requestRename(const string& path, const string& text, int line, int col, const string& newName) { if (!alive.load()) return; didChange(path, text);
        int id = ++reqSeq; { std::lock_guard<std::mutex> lk(cmx); renameAwaitId = id; }
        request(id, "textDocument/rename", "{\"textDocument\":{\"uri\":" + jstr(uriOf(path)) + "},\"position\":{\"line\":" + std::to_string(line) + ",\"character\":" + std::to_string(col) + "},\"newName\":" + jstr(newName) + "}"); }
    std::map<string, vector<TextEdit>> renameEdits() { std::lock_guard<std::mutex> lk(cmx); auto r = renameResult; renameResult.clear(); return r; }

private:
    // read Content-Length framed messages; dispatch publishDiagnostics notifications.
    void readLoop() { string buf; char tmp[8192];
        while (alive.load()) { ssize_t n = ::read(outFd, tmp, sizeof tmp); if (n <= 0) break; buf.append(tmp, n);
            for (;;) { size_t he = buf.find("\r\n\r\n"); if (he == string::npos) break;
                size_t cl = buf.find("Content-Length:"); if (cl == string::npos || cl > he) { buf.erase(0, he + 4); continue; }
                long len = strtol(buf.c_str() + cl + 15, nullptr, 10); size_t bodyStart = he + 4;
                if (buf.size() < bodyStart + (size_t)len) break;                  // wait for the full body
                string body = buf.substr(bodyStart, len); buf.erase(0, bodyStart + len);
                onMessage(body); } }
        alive = false;
    }
    static long jnum(const string& s, size_t from) { while (from < s.size() && (s[from] == ' ' || s[from] == ':')) from++; return strtol(s.c_str() + from, nullptr, 10); }
    void onMessage(const string& m) {
        if (m.find("\"method\"") == string::npos) { handleResponse(m); return; }                  // a reply to one of our requests (id, no method)
        if (m.find("\"method\":\"textDocument/publishDiagnostics\"") == string::npos) return;
        size_t up = m.find("\"uri\":\""); if (up == string::npos) return; up += 7; size_t ue = m.find('"', up); string uri = m.substr(up, ue - up);
        vector<Diag> ds; size_t da = m.find("\"diagnostics\":["); if (da != string::npos) { size_t i = da + 15; int depth = 0;
            for (; i < m.size(); i++) { char c = m[i]; if (c == '{') { if (depth == 0) { // start of one diagnostic object
                        size_t j = i, d2 = 0; for (; j < m.size(); j++) { if (m[j] == '{') d2++; else if (m[j] == '}') { if (--d2 == 0) { j++; break; } } }
                        ds.push_back(parseDiag(m.substr(i, j - i))); i = j - 1; } }
                else if (c == ']' && depth == 0) break; } }
        { std::lock_guard<std::mutex> lk(mx); diags[uri] = ds; } epoch++;
    }
    Diag parseDiag(const string& o) { Diag d; size_t r = o.find("\"range\":");
        if (r != string::npos) { size_t ls = o.find("\"line\":", r); if (ls != string::npos) d.line = (int)jnum(o, ls + 7);
            size_t cs = o.find("\"character\":", r); if (cs != string::npos) d.col = (int)jnum(o, cs + 12); }
        size_t sv = o.find("\"severity\":"); if (sv != string::npos) d.severity = (int)jnum(o, sv + 11);
        size_t ms = o.find("\"message\":\""); if (ms != string::npos) { ms += 11; string s; for (size_t k = ms; k < o.size(); k++) { char c = o[k];
            if (c == '\\' && k + 1 < o.size()) { char n = o[++k]; s += (n == 'n' ? '\n' : n == 't' ? ' ' : n); } else if (c == '"') break; else s += c; } d.msg = s; }
        // clangd inline quick-fix (codeActionsInline) — bound to ONLY the FIRST code action's edits (scanning the
        // whole diagnostic would mix edits from multiple actions and corrupt the file).
        size_t ca = o.find("\"codeActions\":["); if (ca != string::npos) { size_t a0 = o.find('{', ca);
            if (a0 != string::npos) { size_t j = a0, d2 = 0; for (; j < o.size(); j++) { if (o[j] == '{') d2++; else if (o[j] == '}') { if (--d2 == 0) { j++; break; } } }
                string act = o.substr(a0, j - a0); d.fixTitle = jfield(act, "\"title\":");
                size_t ch = act.find("\"changes\":"); size_t arr = act.find('[', ch != string::npos ? ch : act.find("\"edit\":"));   // the edits array under edit.changes[uri]
                if (arr != string::npos) for (size_t i = arr + 1; i < act.size(); i++) { char c = act[i];
                    if (c == '{') { size_t k = i, dd = 0; for (; k < act.size(); k++) { if (act[k] == '{') dd++; else if (act[k] == '}') { if (--dd == 0) { k++; break; } } }
                        TextEdit te = parseTextEdit(act.substr(i, k - i));
                        bool dup = false; for (auto& x : d.fix) if (x.line == te.line && x.sc == te.sc && x.newText == te.newText) dup = true;
                        if (!dup) d.fix.push_back(te); i = k - 1; if (d.fix.size() > 8) break; }
                    else if (c == ']') break; } } }
        size_t nlp = d.msg.find('\n'); if (nlp != string::npos) d.msg = d.msg.substr(0, nlp);   // drop clangd's appended note lines -> clean one-line message
        return d; }
    // parse one LSP TextEdit object `{newText, range:{start,end}}` — KEY-ORDER-INDEPENDENT (clangd serializes
    // newText before range, end before start, character before line — anchoring on "start"/"end" names is the only safe way).
    static TextEdit parseTextEdit(const string& o) { TextEdit te;
        size_t st = o.find("\"start\":"); if (st != string::npos) { size_t lp = o.find("\"line\":", st); if (lp != string::npos) te.line = (int)jnum(o, lp + 7);
            size_t cp = o.find("\"character\":", st); if (cp != string::npos) te.sc = (int)jnum(o, cp + 12); }
        size_t en = o.find("\"end\":"); if (en != string::npos) { size_t lp = o.find("\"line\":", en); if (lp != string::npos) te.eline = (int)jnum(o, lp + 7);
            size_t cp = o.find("\"character\":", en); if (cp != string::npos) te.ec = (int)jnum(o, cp + 12); }
        te.newText = jfield(o, "\"newText\":"); return te; }
    static string jfield(const string& o, const char* key) { size_t p = o.find(key); if (p == string::npos) return ""; p = o.find('"', p + strlen(key)); if (p == string::npos) return ""; p++;
        string s; for (size_t k = p; k < o.size(); k++) { char c = o[k]; if (c == '\\' && k + 1 < o.size()) { char n = o[++k]; s += (n == 'n' ? '\n' : n == 't' ? ' ' : n == 'u' ? '?' : n); } else if (c == '"') break; else s += c; } return s; }
    void handleResponse(const string& m) {
        size_t idp = m.find("\"id\":"); if (idp == string::npos) return; int id = (int)jnum(m, idp + 5);
        int cAwait, dAwait; { std::lock_guard<std::mutex> lk(cmx); cAwait = complAwaitId; dAwait = defAwaitId; }
        int rAwait; { std::lock_guard<std::mutex> lk(cmx); rAwait = renameAwaitId; }
        if (id == cAwait) { vector<Compl> items = parseCompletions(m); { std::lock_guard<std::mutex> lk(cmx); complResult = items; complAwaitId = -1; } complEpoch++; return; }
        if (id == dAwait) { Loc l = parseDefinition(m); { std::lock_guard<std::mutex> lk(cmx); defResult = l; defAwaitId = -1; } defEpoch++; return; }
        if (id == rAwait) { auto e = parseWorkspaceEdit(m); { std::lock_guard<std::mutex> lk(cmx); renameResult = e; renameAwaitId = -1; } renameEpoch++; return; }
    }
    static string uriToPath(const string& uri) { if (uri.rfind("file://", 0) != 0) return ""; string u = uri.substr(7), p;
        for (size_t i = 0; i < u.size(); i++) { if (u[i] == '%' && i + 2 < u.size()) { p += (char)strtol(u.substr(i + 1, 2).c_str(), nullptr, 16); i += 2; } else p += u[i]; } return p; }
    std::map<string, vector<TextEdit>> parseWorkspaceEdit(const string& m) {   // both "changes":{uri:[…]} and documentChanges:[{textDocument:{uri},edits:[…]}]
        std::map<string, vector<TextEdit>> out; size_t p = 0;
        while ((p = m.find("file://", p)) != string::npos) { size_t ue = m.find('"', p); if (ue == string::npos) break; string path = uriToPath(m.substr(p, ue - p)); p = ue + 1;
            size_t br = m.find('[', ue); if (br == string::npos) continue; vector<TextEdit> edits;
            for (size_t i = br + 1; i < m.size(); i++) { char c = m[i];
                if (c == '{') { size_t j = i, d = 0; for (; j < m.size(); j++) { if (m[j] == '{') d++; else if (m[j] == '}') { if (--d == 0) { j++; break; } } }
                    edits.push_back(parseTextEdit(m.substr(i, j - i))); i = j - 1; }
                else if (c == ']') { p = i; break; } }
            if (!edits.empty() && !path.empty()) out[path] = edits; }
        return out; }
    vector<Compl> parseCompletions(const string& m) { vector<Compl> out; size_t a = m.find("\"items\":["); if (a == string::npos) a = m.find("\"result\":[");
        if (a == string::npos) return out; size_t i = m.find('[', a) + 1;
        for (; i < m.size(); i++) { char c = m[i]; if (c == '{') { size_t j = i, d = 0; for (; j < m.size(); j++) { if (m[j] == '{') d++; else if (m[j] == '}') { if (--d == 0) { j++; break; } } }
                out.push_back(parseCompl(m.substr(i, j - i))); i = j - 1; if (out.size() > 200) break; } else if (c == ']') break; }
        return out; }
    Compl parseCompl(const string& o) { Compl c; c.label = jfield(o, "\"label\":"); c.insert = jfield(o, "\"insertText\":"); c.detail = jfield(o, "\"detail\":");
        size_t kp = o.find("\"kind\":"); if (kp != string::npos) c.kind = (int)jnum(o, kp + 7);
        if (c.insert.empty()) { c.insert = c.label; size_t pp = c.insert.find('('); if (pp != string::npos) c.insert = c.insert.substr(0, pp); }
        size_t s0 = 0; while (s0 < c.insert.size() && !(isalnum((unsigned char)c.insert[s0]) || c.insert[s0] == '_' || c.insert[s0] == '~')) s0++; c.insert = c.insert.substr(s0);   // strip clangd's leading bullet/space
        size_t b0 = 0; while (b0 < c.label.size() && ((unsigned char)c.label[b0] < 0x21 || (unsigned char)c.label[b0] >= 0x80)) b0++; c.label = c.label.substr(b0); return c; }   // strip the kind bullet/space
    Loc parseDefinition(const string& m) { Loc l; size_t r = m.find("\"uri\":\""); if (r == string::npos) return l; r += 7; size_t e = m.find('"', r); string uri = m.substr(r, e - r);
        if (uri.rfind("file://", 0) == 0) { uri = uri.substr(7); string p; for (size_t i = 0; i < uri.size(); i++) { if (uri[i] == '%' && i + 2 < uri.size()) { p += (char)strtol(uri.substr(i + 1, 2).c_str(), nullptr, 16); i += 2; } else p += uri[i]; } l.path = p; }
        size_t st = m.find("\"start\":", e); if (st != string::npos) { size_t ls = m.find("\"line\":", st); if (ls != string::npos) l.line = (int)jnum(m, ls + 7);
            size_t cs = m.find("\"character\":", st); if (cs != string::npos) l.col = (int)jnum(m, cs + 12); }
        l.ok = !l.path.empty(); return l; }
};
#else  // ── Windows: no-op stub (LSP needs a POSIX subprocess) ──
struct Client {
    std::atomic<int> epoch{0}, complEpoch{0}, defEpoch{0};
    bool start(const string&, const string&) { return false; }
    void stop() {} void didOpen(const string&, const string&) {} void didChange(const string&, const string&) {}
    std::vector<Diag> diagnostics(const string&) { return {}; }
    std::atomic<int> renameEpoch{0};
    void clearDiagnostics(const string&) {}
    void requestCompletion(const string&, const string&, int, int) {} std::vector<Compl> completions() { return {}; }
    void requestDefinition(const string&, int, int) {} Loc definition() { return {}; }
    void requestRename(const string&, const string&, int, int, const string&) {} std::map<string, std::vector<TextEdit>> renameEdits() { return {}; }
    bool isAlive() { return false; }
};
#endif
inline Client client;   // the one shared LSP client
} // namespace lsp
#endif
