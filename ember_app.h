// ember_app.h — EmberDragon workspace: a Binary-Ninja-style layout (toolbar,
// dockable tab strips, status bar) wired to the model. Immediate-mode: the
// platform fills ui::in for each event and calls app::render(). Panels are plain
// functions that take a rect; navigation/selection/context-menus live in App.
#ifndef EMBER_APP_H
#define EMBER_APP_H
#include "ember_ui.h"
#include "ember_image.h"
#include "ember_model.h"
#include "ember_store.h"
#include "ember_term.h"
#include "ember_lsp.h"
#include <functional>
#include <fstream>
#include <algorithm>
#include <cxxabi.h>    // demangle C++ symbols when matching a recompiled function back to its name
#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <set>
#include <utility>

namespace app {
using ui::Col; using std::string; using std::vector;

// platform-provided hooks
inline std::function<string()>            pickFile;     // open-file dialog -> path ("" = cancel)
inline std::function<string()>            pickSavePath; // save-as dialog -> path ("" = cancel)
inline std::function<void(const string&)> setClipboard; // copy text
inline std::function<string()>            getClipboard; // read clipboard text

enum SideTab { SIDE_FUNCS, SIDE_SYMS, SIDE_STR, SIDE_FILES };
enum MainTab { MAIN_PSEUDO, MAIN_GRAPH, MAIN_DISASM, MAIN_HEX, MAIN_FILE, MAIN_DIFF };
enum Pending { P_NONE, P_OPEN, P_EXPORT, P_OPENPATH, P_SAVEAS };   // deferred actions (run outside the GL draw)
enum Focus { F_NONE, F_FILTER, F_DOC, F_TERM, F_AI, F_DEBUG };  // where typed characters go (F_DOC -> activeDoc)
enum BottomTab { BOTTOM_LOG, BOTTOM_TERM, BOTTOM_DEBUG };

// context-menu action ids
enum { ACT_COPY = 1, ACT_COPY_NAME, ACT_GOTO, ACT_EXPORT, ACT_XREF, ACT_COPY_ADDR, ACT_SELALL, ACT_COPY_FUNC, ACT_RENAME, ACT_TOBYTES, ACT_NOP, ACT_ASMPATCH, ACT_PATCHFN, ACT_TODISASM, ACT_DEF, ACT_FIX };

// A byte-writing patch Claude proposed via @nop / @patch. It is QUEUED and shown in a
// confirmation dialog — the AI never writes a byte until you click Apply. (Rename / goto /
// comment are non-destructive and apply immediately; anything that touches the binary is gated.)
struct AiPatch { size_t off = SIZE_MAX; int slot = 0; vector<unsigned char> bytes; string desc; bool nop = false; };

struct App {
    string dir;                       // emberdragon launcher dir
    model::Binary bin;
    bool home = true;
    vector<string> recent;
    string lastProject, lastSource;   // for "reopen last session"

    SideTab sideTab = SIDE_FUNCS;
    MainTab mainTab = MAIN_PSEUDO;
    bool tabOpen[6] = { true, true, true, true, true, true };   // which center tabs are open (indexed by MainTab); closable via the ×
    float graphPanX = 0, graphPanY = 0;             // CFG graph view pan
    int hexSel = -1, hexSelEnd = -1, hexNibble = 0; std::set<size_t> patched; bool patchDirty = false;   // hex-editor byte patching (hexSelEnd>=0 = a drag/shift range for copy)
    bool hexAscii = false;              // hex view: editing the ASCII column (type a char -> byte) vs the hex nibbles
    std::map<size_t, unsigned char> patchOrig; int diffScroll = 0;   // original byte at each patched offset, for the side-by-side diff
    // byte-patch undo/redo: each entry is one logical edit = the (offset, previous-byte) pairs it changed
    std::vector<std::vector<std::pair<size_t, unsigned char>>> patchUndo, patchRedo;
    std::vector<std::pair<size_t, unsigned char>> patchPending;   // the edit being accumulated (committed on byte/selection change)
    size_t asmOff = SIZE_MAX; int asmLen = 0; string asmText;   // asm<->bytes: the right-clicked instruction's file offset / length / text
    uint64_t disasmGotoAddr = 0;   // a pseudocode-line "Show in Disassembly" request -> scroll the Disasm view to this vaddr next frame
    bool savePrompt = false;            // "overwrite or save-as-new" patch-save overlay
    bool asmPatchPrompt = false; string asmPatchText, asmPatchErr;   // "assemble a replacement instruction" overlay
    int selFn = 0;
    bool pseudoWhole = true;            // pseudocode shows the WHOLE program by default; click a fn in the sidebar to jump
    vector<string> wholeLines; vector<int> wholeFnLine; int wholeVer = 0, wholeVerBuilt = -1; bool pseudoJump = false;
    int codeScroll = 0, sideScroll = 0, hexScroll = 0, disScroll = 0, logScroll = 0, xrefScroll = 0, aiScroll = 0, fileScroll = 0;
    bool logStick = true;    // auto-follow the log to the bottom ONLY while the user is already at the bottom
    string filter;
    int selA = -1, selB = -1;          // code line-range selection
    int ctxTarget = -1;                // function index a context menu was opened on
    string ctxText;                    // text payload for copy actions

    store::Node fileTree;              // project file explorer
    string fileViewName, fileViewPath; // currently-opened editor file
    vector<string> editLines;          // editable buffer
    int edLine = 0, edCol = 0, fileViewScroll = 0; bool fileDirty = false;
    Focus focus = F_NONE;

    bool paletteOpen = false; string paletteQuery; int paletteSel = 0;   // command palette (Cmd/Ctrl+Shift+P)
    bool settingsOpen = false;          // settings overlay
    bool ctrlSave = false;             // platform sets this on Cmd/Ctrl+S
    BottomTab bottomTab = BOTTOM_LOG; int termScroll = 0, dbgScroll = 0;   // bottom dock: Log / Terminal / Debugger
    string dbgBreak; bool dbgBpFocus = false;   // the debugger's "break at" input field + its focus

    Pending pending = P_NONE; string pendingArg;   // deferred open/export

    float sideW = 260, rightW = 300, bottomH = 140;   // logical px
    bool showRight = true, showBottom = true;
    int dragSplit = 0;                 // 1=left 2=right 3=bottom

    vector<string> aiOut;              // AI panel output / chat transcript
    string aiInput;                    // the chat box (ask Claude anything)
    vector<std::pair<string,string>> renameLog;   // every global (function/type/symbol) rename, for program-wide re-propagation
    std::map<uint64_t, vector<std::pair<string,string>>> fnLocalRenames;   // per-function (by addr) local var renames, ordered — replayed after a re-lift so your names survive
    vector<AiPatch> aiPatchQ; bool aiPatchPrompt = false; int aiPatchScroll = 0;   // AI-proposed byte patches (@nop/@patch) awaiting your confirmation (scrollable so ALL are reviewable)
    int pendingMainTab = -1;   // a center-tab switch deferred to the TOP of next render, so its panel body actually draws that frame (no stale frame)
    bool aiOptIn = false;              // AI is OFF by default — user must opt in before any Claude call
    bool structWidths = false;         // OFF by default — opt-in precise struct-array field widths (best-effort, see warning)

    void log(const string& m) { bin.log.push_back(m); if (logStick) logScroll = 1 << 30; }   // only snap to bottom if the user is following along
};
inline App g;
// overlay "just opened this frame" guards: the very click that opens the palette/
// settings must not also be treated as an outside-click that closes them.
inline bool paletteJustOpened = false, settingsJustOpened = false;
inline bool aiPatchJustOpened = false;   // the AI-patch modal can be opened by pumpAI() in the same frame a live Enter exists — skip that Enter so it can't self-Apply
inline bool asmPatchJustOpened = false;  // same guard for the assemble-instruction editor opened via Enter on a disasm line

// ── projects (real on-disk output under Application Support) ──────────────────
inline void refreshTree() { if (g.bin.loaded) g.fileTree = store::scanTree(store::projectDir(g.bin.name), g.bin.name); }
// whole-program pseudocode buffer: concatenate every function, remembering where each starts (for sidebar jump)
inline void rebuildWhole() {
    g.wholeLines.clear(); g.wholeFnLine.assign(g.bin.funcs.size(), 0);
    for (size_t i = 0; i < g.bin.funcs.size(); i++) { g.wholeFnLine[i] = (int)g.wholeLines.size();
        for (auto& l : g.bin.funcs[i].lines) g.wholeLines.push_back(l); g.wholeLines.push_back(""); }
    g.wholeVerBuilt = g.wholeVer;
}

inline void openFileInViewer(const string& path);   // fwd
inline void lspChange(const string& path, const string& text);   // fwd (clangd buffer resync)

// ── async analysis (real decompile work on a worker thread, live progress) ────
// The engine streams per-function progress to stderr; we tail it into the Log so
// the user sees real work happen instead of a frozen window + an "instant" result.
inline std::atomic<bool> analyzing{false};
inline std::mutex anaMx; inline vector<string> anaLines; inline std::atomic<bool> anaJustFinished{false};
inline string anaPath, anaDecomp, anaName, anaProj;
inline std::atomic<int> anaCount{0}; inline string anaStatus;   // in-app decompiling-popup progress (functions seen + last status line)
inline std::mutex flirtMx; inline vector<std::pair<uint64_t,string>> flirtMatches;   // FLIRT: addr -> identified library-function name (applied after load)
inline void applyRenames(const std::map<string,string>& m);   // fwd — pumpAnalysis applies FLIRT identifications
inline int autoNameOffline();                                 // fwd — pumpAnalysis auto-names by behavior on every decompile
inline void reapplyFnNames(int fi);                           // fwd — replay your renames so they survive a re-decompile (re-lift)
inline void openBinary(const string& path) {
    if (path.empty() || analyzing.load()) return;
    string name = store::base(path), proj = store::projectDir(name), decomp = proj + "/decomp";
    store::mkdirs(decomp);
    g.home = false; g.bin.loaded = false; g.selFn = 0; g.pseudoJump = true; g.showBottom = true;   // show workspace + log immediately
    g.patched.clear(); g.patchOrig.clear(); g.diffScroll = 0; g.hexSel = -1; g.hexSelEnd = -1; g.hexNibble = 0; g.patchDirty = false;   // a new binary starts with no byte patches
    g.hexAscii = false; g.patchUndo.clear(); g.patchRedo.clear(); g.patchPending.clear();
    anaName = name; anaProj = proj; anaDecomp = decomp; anaPath = path; analyzing = true;
    anaStatus = "starting…"; anaCount = 0;                       // reset the in-app progress popup
    string dir = g.dir; bool sw = g.structWidths;
    std::thread([path, name, decomp, dir, sw]() {               // runs OFF the UI thread (no freeze)
        { std::lock_guard<std::mutex> lk(anaMx); anaLines.push_back("analyzing " + store::base(path) + " ..."); }
        string cmd = model::shq(dir + "emberdragon") + " " + model::shq(path) + " --collapse" + (sw ? " --struct-widths" : "") + " --export --out " + model::shq(decomp) + " 2>&1";   // clean, source-like output by default; opt-in precise struct field widths
        model::runCmdStream(cmd, [](const string& ln) { string t = model::trim(ln); if (t.empty()) return;
            { std::lock_guard<std::mutex> lk(anaMx); anaLines.push_back("  " + t); }
            anaStatus = t.size() > 52 ? t.substr(0, 52) : t; if (t.rfind("decompiling", 0) == 0) anaCount++; });   // feed the popup
        // FLIRT: grow the persistent signature DB with THIS binary's named functions, then identify any
        // functions already known from other binaries you've decompiled (turns sub_<addr> into real names).
        { string sigs = dir + "ember-sigs", db = store::root() + "/sigs.db";
          if (store::exists(sigs)) {
            model::runCmd(model::shq(sigs) + " gen " + model::shq(db) + " " + model::shq(path) + " 2>/dev/null");   // grow the DB with this binary's named functions
            // function-start addresses from the exported disasm (the addr right after each `name:` label) —
            // works even on STRIPPED binaries, where the symbol table is gone but the lifter still found the functions.
            string asmtxt = model::readText(decomp + "/asm/" + name + ".s"), addrs; bool want = false;
            { size_t i = 0; while (i < asmtxt.size()) { size_t e = asmtxt.find('\n', i); string ln = asmtxt.substr(i, e == string::npos ? string::npos : e - i); i = (e == string::npos) ? asmtxt.size() : e + 1;
                string t = model::trim(ln); if (t.empty()) continue; size_t c = 0; while (c < t.size() && isxdigit((unsigned char)t[c])) c++;
                if (c > 0 && c < t.size() && t[c] == ':') { if (want) { addrs += t.substr(0, c) + "\n"; want = false; } } else if (t.size() > 1 && t.back() == ':') want = true; } }
            string af = decomp + "/.flirt_addrs"; store::writeFile(af, addrs);
            string mo = model::runCmd(model::shq(sigs) + " match " + model::shq(db) + " " + model::shq(path) + " " + model::shq(af) + " 2>/dev/null"); remove(af.c_str());
            std::lock_guard<std::mutex> lk(flirtMx); flirtMatches.clear();
            size_t i = 0; while (i < mo.size()) { size_t e = mo.find('\n', i); string ln = mo.substr(i, e == string::npos ? string::npos : e - i); i = (e == string::npos) ? mo.size() : e + 1;
                size_t sp = ln.find(' '); if (sp == string::npos) continue; uint64_t a = strtoull(ln.substr(0, sp).c_str(), nullptr, 16); string nm = model::trim(ln.substr(sp + 1));
                if (a && !nm.empty()) flirtMatches.push_back({ a, nm }); } } }
        anaJustFinished = true;                                 // NOTE: leave `analyzing` true until pumpAnalysis finalizes (re-entrancy guard)
    }).detach();
}
inline void pumpAnalysis() {                                    // main-thread: drain progress + finalize the load
    { std::lock_guard<std::mutex> lk(anaMx); if (!anaLines.empty()) { for (auto& l : anaLines) g.log(l); anaLines.clear(); g.showBottom = true; } }
    if (anaJustFinished.exchange(false)) {
        g.bin.load(g.dir, anaPath, anaDecomp);                  // parse freshly-written files (fast; main thread owns g.bin)
        if (g.bin.loaded) {
            store::writeProject(anaName, anaPath, g.bin.arch, "pseudo");
            store::addRecent(anaPath); g.recent = store::loadRecent();
            store::saveLastSession(anaName, anaPath); g.lastProject = anaName; g.lastSource = anaPath;
            { std::lock_guard<std::mutex> lk(flirtMx); std::map<string,string> fr; std::set<string> taken; for (auto& f : g.bin.funcs) taken.insert(f.name);   // apply FLIRT: rename the sub_<addr> funcs the DB recognized
              for (auto& mch : flirtMatches) for (auto& f : g.bin.funcs) if (f.addr == mch.first && f.kind == 0 && f.name.rfind("sub_", 0) == 0) {
                  string fin = mch.second; int s = 2; while (taken.count(fin)) fin = mch.second + std::to_string(s++); taken.insert(fin); fr[f.name] = fin; break; }
              if (!fr.empty()) { applyRenames(fr); for (auto& kv : fr) g.renameLog.push_back({ kv.first, kv.second }); g.bin.buildXrefs();
                g.log("FLIRT: identified " + std::to_string((int)fr.size()) + " function(s) from the signature database"); } }
            for (int i = 0; i < (int)g.bin.funcs.size(); i++) reapplyFnNames(i);   // FIRST replay your renames (global + per-fn locals) so a re-decompile keeps them; no-op on a fresh load
            { int an = autoNameOffline(); if (an) g.log("auto-named " + std::to_string(an) + " function(s) from their behavior"); }   // THEN auto-name only the remaining sub_ placeholders
            refreshTree(); g.selFn = 0; g.pseudoJump = true; g.wholeVer++;   // fresh program -> rebuild whole-pseudo buffer
            g.log("decompiled " + std::to_string((int)g.bin.funcs.size()) + " function(s) -> " + anaProj);
        } else g.log("error: failed to load " + anaPath);
        analyzing = false;                                      // clear ONLY after finalize -> a second Open can't race the load target
    }
}
inline void exportBinary() { if (g.bin.loaded) openBinary(g.bin.path); }   // re-run analysis (async, streamed)

// ── "Clean Up" engine: collapse the raw decompiler output toward clean source ─
// Offline de-bloat (ember-collapse) always; the Claude naming pass ONLY when the
// user has explicitly opted in. Writes a verified sibling optimized/ folder.
inline std::atomic<bool> optimizing{false};   // true while a pass runs (button disabled)
inline std::mutex optMx; inline vector<string> optLines; inline std::atomic<bool> optJustFinished{false};
inline string optResultFile;
// Tidy a Clean Up result so re-running it (or the AI pass) can't leave a mess:
//  - dedup the top-level preamble: repeated #include / using namespace std; / identical forward declarations
//  - drop a bare local declaration that SHADOWS a parameter (e.g. a spurious `int arg1;` when arg1 is a param)
//  - collapse runs of blank lines. Deterministic + brace-depth-aware (never touches statements inside a body).
inline string tidySource(const string& in) {
    vector<string> L; { size_t i = 0; while (i < in.size()) { size_t e = in.find('\n', i); L.push_back(in.substr(i, e == string::npos ? string::npos : e - i)); if (e == string::npos) break; i = e + 1; } }
    auto tr = [](const string& s) { size_t a = s.find_first_not_of(" \t"); if (a == string::npos) return string(); size_t b = s.find_last_not_of(" \t"); return s.substr(a, b - a + 1); };
    auto idtail = [](const string& s) { int e = (int)s.size(); while (e > 0 && !(isalnum((unsigned char)s[e-1]) || s[e-1] == '_')) e--; int b = e; while (b > 0 && (isalnum((unsigned char)s[b-1]) || s[b-1] == '_')) b--; return s.substr(b, e - b); };
    auto isType = [](const string& w) { static const char* T[] = { "int","long","char","short","unsigned","signed","float","double","bool","void","auto","size_t","int8_t","int16_t","int32_t","int64_t","uint8_t","uint16_t","uint32_t","uint64_t","struct","const","static" }; for (auto t : T) if (w == t) return true; return false; };
    // pass 1: dedup the depth-0 preamble (includes / using / forward-decl prototypes)
    std::set<string> seenInc, seenProto; bool seenUsing = false; vector<string> a; int depth = 0;
    for (auto& l : L) { string t = tr(l); bool drop = false;
        if (depth == 0) {
            if (t.rfind("#include", 0) == 0) { if (seenInc.count(t)) drop = true; else seenInc.insert(t); }
            else if (t == "using namespace std;") { if (seenUsing) drop = true; else seenUsing = true; }
            else if (!t.empty() && t.back() == ';' && t.find('(') != string::npos && t.find('=') == string::npos && t.find('{') == string::npos && t.rfind("return", 0) != 0) { if (seenProto.count(t)) drop = true; else seenProto.insert(t); }   // forward declaration
        }
        if (!drop) a.push_back(l);
        for (char c : l) { if (c == '{') depth++; else if (c == '}' && depth > 0) depth--; }
    }
    // pass 2: within a function, drop a bare `<type> ident;` that shadows one of its parameters
    vector<string> b; depth = 0; std::set<string> params;
    for (auto& l : a) { string t = tr(l); bool drop = false;
        if (depth == 0) { size_t lp = l.find('('); if (lp != string::npos && l.find('{') != string::npos && l.find(')') != string::npos && (t.empty() || t.back() != ';')) { params.clear();
            string args = l.substr(lp + 1, l.rfind(')') - lp - 1); size_t p = 0;
            while (p <= args.size()) { size_t c = args.find(',', p); string one = args.substr(p, c == string::npos ? string::npos : c - p); string nm = idtail(one); if (!nm.empty()) params.insert(nm); if (c == string::npos) break; p = c + 1; } } }
        else if (!t.empty() && t.back() == ';' && t.find('(') == string::npos && t.find('=') == string::npos) {
            size_t sp = t.find_first_of(" \t"); if (sp != string::npos) { string first = t.substr(0, sp); for (char& ch : first) if (ch == '*' || ch == '&') ch = ' '; first = tr(first);
                if (isType(first)) { string nm = idtail(t.substr(0, t.size() - 1)); if (params.count(nm)) drop = true; } } }
        if (!drop) b.push_back(l);
        for (char c : l) { if (c == '{') depth++; else if (c == '}' && depth > 0) { depth--; if (depth == 0) params.clear(); } }
    }
    string r; int blanks = 0; for (auto& l : b) { if (tr(l).empty()) { if (++blanks > 1) continue; } else blanks = 0; r += l + "\n"; }
    return r;
}
inline void applyRenames(const std::map<string,string>& m);   // fwd — Analyze propagates your renames first
inline int autoNameOffline();                                 // fwd — Analyze's offline (no-AI) function auto-namer
inline int autoNameVarsOffline();                             // fwd — Analyze's offline (no-AI) VARIABLE auto-namer (no v102)
// Analyze's "learn from YOUR renames like Ghidra": teach the FLIRT DB every function's current name (your manual
// renames + the auto-names) WITHOUT overriding what's already known, then re-identify still-placeholder functions
// from the now-richer DB — so naming you did here (or in past binaries) is applied equivalently everywhere.
inline int learnFromRenames() {
    if (!g.bin.loaded) return 0;
    string sigs = g.dir + "ember-sigs", db = store::root() + "/sigs.db"; if (!store::exists(sigs)) return 0;
    char b[40]; string names; for (auto& f : g.bin.funcs) { snprintf(b, sizeof b, "%llx ", (unsigned long long)f.addr); names += string(b) + f.name + "\n"; }
    string nf = store::decompDir(g.bin.name) + "/.learn_names"; store::writeFile(nf, names);
    model::runCmd(model::shq(sigs) + " gen " + model::shq(db) + " " + model::shq(g.bin.path) + " --names " + model::shq(nf) + " 2>/dev/null");   // ADD-ONLY: never overrides existing signatures
    remove(nf.c_str());
    string af; for (auto& f : g.bin.funcs) { snprintf(b, sizeof b, "%llx\n", (unsigned long long)f.addr); af += b; }
    string afp = store::decompDir(g.bin.name) + "/.learn_addrs"; store::writeFile(afp, af);
    string mo = model::runCmd(model::shq(sigs) + " match " + model::shq(db) + " " + model::shq(g.bin.path) + " " + model::shq(afp) + " 2>/dev/null"); remove(afp.c_str());
    std::map<string,string> ren; std::set<string> taken; for (auto& f : g.bin.funcs) taken.insert(f.name);
    size_t i = 0; while (i < mo.size()) { size_t e = mo.find('\n', i); string ln = mo.substr(i, e == string::npos ? string::npos : e - i); i = (e == string::npos) ? mo.size() : e + 1;
        size_t sp = ln.find(' '); if (sp == string::npos) continue; uint64_t a = strtoull(ln.substr(0, sp).c_str(), 0, 16); string nm = model::trim(ln.substr(sp + 1));
        if (!a || nm.empty()) continue;
        for (auto& f : g.bin.funcs) if (f.addr == a && f.name.rfind("sub_", 0) == 0) { string fin = nm; int k = 2; while (taken.count(fin)) fin = nm + std::to_string(k++); taken.insert(fin); ren[f.name] = fin; break; } }   // only fill placeholders, never override
    if (!ren.empty()) { applyRenames(ren); for (auto& kv : ren) g.renameLog.push_back({ kv.first, kv.second }); g.bin.buildXrefs(); }
    return (int)ren.size();
}
inline void runOptimize() {
    if (optimizing.load() || !g.bin.loaded) return;
    optimizing = true;
    // ANALYZE = real OFFLINE heavy lifting (no AI): (1) name every placeholder function from its behavior,
    // (2) propagate every rename (yours + the auto-named) across the WHOLE program, then (3) the deep clean pass.
    int named = g.aiOptIn ? 0 : autoNameOffline();
    int vnamed = g.aiOptIn ? 0 : autoNameVarsOffline();        // name the local vars too (no more v102)
    int propd = 0;
    if (!g.renameLog.empty()) { std::map<string,string> m; for (auto& pr : g.renameLog) m[pr.first] = pr.second; applyRenames(m); g.bin.buildXrefs(); propd = (int)m.size(); }
    int learned = learnFromRenames();   // teach FLIRT your names (add-only) + identify more functions equivalently (Ghidra-style)
    if (named || vnamed || propd || learned) { rebuildWhole(); g.log("analyze: named " + std::to_string(named) + " function(s) + " + std::to_string(vnamed) + " variable(s) from behavior, propagated " + std::to_string(propd) + " rename(s), learned + identified " + std::to_string(learned) + " more from your naming"); }
    // EDIT-AWARE: Clean Up works on your CURRENT source (every rename + body edit you made is already
    // in g.bin.funcs), so it takes your changes into account and propagates names — rather than throwing
    // them away by re-decompiling the binary. With Claude on, this is the deep pass: deflatten, rename
    // everything consistently, recover types, comment. The "takes longer, but clean" button.
    string cur; if (g.pseudoWhole && !g.wholeLines.empty()) { for (auto& l : g.wholeLines) cur += l + "\n"; }   // honor whole-view edits
    else { for (auto& f : g.bin.funcs) { for (auto& l : f.lines) cur += l + "\n"; cur += "\n"; } }
    string dir = g.dir, name = g.bin.name, optDir = store::optimizedDir(name);
    bool useAI = g.aiOptIn, cpp = g.bin.cppOut;
    std::thread([cur, dir, name, optDir, useAI, cpp]() {       // runs OFF the UI thread
        store::mkdirs(optDir + (cpp ? "/cpp" : "/c"));
        { std::lock_guard<std::mutex> lk(optMx); optLines.push_back(string("clean up: deep pass over your current source") + (useAI ? "  + Claude (deflatten / rename-all / types / comments — this takes a bit)" : "  (offline collapse)")); }
        string in = optDir + "/.cleanup.in"; store::writeFile(in, cur);
        string cmd = (useAI ? model::shq(dir + "ember-claude") : model::shq(dir + "ember-collapse")) + " < " + model::shq(in) + " 2>&1";   // stdin redirect (portable: no `cat` on Windows)
        string out = tidySource(model::runCmd(cmd));   // dedup any doubled preamble + drop param-shadowing locals (covers AI + offline)
        string outfile = optDir + (cpp ? "/cpp/" + name + ".cpp" : "/c/" + name + ".c"); store::writeFile(outfile, out); remove(in.c_str());
        { std::lock_guard<std::mutex> lk(optMx); optLines.push_back("clean up: done -> " + outfile); }
        optResultFile = outfile; optimizing = false; optJustFinished = true;
    }).detach();
}
inline void pumpOptimize() {                                    // main-thread drain (called each frame / timer)
    { std::lock_guard<std::mutex> lk(optMx); if (!optLines.empty()) { for (auto& l : optLines) g.log(l); optLines.clear(); g.showBottom = true; } }
    if (optJustFinished.exchange(false)) { refreshTree(); if (store::exists(optResultFile)) openFileInViewer(optResultFile); }
}
// ── AI panel: chat ("talk to Claude") + "Rewrite function" (async claude -p) ──
inline std::atomic<bool> aiBusy{false}, aiDone{false}; inline std::mutex aiMx; inline vector<string> aiResult;
inline void applyRenames(const std::map<string,string>& m);   // fwd (defined below) — pumpAI executes AI @rename
inline void queueAiPatch(uint64_t va, const string& ins, bool nop);   // fwd — pumpAI proposes confirm-gated @nop/@patch
inline void runAutoName();                                            // fwd — pumpAI's @autoname tool
// parse a hex virtual address Claude emitted ("0x100003f2c" or "100003f2c"); UINT64_MAX if not valid hex
inline uint64_t parseHexAddr(const string& s) { string t = model::trim(s); if (t.rfind("0x", 0) == 0 || t.rfind("0X", 0) == 0) t = t.substr(2);
    while (t.size() > 1 && t[0] == '0') t.erase(0, 1);   // strip leading zeros so a legit value isn't mistaken for overflow
    if (t.empty() || t.size() > 16) return UINT64_MAX;   // >16 hex digits can't fit 64 bits -> reject rather than silently saturate
    for (char c : t) if (!isxdigit((unsigned char)c)) return UINT64_MAX; return strtoull(t.c_str(), nullptr, 16); }
inline void pumpAI() {   // append to the transcript + EXECUTE any @rename/@goto tool commands Claude emitted
    if (!aiDone.exchange(false)) return; std::lock_guard<std::mutex> lk(aiMx);
    auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_' || c == ':' || c == '~'; };
    for (auto& l : aiResult) { string t = model::trim(l);
        if (t.rfind("@rename ", 0) == 0) {                                           // TOOL: program-wide rename
            string rest = model::trim(t.substr(8)); size_t sp = rest.find(' ');
            if (sp == string::npos) { g.aiOut.push_back("  (@rename needs an old name then a new name)"); continue; }
            string oldn = model::trim(rest.substr(0, sp)), newn = model::trim(rest.substr(sp + 1));
            size_t e = 0; while (e < newn.size() && (isalnum((unsigned char)newn[e]) || newn[e] == '_')) e++; newn = newn.substr(0, e);   // valid identifier only
            if (!oldn.empty() && !newn.empty() && oldn != newn) { std::map<string,string> m{ { oldn, newn } }; applyRenames(m); g.renameLog.push_back({ oldn, newn }); g.bin.buildXrefs();
                g.aiOut.push_back("  \xe2\x9c\x93 renamed " + oldn + " -> " + newn); }
            else g.aiOut.push_back("  (@rename: give a valid new identifier different from the old name)");
            continue; }
        if (t.rfind("@goto ", 0) == 0) {                                             // TOOL: navigate to a function
            string fn = model::trim(t.substr(6)); size_t e = 0; while (e < fn.size() && idc(fn[e])) e++; fn = fn.substr(0, e);
            bool found = false; for (int i = 0; i < (int)g.bin.funcs.size(); i++) if (g.bin.funcs[i].name == fn) { g.selFn = i; g.pseudoJump = true; g.aiOut.push_back("  \xe2\x9c\x93 went to " + fn); found = true; break; }
            if (!found) g.aiOut.push_back("  (no function named " + fn + ")");
            continue; }
        if (t.rfind("@nop ", 0) == 0) {                                              // TOOL: NOP an instruction (CONFIRM-GATED — writes bytes)
            uint64_t va = parseHexAddr(t.substr(5));
            if (va == UINT64_MAX) g.aiOut.push_back("  (couldn't read the @nop address)"); else queueAiPatch(va, "", true);
            continue; }
        if (t.rfind("@patch ", 0) == 0) {                                            // TOOL: assemble + patch an instruction (CONFIRM-GATED)
            string rest = model::trim(t.substr(7)); size_t sp = rest.find(' ');
            if (sp == string::npos) { g.aiOut.push_back("  (@patch needs an address then an instruction)"); continue; }
            uint64_t va = parseHexAddr(rest.substr(0, sp)); string ins = model::trim(rest.substr(sp + 1));
            if (va == UINT64_MAX) g.aiOut.push_back("  (couldn't read the @patch address)"); else queueAiPatch(va, ins, false);
            continue; }
        if (t.rfind("@comment ", 0) == 0) {                                          // TOOL: attach an explanatory comment above a function
            string rest = model::trim(t.substr(9)); size_t sp = rest.find(' ');
            if (sp == string::npos) { g.aiOut.push_back("  (@comment needs a function name then the comment text)"); continue; }
            string fn = model::trim(rest.substr(0, sp)), txt = model::trim(rest.substr(sp + 1));
            size_t e = 0; while (e < fn.size() && idc(fn[e])) e++; fn = fn.substr(0, e);
            if (txt.rfind("//", 0) == 0) txt = model::trim(txt.substr(2));
            if (txt.empty()) { g.aiOut.push_back("  (@comment needs some text after the function name)"); continue; }
            bool done = false; for (auto& f : g.bin.funcs) if (f.name == fn && f.kind == 0) { f.lines.insert(f.lines.begin(), "// " + txt); g.wholeVer++; done = true; break; }
            g.aiOut.push_back(done ? ("  \xe2\x9c\x93 commented " + fn) : ("  (no function named " + fn + ")"));
            continue; }
        if (t == "@autoname") { runAutoName(); g.aiOut.push_back("  \xe2\x9c\x93 auto-naming the program's functions"); continue; }   // TOOL: heuristic/AI rename-all
        g.aiOut.push_back(l);
    }
    g.aiScroll = 1 << 29;
}   // big -> the draw clamps it to the true bottom (after word-wrap)
inline void aiPush(const string& l) { g.aiOut.push_back(l); g.aiScroll = 1 << 29; }
// free-form question -> claude (with the current function as context). claude -p, no API key.
inline void runAsk(const string& q) {
    if (q.empty() || aiBusy.load()) return;
    if (!g.aiOptIn) { aiPush("> enable \"Use Claude\" above to chat."); return; }
    string ctx; if (g.bin.loaded && g.selFn < (int)g.bin.funcs.size()) { for (auto& l : g.bin.funcs[g.selFn].lines) ctx += l + "\n"; }
    if (g.bin.loaded && !model::trim(g.bin.disasm).empty()) {   // also hand Claude the disassembly (with addresses) so @nop/@patch can target real instructions
        ctx += "\n\n; ---- disassembly (addresses for @nop / @patch) ----\n";
        const string& d = g.bin.disasm; size_t i = 0; int ln = 0;
        while (i < d.size() && ln < 500) { size_t e = d.find('\n', i); ctx += d.substr(i, e == string::npos ? string::npos : e - i) + "\n"; if (e == string::npos) break; i = e + 1; ln++; }
        if (ln >= 500) ctx += "; (disassembly truncated — ask me to focus on a function for its addresses)\n";
    }
    { size_t i = 0; bool first = true; while (i <= q.size()) { size_t e = q.find('\n', i); string seg = q.substr(i, e == string::npos ? string::npos : e - i);   // multi-line question -> one transcript line each
        aiPush((first ? "> you: " : "       ") + seg); first = false; if (e == string::npos) break; i = e + 1; } }
    aiBusy = true; string dir = g.dir;
    std::thread([dir, q, ctx]() {                                   // off the UI thread (claude -p takes a few seconds)
        string cmd = "printf %s " + model::shq(ctx) + " | " + model::shq(dir + "ember-ask") + " " + model::shq(q) + " 2>&1";
        string out = model::runCmd(cmd);
        { std::lock_guard<std::mutex> lk(aiMx); aiResult.clear(); size_t i = 0;
          while (i < out.size()) { size_t e = out.find('\n', i); aiResult.push_back("  " + out.substr(i, e == string::npos ? string::npos : e - i)); if (e == string::npos) break; i = e + 1; }
          if (aiResult.empty()) aiResult = { "  (no response — is the `claude` CLI installed + logged in?)" }; }
        aiBusy = false; aiDone = true;
    }).detach();
}

// ── whole-program understanding -> global rename (Claude reads EVERYTHING) ─────
inline std::atomic<bool> understanding{false}, understandDone{false}; inline std::mutex undMx; inline string undJSON;
inline std::map<string,string> parseJsonMap(const string& js) {
    std::map<string,string> m; size_t i = 0;
    auto rd = [&](size_t& p, string& out) -> bool { while (p < js.size() && js[p] != '"') { if (js[p] == '}') return false; p++; } if (p >= js.size()) return false; p++; out.clear();
        while (p < js.size() && js[p] != '"') { if (js[p] == '\\' && p + 1 < js.size()) { out += js[p + 1]; p += 2; } else out += js[p++]; } if (p < js.size()) p++; return true; };
    while (i < js.size()) { string k, v; if (!rd(i, k)) break; while (i < js.size() && js[i] != ':' && js[i] != '}') i++; if (i >= js.size() || js[i] == '}') break; i++; if (!rd(i, v)) break; if (!k.empty() && !v.empty() && k != v) m[k] = v; }
    return m;
}
inline void applyRenames(const std::map<string,string>& m) {
    if (m.empty()) return; auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    // ONE simultaneous pass per line: match a whole token, look it up, emit the replacement and skip past it
    // (a previous N-sequential-replace version cascaded — newName==anotherOldKey collapsed two symbols).
    auto repl = [&](string& L) { string r; size_t i = 0;
        while (i < L.size()) { if ((i == 0 || !idc(L[i - 1])) && idc(L[i])) { size_t j = i; while (j < L.size() && idc(L[j])) j++;
            auto it = m.find(L.substr(i, j - i)); if (it != m.end()) { r += it->second; i = j; continue; } }
            r += L[i++]; } L = r; };
    for (auto& f : g.bin.funcs) { repl(f.name); repl(f.sig); for (auto& l : f.lines) repl(l); }
    g.wholeVer++;   // funcs changed -> the whole-program buffer must rebuild
}
// after a function is RE-LIFTED (back to fresh placeholder names), REPLAY your renames so they survive:
// (1) program-wide names (renameLog), then (2) this function's local renames in the order you made them.
// This is what lets "patch the asm -> pseudocode re-lifts -> your names stay" work.
inline void reapplyFnNames(int fi) {
    if (fi < 0 || fi >= (int)g.bin.funcs.size()) return;
    auto& f = g.bin.funcs[fi];
    auto idcL = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    auto replOne = [&](string& L, const string& o, const string& n) { if (o.empty() || o == n) return; string r; size_t i = 0;
        while (i < L.size()) { if (L.compare(i, o.size(), o) == 0 && (i == 0 || !idcL(L[i - 1])) && (i + o.size() >= L.size() || !idcL(L[i + o.size()]))) { r += n; i += o.size(); } else r += L[i++]; } L = r; };
    for (auto& pr : g.renameLog) { replOne(f.sig, pr.first, pr.second); for (auto& L : f.lines) replOne(L, pr.first, pr.second); }   // global names
    auto it = g.fnLocalRenames.find(f.addr);                                                                                       // this fn's locals, in order
    if (it != g.fnLocalRenames.end()) for (auto& pr : it->second) { replOne(f.sig, pr.first, pr.second); for (auto& L : f.lines) replOne(L, pr.first, pr.second); }
}
// is `s` a PROGRAM-WIDE name (function / global / type) — safe to propagate everywhere — vs a
// function-local (vN/tN/argN/lowercase) that must NOT leak into other functions?
inline bool isGlobalName(const string& s) {
    if (s.empty()) return false;
    if (s.rfind("sub_", 0) == 0 || s.rfind("g_", 0) == 0 || s.rfind("S_", 0) == 0) return true;   // placeholder fn / global / struct
    if (isupper((unsigned char)s[0])) return true;                                                 // a Type / Class name
    return false;   // vN / tN / argN / lowercase local -> scoped to one function (a lowercase token that merely
                    // happens to equal a function name is NOT propagated — that's the caller's job via curName)
}
inline void runUnderstand() {
    if (understanding.load() || !g.bin.loaded) return;
    if (!g.aiOptIn) { g.aiOut = { "Enable \"Use Claude\" in the AI panel first —", "whole-program analysis sends the code to Claude." }; g.showRight = true; return; }
    understanding = true; g.showBottom = true;
    string dir = g.dir, dd = store::decompDir(g.bin.name), cfile = g.bin.cppOut ? dd + "/cpp/" + g.bin.name + ".cpp" : dd + "/c/" + g.bin.name + ".c";
    { std::lock_guard<std::mutex> lk(optMx); optLines.push_back("understand: Claude is reading the whole program..."); }
    std::thread([dir, cfile]() {
        string cmd = model::shq(dir + "ember-rename") + " < " + model::shq(cfile) + " 2>/dev/null";   // stdin redirect (portable)
        string js = model::runCmd(cmd);
        { std::lock_guard<std::mutex> lk(undMx); undJSON = js; }
        understanding = false; understandDone = true;
    }).detach();
}
inline void pumpUnderstand() {
    if (!understandDone.exchange(false)) return;
    std::map<string,string> m; { std::lock_guard<std::mutex> lk(undMx); m = parseJsonMap(undJSON); }
    if (m.empty()) { g.log("understand: no renames returned (is Claude enabled + the CLI logged in?)"); return; }
    applyRenames(m); for (auto& kv : m) g.renameLog.push_back({ kv.first, kv.second });   // record so Propagate can re-apply program-wide later
    g.bin.buildXrefs(); g.log("understand: applied " + std::to_string((int)m.size()) + " AI renames across the program");
}

// ── offline whole-program function auto-namer (no AI) — infers an accurate name ─
// for each placeholder function from the library calls + idioms in its body.
inline string verbOf(const string& c) {
    static const std::map<string,string> V = {
        {"open","openFile"},{"openat","openFile"},{"fopen","openFile"},{"creat","createFile"},{"close","closeFile"},{"fclose","closeFile"},
        {"read","readBytes"},{"fread","readBytes"},{"pread","readBytes"},{"write","writeBytes"},{"fwrite","writeBytes"},{"pwrite","writeBytes"},
        {"malloc","allocate"},{"calloc","allocate"},{"realloc","reallocate"},{"free","release"},
        {"memcpy","copyMemory"},{"memmove","moveMemory"},{"memset","fillMemory"},{"bzero","zeroMemory"},
        {"strlen","stringLength"},{"strcmp","compareStrings"},{"strncmp","compareStrings"},{"strcasecmp","compareStrings"},
        {"strcpy","copyString"},{"strncpy","copyString"},{"strcat","concatStrings"},{"strdup","dupString"},{"strchr","findChar"},{"strstr","findSubstring"},
        {"printf","printText"},{"fprintf","printText"},{"snprintf","formatString"},{"sprintf","formatString"},{"puts","printLine"},{"fputs","printText"},{"perror","printError"},
        {"socket","createSocket"},{"connect","connectSocket"},{"bind","bindSocket"},{"listen","listenSocket"},{"accept","acceptConnection"},{"send","sendData"},{"recv","recvData"},
        {"opendir","openDir"},{"readdir","readDir"},{"closedir","closeDir"},{"stat","statPath"},{"lstat","statPath"},{"mkdir","makeDir"},{"unlink","removeFile"},{"rmdir","removeDir"},{"rename","renamePath"},
        {"getenv","getEnvVar"},{"setenv","setEnvVar"},{"exit","exitProcess"},{"abort","abortProcess"},{"fork","forkProcess"},{"waitpid","waitForChild"},
        {"pthread_create","startThread"},{"pthread_join","joinThread"},{"pthread_mutex_lock","lock"},{"pthread_mutex_unlock","unlock"},
        {"sqrt","computeSqrt"},{"pow","computePow"},{"atoi","parseInt"},{"atol","parseLong"},{"strtol","parseLong"},{"atof","parseDouble"},{"strtod","parseDouble"},
        {"fgets","readLine"},{"getline","readLine"},{"fgetc","readChar"},{"getchar","readChar"},{"fputc","writeChar"},{"putchar","writeChar"},{"fseek","seekFile"},{"ftell","tellFile"},{"rewind","rewindFile"},{"feof","atEndOfFile"},
        {"qsort","sortArray"},{"bsearch","binarySearch"},{"system","runCommand"},{"popen","runCommand"},{"getpid","getProcessId"},{"sleep","sleepSeconds"},{"usleep","sleepMicros"},{"nanosleep","sleepNanos"},
        {"time","getTime"},{"clock","getClock"},{"gettimeofday","getTime"},{"rand","randomValue"},{"srand","seedRandom"},{"random","randomValue"},
        {"tolower","toLower"},{"toupper","toUpper"},{"isdigit","isDigit"},{"isalpha","isAlpha"},{"isspace","isSpace"},
        {"sigaction","installSignalHandler"},{"signal","installSignalHandler"},{"mmap","mapMemory"},{"munmap","unmapMemory"},{"dlopen","loadLibrary"},{"dlsym","resolveSymbol"},
        {"htons","hostToNet"},{"ntohs","netToHost"},{"inet_addr","parseAddress"},{"setsockopt","configureSocket"},{"getaddrinfo","resolveHost"},
    };
    auto it = V.find(c); return it == V.end() ? "" : it->second;
}
inline string guessName(const model::Func& f) {
    std::map<string,int> calls; auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    for (auto& l : f.lines) for (size_t i = 0; i < l.size();) {
        if (isalpha((unsigned char)l[i]) || l[i] == '_') { size_t j = i; while (j < l.size() && idc(l[j])) j++;
            if (j < l.size() && l[j] == '(') { string w = l.substr(i, j - i);
                if (w != "if" && w != "while" && w != "for" && w != "switch" && w != "return" && w != "sizeof" && w != f.name) calls[w]++; }
            i = j; } else i++; }
    auto has = [&](const char* n) { return calls.count(n) > 0; };
    // recognized multi-call idioms (most specific first)
    if ((has("open") || has("openat") || has("fopen")) && (has("read") || has("fread"))) return "readFile";
    if ((has("open") || has("openat") || has("fopen")) && (has("write") || has("fwrite"))) return "writeFile";
    if (has("opendir") && has("readdir")) return "listDirectory";
    if (has("socket") && has("connect")) return "openConnection";
    if (has("socket") && (has("bind") || has("listen"))) return "startServer";
    if (has("malloc") && has("memcpy")) return "cloneBuffer";
    if (has("qsort")) return "sortArray";
    if (has("bsearch")) return "binarySearch";
    if ((has("fopen") || has("open")) && (has("fgets") || has("getline"))) return "readFileLines";
    if (has("malloc") && (has("free") || has("realloc"))) return "manageBuffer";
    if (has("system") || has("popen")) return "runCommand";
    if (has("connect") && (has("send") || has("write"))) return "sendRequest";
    if (has("accept") && (has("recv") || has("read"))) return "handleClient";
    if (has("pthread_create")) return "startThread";
    // string-content hints — a distinctive literal often names the function better than its calls do
    { bool cmp = has("strcmp") || has("strncmp") || has("strcasecmp") || has("memcmp");
      for (auto& l : f.lines) { size_t q = l.find('"'); while (q != string::npos) { size_t e = l.find('"', q + 1); if (e == string::npos) break;
        string s; for (size_t k = q + 1; k < e; k++) s += (char)tolower((unsigned char)l[k]);
        if (s.find("usage:") != string::npos || s.find("usage ") != string::npos) return "printUsage";
        if (s.find("version") != string::npos && s.size() < 24) return "printVersion";
        if (cmp && (s.find("password") != string::npos || s.find("incorrect") != string::npos || s.find("correct") != string::npos)) return "verifyPassword";
        q = l.find('"', e + 1); } } }
    if (has("getenv")) return "readEnv";
    if (has("fork") || has("execve") || has("execvp") || has("posix_spawn")) return "spawnProcess";
    if (has("mmap") || has("VirtualAlloc")) return "mapMemory";
    if (has("regcomp") || has("regexec")) return "matchRegex";
    if (has("inet_pton") || has("getaddrinfo") || has("gethostbyname")) return "resolveHost";
    if (has("localtime") || has("strftime") || has("gmtime")) return "formatTime";
    if (has("setsockopt") || has("getsockopt")) return "configureSocket";
    if (has("ioctl")) return "deviceControl";
    if (has("dlopen") || has("LoadLibrary") || has("dlsym") || has("GetProcAddress")) return "loadModule";
    if (has("CreateFileA") || has("CreateFileW") || has("ReadFile")) return "readFile";
    if (has("snprintf") || has("sprintf") || has("vsnprintf")) return "formatString";
    if (has("strcmp") || has("strncmp") || has("strcasecmp")) return "compareStrings";
    // else: the most frequent recognized library call gives the verb
    string best; int bestc = 0; for (auto& kv : calls) { string v = verbOf(kv.first); if (!v.empty() && kv.second > bestc) { best = v; bestc = kv.second; } }
    if (!best.empty()) return best;
    if (calls.size() == 1) return calls.begin()->first + "Wrapper";   // a thin pass-through wrapper
    return "";                                                          // not confident -> leave as sub_
}
// the offline (no-AI) function auto-namer: rename every placeholder sub_ function from its behavior.
// Applies the renames program-wide + logs them for propagation. Returns how many it named. Used by the
// Auto-Name button AND folded into Analyze (so Analyze does real offline heavy lifting without Claude).
inline int autoNameOffline() {
    std::map<string,string> renames; std::set<string> used; auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    for (auto& f : g.bin.funcs) { used.insert(f.name);                          // collision universe = EVERY identifier token in the program
        for (auto& l : f.lines) for (size_t i = 0; i < l.size();) { if ((isalpha((unsigned char)l[i]) || l[i] == '_')) { size_t j = i; while (j < l.size() && idc(l[j])) j++; used.insert(l.substr(i, j - i)); i = j; } else i++; } }
    for (auto& f : g.bin.funcs) { if (f.kind != 0) continue; if (f.name.rfind("sub_", 0) != 0) continue;   // only placeholder functions
        string nm = guessName(f); if (nm.empty() || nm == f.name) continue;
        string base = nm; int k = 2; while (used.count(nm)) nm = base + std::to_string(k++); used.insert(nm); renames[f.name] = nm; }
    if (renames.empty()) return 0;
    applyRenames(renames); for (auto& kv : renames) g.renameLog.push_back({ kv.first, kv.second });
    g.bin.buildXrefs(); return (int)renames.size();
}
// behavior-based name for ONE function-local placeholder var (vN/tN/argN), from its uses in f.
// Conservative: returns "" unless a clear pattern matches, so Analyze never invents a misleading name.
inline string guessVarName(const model::Func& f, const string& v) {
    auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    bool returned = false, arrow = false, indexed = false, deref = false, setZero = false, inc = false, accum = false;
    for (auto& L : f.lines) {
        for (size_t p = 0; (p = L.find(v, p)) != string::npos; ) {
            size_t e = p + v.size();
            if (!((p == 0 || !idc(L[p - 1])) && (e >= L.size() || !idc(L[e])))) { p = e; continue; }   // whole token only
            size_t a = p; while (a > 0 && L[a - 1] == ' ') a--;  char pre  = a > 0 ? L[a - 1] : 0;       // char before
            size_t b = e; while (b < L.size() && L[b] == ' ') b++;
            char post = b < L.size() ? L[b] : 0, post2 = (b + 1 < L.size()) ? L[b + 1] : 0;             // chars after
            if (pre == '*') deref = true;
            if (post == '[') indexed = true;
            if (post == '-' && post2 == '>') arrow = true;
            if (p >= 7 && L.compare(p - 7, 7, "return ") == 0) returned = true;   // "return v" (use p, not space-skipped a)
            if (post == '=' && post2 != '=') {                                                          // v = RHS (a definition)
                size_t r = b + 1; while (r < L.size() && L[r] == ' ') r++; string rhs = L.substr(r);
                if (!rhs.empty() && rhs[0] == '0' && (rhs.size() == 1 || !idc(rhs[1]))) setZero = true;
                else if (rhs.find(v) != string::npos) { if (rhs.find(v + " + 1") != string::npos) inc = true; else accum = true; }
            }
            p = e;
        }
        if (L.find(v + "++") != string::npos) inc = true;
    }
    if (arrow)            return "node";      // v->field  -> a struct/node pointer
    if (deref)            return "ptr";       // *v        -> a pointer
    if (indexed)          return "buf";       // v[i]      -> a buffer/array
    if (setZero && inc)   return "i";         // =0 then ++ -> loop counter / index
    if (setZero && accum) return "sum";       // =0 then v=v OP.. -> accumulator (better name than 'result')
    if (returned)         return "result";    // a returned scalar -> the result
    return "";                                // not confident -> leave the placeholder
}
// offline (no-AI) VARIABLE auto-namer: per function, rename placeholder locals (vN/tN/argN) from their
// behavior, SCOPED to that function (locals must NOT leak across functions). Returns how many it named.
inline int autoNameVarsOffline() {
    auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    auto isPlaceholder = [&](const string& t) -> bool {
        size_t i = 0; if (t.size() < 2) return false;
        if (t[0] == 'v' || t[0] == 't') i = 1; else if (t.rfind("arg", 0) == 0) i = 3; else return false;
        if (i >= t.size()) return false; for (size_t k = i; k < t.size(); k++) if (!isdigit((unsigned char)t[k])) return false; return true;
    };
    int total = 0;
    for (auto& f : g.bin.funcs) { if (f.kind != 0) continue;
        std::set<string> vars, used;
        for (auto& l : f.lines) for (size_t i = 0; i < l.size();) {
            if (isalpha((unsigned char)l[i]) || l[i] == '_') { size_t j = i; while (j < l.size() && idc(l[j])) j++; string t = l.substr(i, j - i);
                used.insert(t); if (isPlaceholder(t)) vars.insert(t); i = j; } else i++; }
        if (vars.empty()) continue;
        std::map<string,string> rn;
        for (auto& v : vars) { string nm = guessVarName(f, v); if (nm.empty() || nm == v) continue;
            string base = nm; int k = 2; while (used.count(nm)) nm = base + std::to_string(k++); used.insert(nm); rn[v] = nm; }
        if (rn.empty()) continue;
        auto repl = [&](string& L) { string r; size_t i = 0; while (i < L.size()) {                    // function-SCOPED token replace
            if ((i == 0 || !idc(L[i - 1])) && idc(L[i])) { size_t j = i; while (j < L.size() && idc(L[j])) j++;
                auto it = rn.find(L.substr(i, j - i)); if (it != rn.end()) { r += it->second; i = j; continue; } }
            r += L[i++]; } L = r; };
        repl(f.sig); for (auto& l : f.lines) repl(l);
        total += (int)rn.size();
    }
    if (total) g.wholeVer++;   // bodies changed -> whole-program buffer rebuilds
    return total;
}
inline void runAutoName() {
    if (!g.bin.loaded) return;
    if (g.aiOptIn) { g.log("auto-name: asking Claude for the most accurate names (whole program)..."); runUnderstand(); return; }   // AI = best names
    g.log("auto-name: parsing " + std::to_string((int)g.bin.funcs.size()) + " functions (offline heuristics)...");
    int n = autoNameOffline();
    g.log(n ? ("auto-name: renamed " + std::to_string(n) + " function(s) from behavior. (Enable Claude for sharper, intent-based names.)")
            : "auto-name: no confident offline names found — enable \"Use Claude\" for semantic names.");
}
// re-apply every recorded global rename across the WHOLE program (functions, bodies, call sites, sigs) — idempotent
inline void runPropagate() {
    if (!g.bin.loaded) return;
    if (g.renameLog.empty()) { g.log("propagate: nothing to propagate yet — run Auto-Name or rename a function/type first."); return; }
    std::map<string,string> m; for (auto& pr : g.renameLog) m[pr.first] = pr.second;   // dedup, last-wins
    applyRenames(m); g.bin.buildXrefs();
    g.log("propagate: re-applied " + std::to_string((int)m.size()) + " rename(s) across the entire program");
}

// ── Recompile: turn the (edited / cleaned) decompiled source back into a binary ─
inline std::atomic<bool> recompiling{false};
inline void saveFile();   // fwd
inline string extOf(const string& n);   // fwd (defined with the explorer)
// ── bundled toolchain resolution ─────────────────────────────────────────────
// EmberDragon ships a full compile toolchain (clang/clang++/clangd + macOS SDK + mingw) so it builds
// native AND Windows binaries with zero system deps. Found in the .app Resources or the shared install
// dir; falls back to system tools if absent. ember-toolchain populates it.
inline string toolchainDir() {
    static string cached; static bool done = false; if (done) return cached;
    done = true;
    for (const string& d : { g.dir + "../Resources/toolchain", string("/usr/local/share/emberdragon/toolchain") })
        if (store::exists(d + "/usr/bin/clang")) { cached = d; break; }
    return cached;
}
inline string nativeCxx() { string d = toolchainDir(); return d.empty() ? "clang++" : model::shq(d + "/usr/bin/clang++") + " -isysroot " + model::shq(d + "/MacOSX.sdk"); }
inline string nativeCc()  { string d = toolchainDir(); return d.empty() ? "clang"   : model::shq(d + "/usr/bin/clang")   + " -isysroot " + model::shq(d + "/MacOSX.sdk"); }
inline string winCxx()    { string d = toolchainDir(); return (!d.empty() && store::exists(d + "/mingw/bin/x86_64-w64-mingw32-g++")) ? model::shq(d + "/mingw/bin/x86_64-w64-mingw32-g++") : "x86_64-w64-mingw32-g++"; }
inline string clangdPath(){ string d = toolchainDir(); return (!d.empty() && store::exists(d + "/usr/bin/clangd")) ? d + "/usr/bin/clangd" : "clangd"; }

inline void runRecompile() {
    if (recompiling.load() || !g.bin.loaded) return;
    if (g.fileDirty) saveFile();                              // compile the latest edits
    string name = g.bin.name, proj = store::projectDir(name);
    string openp = g.fileViewPath, oe = openp.empty() ? "" : extOf(openp), src;
    if (!openp.empty() && (oe == "c" || oe == "cpp" || oe == "cc" || oe == "h" || oe == "hpp")) src = openp;   // the open editor file
    else if (store::exists(store::optimizedDir(name) + "/cpp/" + name + ".cpp")) src = store::optimizedDir(name) + "/cpp/" + name + ".cpp";   // the cleaned C++ output
    else if (store::exists(store::optimizedDir(name) + "/c/" + name + ".c")) src = store::optimizedDir(name) + "/c/" + name + ".c";
    else if (store::exists(store::decompDir(name) + "/cpp/" + name + ".cpp")) src = store::decompDir(name) + "/cpp/" + name + ".cpp";   // the faithful C++ decomp
    else src = store::decompDir(name) + "/c/" + name + ".c";
    string outdir = proj + "/recompiled"; store::mkdirs(outdir);
    string out = outdir + "/" + name; bool cpp = extOf(src) != "c";
    recompiling = true; g.showBottom = true;
    bool bundled = !toolchainDir().empty();
    { std::lock_guard<std::mutex> lk(optMx); optLines.push_back(string("recompile: ") + (bundled ? "[bundled clang] " : "") + (cpp ? "C++17" : "C11") + " " + store::base(src)); }
    std::thread([src, out, cpp]() {
        string cc = (cpp ? nativeCxx() + " -std=c++17 -O0" : nativeCc() + " -std=c11 -O0");
        string cmd = cc + " " + model::shq(src) + " -o " + model::shq(out) + " 2>&1";
        model::runCmdStream(cmd, [](const string& ln) { if (!model::trim(ln).empty()) { std::lock_guard<std::mutex> lk(optMx); optLines.push_back("  " + ln); } });
        { std::lock_guard<std::mutex> lk(optMx);
          optLines.push_back(store::exists(out) ? ("recompile: OK -> " + out) : "recompile: FAILED — raw decompiled source isn't compilable yet; run Clean Up (with Claude on) first"); }
        recompiling = false;
    }).detach();
}

// openFileInViewer + saveFile are defined below, after the DocView instances exist.
inline void saveFile();
inline void toggleSettings();   // fwd (defined with the overlays)
inline void togglePalette();
inline void goHome() { g.home = true; g.focus = F_NONE; g.paletteOpen = false; }
// ── config (window layout + settings) persisted to App Support/config.json ────
inline void saveConfig() {
    char b[600]; snprintf(b, sizeof b,
        "{\n  \"aiOptIn\": \"%s\",\n  \"structWidths\": \"%s\",\n  \"sideW\": \"%d\",\n  \"rightW\": \"%d\",\n  \"bottomH\": \"%d\",\n  \"showRight\": \"%s\",\n  \"showBottom\": \"%s\"\n}\n",
        g.aiOptIn ? "true" : "false", g.structWidths ? "true" : "false", (int)g.sideW, (int)g.rightW, (int)g.bottomH, g.showRight ? "true" : "false", g.showBottom ? "true" : "false");
    store::mkdirs(store::root()); store::writeFile(store::root() + "/config.json", b);
}
inline void loadConfig() {
    string js = store::readFile(store::root() + "/config.json"); if (js.empty()) { g.aiOptIn = store::loadAiOptIn(); return; }
    g.aiOptIn = store::jsonStr(js, "aiOptIn") == "true";
    g.structWidths = store::jsonStr(js, "structWidths") == "true";
    auto numC = [&](const string& k, float def, float lo, float hi) { string v = store::jsonStr(js, k); if (v.empty()) return def; float n = (float)atoi(v.c_str()); return (n < lo || n > hi) ? def : n; };
    g.sideW = numC("sideW", g.sideW, 160, 600); g.rightW = numC("rightW", g.rightW, 200, 4000); g.bottomH = numC("bottomH", g.bottomH, 90, 4000);
    string sr = store::jsonStr(js, "showRight"), sb = store::jsonStr(js, "showBottom");
    if (!sr.empty()) g.showRight = sr == "true"; if (!sb.empty()) g.showBottom = sb == "true";
}
inline void init(const string& launcherDir) { g.dir = launcherDir;
    { string udb = store::root() + "/sigs.db";   // ship a seed FLIRT DB (.app Resources OR a /usr/local install) -> signatures work out-of-the-box
      for (const string& bdb : { launcherDir + "../Resources/sigs.db", string("/usr/local/share/emberdragon/sigs.db") })
        if (!store::exists(udb) && store::exists(bdb)) { store::mkdirs(store::root()); store::writeFile(udb, model::readText(bdb)); } }
    g.recent = store::loadRecent(); auto ls = store::loadLastSession(); g.lastProject = ls.first; g.lastSource = ls.second; loadConfig(); }
    // (the grid VT emulator handles cursor-addressing now, so both terminals run a real xterm-256color
    //  TERM — lldb's editline AND its `gui` curses mode work, instead of the old dumb/linear fallback.)

// the real EmberDragon emblem (bundle Resources, or dev assets) -> texture, loaded once
inline img::Tex g_logo; inline bool g_logoTried = false;
inline img::Tex& logo() {
    if (!g_logoTried) { g_logoTried = true;
        for (const string& p : { g.dir + "../Resources/logo.png", string("/usr/local/share/emberdragon/logo.png"), g.dir + "assets/logo.png", g.dir + "../../../assets/logo.png" })
            if (store::exists(p)) { g_logo = img::load(p); if (g_logo.id) break; }
    }
    return g_logo;
}
// draw the logo fit into a box, preserving aspect; falls back to the vector mark
inline void drawLogo(float x, float y, float box) {
    img::Tex& t = logo();
    if (!t.id || !t.w || !t.h) { ui::icon(x, y + box * 0.06f, box * 0.88f, ui::IC_DRAGON, th::ACCENT); return; }
    float lw = box, lh = box; if (t.w >= t.h) lh = box * t.h / t.w; else lw = box * t.w / t.h;
    img::draw(t, x + (box - lw) / 2, y + (box - lh) / 2, lw, lh);
}

// ── syntax highlighting (shared) ─────────────────────────────────────────────
// control/storage keywords -> blue. Types are handled by isTy (teal) so they don't collide.
inline bool isKw(const string& w) { static const char* K[] = {
    "return","if","else","while","for","do","switch","case","default","break","continue","goto",
    "class","struct","union","enum","namespace","using","typedef","template","typename","this","operator",
    "public","private","protected","virtual","override","final","friend","explicit","inline","mutable",
    "static","const","constexpr","consteval","volatile","extern","register","thread_local","decltype",
    "new","delete","try","catch","throw","noexcept","sizeof","alignof","static_cast","reinterpret_cast",
    "const_cast","dynamic_cast","true","false","nullptr","NULL","and","or","not","xor", 0 };
    for (int i = 0; K[i]; i++) if (w == K[i]) return true; return false; }
// types -> teal: primitives, fixed-width, std:: containers, and Capitalised / S_ names (recovered structs).
inline bool isTy(const string& w) { static const char* T[] = {
    "int","long","char","short","void","unsigned","signed","float","double","bool","auto","wchar_t",
    "char8_t","char16_t","char32_t","size_t","ssize_t","ptrdiff_t","intptr_t","uintptr_t",
    "string","wstring","vector","map","set","unordered_map","unordered_set","deque","list","array","pair",
    "tuple","optional","variant","function","shared_ptr","unique_ptr","weak_ptr","FILE", 0 };
    for (int i = 0; T[i]; i++) if (w == T[i]) return true;
    if (w.find("int8")!=string::npos||w.find("int16")!=string::npos||w.find("int32")!=string::npos||w.find("int64")!=string::npos) return true;   // [u]intN_t
    return !w.empty() && (isupper((unsigned char)w[0]) || w.rfind("S_", 0) == 0); }   // Capitalised / recovered-struct names
// NOTE: emits into an open text batch (caller must bracket with ui::beginText()/endText()).
// Full C/C++ highlighter: preprocessor, line+block comments, string/char literals (escape-aware),
// hex/float/suffixed numbers, <include> brackets, keyword/type/function colouring, operators.
inline void drawCode(float x, float y, const string& l, float adv) {
    float cx = x;
    auto put = [&](const string& w, Col col) { ui::emit(cx, y, w, col); cx += w.size() * adv; };
    size_t i = 0; while (i < l.size() && (l[i] == ' ' || l[i] == '\t')) i++;        // leading indent
    if (i > 0) put(l.substr(0, i), th::TEXT);
    // preprocessor: the whole `#directive` line -> directive in keyword colour, <...>/"..." as string, rest dim
    if (i < l.size() && l[i] == '#') { size_t j = i + 1; while (j < l.size() && (isalnum((unsigned char)l[j]) || l[j]=='_' || l[j]==' ')) j++;
        put(l.substr(i, j - i), th::ACCENT2);                                       // `#include` / `#define` (pink accent)
        if (j < l.size()) { char c = l[j];
            if (c == '<') { size_t e = l.find('>', j); e = (e==string::npos)? l.size()-1 : e; put(l.substr(j, e-j+1), th::STR); j = e+1; }
            put(l.substr(j), th::STR); }
        return; }
    while (i < l.size()) { char ch = l[i];
        if (ch == '/' && i+1 < l.size() && l[i+1] == '/') { put(l.substr(i), th::CMT); break; }                 // line comment to EOL
        if (ch == '/' && i+1 < l.size() && l[i+1] == '*') { size_t e = l.find("*/", i+2); e = (e==string::npos)? l.size() : e+2; put(l.substr(i, e-i), th::CMT); i = e; continue; }   // /* … */
        if (ch == '"' || ch == '\'') { char q = ch; size_t j = i + 1; while (j < l.size()) { if (l[j]=='\\') { j += 2; continue; } if (l[j]==q) { j++; break; } j++; }   // escape-aware literal
            put(l.substr(i, j - i), th::STR); i = j; continue; }
        if (isalpha((unsigned char)ch) || ch == '_') { size_t j = i; while (j < l.size() && (isalnum((unsigned char)l[j]) || l[j]=='_')) j++; string w = l.substr(i, j - i);
            size_t k = j; while (k < l.size() && l[k]==' ') k++;                     // a name immediately before '(' is a call
            bool call = k < l.size() && l[k] == '(' && !isKw(w);
            put(w, isKw(w) ? th::KW : isTy(w) ? th::TYPE : call ? th::FNC : th::TEXT); i = j; continue; }
        if (isdigit((unsigned char)ch) || (ch=='.' && i+1<l.size() && isdigit((unsigned char)l[i+1]))) {        // number: hex/dec/float + suffix
            size_t j = i; if (ch=='0' && i+1<l.size() && (l[i+1]=='x'||l[i+1]=='X')) { j = i+2; while (j<l.size() && (isxdigit((unsigned char)l[j])||l[j]=='\'')) j++; }
            else { while (j<l.size() && (isdigit((unsigned char)l[j])||l[j]=='.'||l[j]=='\'')) j++; }
            while (j<l.size() && (l[j]=='u'||l[j]=='U'||l[j]=='l'||l[j]=='L'||l[j]=='f'||l[j]=='F'||l[j]=='e'||l[j]=='E'||l[j]=='+'||l[j]=='-') && (l[j]!='+'&&l[j]!='-' || (j>0&&(l[j-1]=='e'||l[j-1]=='E')))) j++;   // suffix / exponent
            put(l.substr(i, j - i), th::NUM); i = j; continue; }
        static const string OPS = "+-*/%=<>!&|^~?:";                                 // operators -> punctuation colour
        bool op = OPS.find(ch) != string::npos, punc = string("{}()[];,.").find(ch) != string::npos;
        put(string(1, ch), (op||punc) ? th::PUNC : th::TEXT); i++; }
}

// ── DocView: one reusable interactive text/code document ─────────────────────
// Every code view (pseudocode, file editor, disassembly) is a DocView: char-
// granular drag-selection, select-all + copy, wheel/scrollbar/PgUp-PgDn/Home-End
// scrolling, click-to-place caret, and (when editable) in-place typing. It
// borrows a vector<string>* — edits go straight into that buffer.
struct DocView;
inline DocView* activeDoc = nullptr;                       // the doc that owns the caret + keystrokes (fwd for run())
// ── clangd autocomplete popup state ──────────────────────────────────────────
struct ComplState {
    bool open = false; std::vector<lsp::Compl> items; std::vector<int> filt; int sel = 0;
    int anchorL = 0, anchorC = 0; std::string prefix;          // the word being completed: replace [anchor .. caret]
    float caretX = 0, caretY = 0, lineH = 0; DocView* owner = nullptr;
    void close() { open = false; items.clear(); filt.clear(); sel = 0; prefix.clear(); owner = nullptr; }
};
inline ComplState g_compl;
inline void complTrigger(DocView&); inline bool complHandleKey(DocView&); inline void complAccept(DocView&); inline void complRefilter();
inline void gotoDefinition(DocView&);
inline void applyAvailableFix();   // fwd — clangd quick-fix (click the tag or ⌘.)
inline string g_renamePath, g_renameOld, g_renameNew;   // in-flight clangd semantic rename: target file + old/new (pumpRename applies + propagates to the symbol model)
struct DocView {
    vector<string>* lines = nullptr;
    int scroll = 0, caretL = 0, caretC = 0;
    float hscroll = 0;                             // horizontal scroll offset in px (sideways scroll for long lines)
    float rTx = 0, rY = 0, rAdv = 0, rLh = 0;      // last render coords -> screen pos to caret (for right-click placement)
    void placeCaretAt(float mx, float my) { if (rLh <= 0) return; int L, C; hitPx(mx, my, rTx, rY, rAdv, rLh, L, C); caretL = L; caretC = C; clampCaret(); clearSel(); }
    int aL = -1, aC = 0, bL = -1, bC = 0;          // selection anchor(a)..head(b); aL<0 = none
    bool editable = false, syntax = true, dirty = false;
    string lspPath;   // if set, clangd diagnostics for this file are drawn (errors/warnings)
    string tag;                                    // what's currently bound; changing it resets scroll/caret/sel
    bool findOn = false; string findQ; vector<std::pair<int,int>> hits; int hitIdx = -1;   // find bar (Cmd+F)
    bool followCaret = false;                      // scroll to the caret only after a keyboard move, not on wheel
    bool renameOn = false; string renameOld, renameNew; int renameAnchorL = 0, renameAnchorC = 0;   // rename symbol (F2); anchor = caret pos on the symbol (for clangd semantic rename)
    bool propagateGlobal = true;                               // DEFAULT ON: renaming a function/type/class/global propagates program-wide (locals stay scoped). Toggle off in Settings to keep renames local.
    static bool idc(char c) { return isalnum((unsigned char)c) || c == '_'; }
    string tokenAtCaret() { if (!lines || caretL >= nlines()) return ""; const string& L = (*lines)[caretL]; int c = std::min(caretC, (int)L.size());
        int a = c, b = c; while (a > 0 && idc(L[a - 1])) a--; while (b < (int)L.size() && idc(L[b])) b++;
        if (a == b && a > 0) { int a2 = a; while (a2 > 0 && idc(L[a2 - 1])) a2--; if (a2 < a) return L.substr(a2, a - a2); }
        return a < b ? L.substr(a, b - a) : ""; }
    void startRename() { if (!editable) return; renameOld = tokenAtCaret(); if (renameOld.empty() || isdigit((unsigned char)renameOld[0])) return; renameNew = renameOld; renameOn = true; renameAnchorL = caretL; renameAnchorC = caretC; }
    // Enter = rename in THIS function only. Ctrl/Cmd+Enter = also PROPAGATE program-wide — but ONLY for a safe KIND
    // (function / type / class / struct / g_global / s_string). A local var/field (vN/tN/argN/lowercase) is NEVER
    // propagated even on Ctrl+Enter — that's how it recognizes var-vs-class-vs-struct-vs-string and won't break stuff.
    void applyRename(bool wantPropagate = false) { renameOn = false; if (!editable || !lines || renameOld.empty() || renameNew == renameOld) return;
        if (!lspPath.empty() && lsp::client.isAlive()) {                                  // FANCY: clangd semantic rename — renames the symbol everywhere it's REALLY used (scope/type aware)
            string text; for (size_t i = 0; i < lines->size(); i++) { text += (*lines)[i]; if (i + 1 < lines->size()) text += "\n"; }
            lsp::client.requestRename(lspPath, text, renameAnchorL, renameAnchorC, renameNew);
            g_renamePath = lspPath; g_renameOld = renameOld; g_renameNew = renameNew; return; }                // pumpRename also propagates old->new into the symbol model
        bool curName = g.selFn >= 0 && g.selFn < (int)g.bin.funcs.size() && renameOld == g.bin.funcs[g.selFn].name;
        auto isFn = [&](const string& s) { for (auto& f : g.bin.funcs) if (f.name == s) return true; return false; };
        bool glob = curName || isFn(renameOld) || isGlobalName(renameOld) || renameOld.rfind("s_", 0) == 0;   // fn / Type / struct / g_global / s_string
        bool propagate = wantPropagate && glob;
        if (wantPropagate && !glob) g.log("rename: '" + renameOld + "' is a local var/field -> kept inside this function (Ctrl+Enter only propagates functions/classes/structs/globals/strings)");
        auto repl = [&](string& L) { string r; size_t i = 0; while (i < L.size()) {                        // whole-token replace
            if (L.compare(i, renameOld.size(), renameOld) == 0 && (i == 0 || !idc(L[i - 1])) && (i + renameOld.size() >= L.size() || !idc(L[i + renameOld.size()]))) { r += renameNew; i += renameOld.size(); } else r += L[i++]; }
            L = r; };
        pushUndo();
        if (propagate) {                                                                                   // never silently MERGE onto an existing function/type name — de-collide first
            auto taken = [&](const string& nm) { for (auto& f : g.bin.funcs) if (f.name == nm) return true; return false; };
            if (taken(renameNew)) { string base = renameNew; int k = 2; while (taken(renameNew)) renameNew = base + std::to_string(k++);
                g.log("rename: '" + base + "' was already in use -> '" + renameNew + "'"); }
            for (auto& L : *lines) repl(L);                                                                // displayed buffer
            std::map<string,string> m{ { renameOld, renameNew } }; applyRenames(m); g.renameLog.push_back({ renameOld, renameNew }); g.bin.buildXrefs();   // + the whole program, logged for Propagate
        } else if (g.selFn >= 0 && g.selFn < (int)g.bin.funcs.size()) {                                    // LOCAL: this function only, in the model
            auto& f = g.bin.funcs[g.selFn]; repl(f.sig); for (auto& L : f.lines) repl(L); g.wholeVer++;    // (in fn-view *lines IS f.lines so it shows now; in whole-view the rebuild refreshes)
            g.fnLocalRenames[f.addr].push_back({ renameOld, renameNew });                                  // record (ordered) so a re-lift can replay your names
        } else { for (auto& L : *lines) repl(L); }                                                         // fallback: edit the bound buffer
        dirty = true; }

    // (re)bind a buffer; when the identity changes (new file / new function) reset view + caret
    void bind(vector<string>* v, const string& id) { lines = v; if (id != tag) { tag = id; scroll = caretL = caretC = 0; hscroll = 0; clearSel(); dirty = false; undoStack.clear(); redoStack.clear(); lastEdit = 0; renameOn = false; if (findOn) recomputeHits(); } }
    static string lower(string s) { for (auto& c : s) c = (char)tolower((unsigned char)c); return s; }
    void closeFind() { findOn = false; findQ.clear(); hits.clear(); hitIdx = -1; }
    void recomputeHits() { hits.clear(); hitIdx = -1; if (!lines || findQ.empty() || findQ[0] == ':') return; string q = lower(findQ);
        for (int li = 0; li < nlines(); li++) { string L = lower((*lines)[li]); size_t p = 0; while ((p = L.find(q, p)) != string::npos) { hits.push_back({li, (int)p}); p += q.size(); } }
        if (!hits.empty()) { hitIdx = 0; gotoHit(); } }
    void gotoHit() { if (hitIdx < 0 || hitIdx >= (int)hits.size()) return; auto h = hits[hitIdx];
        caretL = h.first; caretC = h.second; aL = h.first; aC = h.second; bL = h.first; bC = h.second + (int)findQ.size(); scroll = std::max(0, caretL - 3); }
    void stepHit(int d) { if (hits.empty()) return; hitIdx = (hitIdx + d + (int)hits.size()) % (int)hits.size(); gotoHit(); }
    int nlines() const { return lines ? (int)lines->size() : 0; }
    void clampCaret() { if (!lines || lines->empty()) { caretL = caretC = 0; return; }
        caretL = std::max(0, std::min(caretL, nlines() - 1)); caretC = std::max(0, std::min(caretC, (int)(*lines)[caretL].size())); }
    bool hasSel() const { return aL >= 0 && (aL != bL || aC != bC); }
    void clearSel() { aL = bL = -1; }
    void selectAll() { if (!lines || lines->empty()) return; aL = 0; aC = 0; bL = nlines() - 1; bC = (int)lines->back().size(); caretL = bL; caretC = bC; }
    void ordered(int& l0, int& c0, int& l1, int& c1) const {            // anchor..head -> top..bottom
        l0 = aL; c0 = aC; l1 = bL; c1 = bC; if (l0 > l1 || (l0 == l1 && c0 > c1)) { std::swap(l0, l1); std::swap(c0, c1); } }
    string selectedText() const {
        if (!hasSel() || !lines) return ""; int l0, c0, l1, c1; ordered(l0, c0, l1, c1); string out;
        for (int li = l0; li <= l1 && li < nlines(); li++) { const string& s = (*lines)[li];
            int a = li == l0 ? c0 : 0, b = li == l1 ? c1 : (int)s.size(); a = std::max(0, std::min(a, (int)s.size())); b = std::max(a, std::min(b, (int)s.size()));
            out += s.substr(a, b - a); if (li != l1) out += "\n"; }
        return out; }
    void setSelToCaret(bool extend) { if (extend) { if (aL < 0) { aL = caretL; aC = caretC; } bL = caretL; bC = caretC; } else clearSel(); }

    // pixel -> (line,col) within the text area (textX = where glyph col 0 starts, cy = top of first row)
    void hitPx(float mx, float my, float textX, float cy, float adv, float lh, int& L, int& C) {
        L = scroll + (int)((my - cy) / lh); L = std::max(0, std::min(L, std::max(0, nlines() - 1)));
        C = (int)((mx - textX) / adv + 0.5f); C = std::max(0, std::min(C, lines && L < nlines() ? (int)(*lines)[L].size() : 0)); }

    void keys() {                                                       // only the focused doc gets called
        if (!lines || lines->empty()) return; clampCaret();
        if (ui::in.key || ui::in.ch) followCaret = true;               // any keyboard activity -> re-center on the caret this frame
        if (findOn) {                                                   // find bar owns the keys while open
            if (ui::in.ch) { unsigned c = ui::in.ch; if (c == 8 || c == 127) { if (!findQ.empty()) findQ.pop_back(); recomputeHits(); } else if (c >= 32 && c < 127) { findQ += (char)c; recomputeHits(); } }
            int kk = ui::in.key;
            if (kk == ui::K_ENTER) { if (!findQ.empty() && findQ[0] == ':') { int ln = atoi(findQ.c_str() + 1); if (ln > 0) { caretL = std::min(nlines() - 1, ln - 1); caretC = 0; scroll = std::max(0, caretL - 3); clearSel(); closeFind(); } }
                                     else stepHit(ui::in.shift ? -1 : 1); }
            else if (kk == ui::K_ESC) closeFind();
            return;
        }
        if (renameOn) {                                                // rename input owns the keys while open
            if (ui::in.ch) { unsigned c = ui::in.ch; if (c == 8 || c == 127) { if (!renameNew.empty()) renameNew.pop_back(); } else if (c < 128 && idc((char)c)) renameNew += (char)c; }
            if (ui::in.key == ui::K_ENTER) applyRename(ui::in.ctrl); else if (ui::in.key == ui::K_ESC) renameOn = false;   // Ctrl/Cmd+Enter = propagate program-wide (kind-aware)
            return;
        }
        if (g_compl.open && g_compl.owner == this && complHandleKey(*this)) return;   // autocomplete popup owns nav/accept/esc while open
        int k = ui::in.key; bool sh = ui::in.shift; int rows = 1;
        auto moveTo = [&](int L, int C){ caretL = L; caretC = C; clampCaret(); setSelToCaret(sh); };
        if (k == ui::K_LEFT) { if (caretC > 0) moveTo(caretL, caretC - 1); else if (caretL > 0) moveTo(caretL - 1, (int)(*lines)[caretL - 1].size()); }
        else if (k == ui::K_RIGHT) { if (caretC < (int)(*lines)[caretL].size()) moveTo(caretL, caretC + 1); else if (caretL + 1 < nlines()) moveTo(caretL + 1, 0); }
        else if (k == ui::K_UP) { if (caretL > 0) moveTo(caretL - 1, caretC); }
        else if (k == ui::K_DOWN) { if (caretL + 1 < nlines()) moveTo(caretL + 1, caretC); }
        else if (k == ui::K_PGUP) { moveTo(std::max(0, caretL - 20), caretC); scroll = std::max(0, scroll - 20); }
        else if (k == ui::K_PGDN) { moveTo(std::min(nlines() - 1, caretL + 20), caretC); scroll += 20; }
        else if (k == ui::K_HOME) { moveTo(caretL, 0); }
        else if (k == ui::K_END) { moveTo(caretL, (int)(*lines)[caretL].size()); }
        if (k == ui::K_LEFT || k == ui::K_RIGHT || k == ui::K_UP || k == ui::K_DOWN || k == ui::K_HOME || k == ui::K_END) lastEdit = 0;   // cursor move ends a typing run
        if (!editable) return;
        if (ui::in.ch) { unsigned ch = ui::in.ch; string& line = (*lines)[caretL];
            if (ch == 8 || ch == 127) { if (lastEdit != 'd') pushUndo(); lastEdit = 'd'; if (hasSel()) deleteSel(); else if (caretC > 0) { line.erase(caretC - 1, 1); caretC--; dirty = true; }
                else if (caretL > 0) { caretC = (int)(*lines)[caretL - 1].size(); (*lines)[caretL - 1] += line; lines->erase(lines->begin() + caretL); caretL--; dirty = true; } }
            else if (ch >= 32 && ch < 127) { if (lastEdit != 'i') pushUndo(); lastEdit = 'i'; if (hasSel()) deleteSel(); (*lines)[caretL].insert((*lines)[caretL].begin() + caretC, (char)ch); caretC++; dirty = true; } }
        if (k == ui::K_ENTER) { pushUndo(); lastEdit = 0; if (hasSel()) deleteSel(); string& line = (*lines)[caretL]; string rest = line.substr(caretC); line.erase(caretC); lines->insert(lines->begin() + caretL + 1, rest); caretL++; caretC = 0; dirty = true; }
        if (editable && !lspPath.empty() && ui::in.ch) { unsigned ch = ui::in.ch;                       // live autocomplete: re-query clangd as you type
            if ((ch < 128 && (isalnum(ch) || ch == '_')) || ch == '.' || ch == '>' || ch == ':') complTrigger(*this);
            else if ((ch == 8 || ch == 127) && g_compl.open) complTrigger(*this);                       // backspace -> re-eval / narrow
            else if (ch == 32 || ch == 40 || ch == 59) g_compl.close(); }                                // space / '(' / ';' dismiss
    }
    // ── undo/redo (snapshot stack) + paste ──
    vector<std::pair<vector<string>, std::pair<int,int>>> undoStack, redoStack; char lastEdit = 0;
    void pushUndo() { if (!lines) return; if (undoStack.size() > 250) undoStack.erase(undoStack.begin()); undoStack.push_back({ *lines, { caretL, caretC } }); redoStack.clear(); }
    void undo() { if (!lines || undoStack.empty()) return; redoStack.push_back({ *lines, { caretL, caretC } }); auto& t = undoStack.back(); *lines = t.first; caretL = t.second.first; caretC = t.second.second; undoStack.pop_back(); clampCaret(); clearSel(); dirty = true; lastEdit = 0; }
    void redo() { if (!lines || redoStack.empty()) return; undoStack.push_back({ *lines, { caretL, caretC } }); auto& t = redoStack.back(); *lines = t.first; caretL = t.second.first; caretC = t.second.second; redoStack.pop_back(); clampCaret(); clearSel(); dirty = true; lastEdit = 0; }
    void paste(const string& t) { if (!editable || !lines || t.empty()) return; clampCaret(); pushUndo(); lastEdit = 0; if (hasSel()) deleteSel();
        size_t i = 0; while (i < t.size()) { size_t e = t.find('\n', i); string seg = e == string::npos ? t.substr(i) : t.substr(i, e - i);
            for (char ch : seg) if (ch != '\r') { (*lines)[caretL].insert((*lines)[caretL].begin() + caretC, ch); caretC++; }
            if (e == string::npos) break; string rest = (*lines)[caretL].substr(caretC); (*lines)[caretL].erase(caretC); lines->insert(lines->begin() + caretL + 1, rest); caretL++; caretC = 0; i = e + 1; }
        dirty = true; }
    void deleteSel() { if (!hasSel() || !lines || lines->empty()) return; int l0, c0, l1, c1; ordered(l0, c0, l1, c1);
        int n = nlines();                                              // clamp the (possibly stale) selection to the live buffer
        l0 = std::max(0, std::min(l0, n - 1)); l1 = std::max(0, std::min(l1, n - 1)); if (l0 > l1) std::swap(l0, l1);
        c0 = std::max(0, std::min(c0, (int)(*lines)[l0].size())); c1 = std::max(0, std::min(c1, (int)(*lines)[l1].size()));
        if (l0 == l1) { if (c1 < c0) std::swap(c0, c1); (*lines)[l0].erase(c0, c1 - c0); }
        else { (*lines)[l0] = (*lines)[l0].substr(0, c0) + (*lines)[l1].substr(c1); lines->erase(lines->begin() + l0 + 1, lines->begin() + l1 + 1); }
        caretL = l0; caretC = c0; clearSel(); dirty = true; }

    // draw + mouse. content rect (x,y,w,h). focused = this doc owns the caret/keys.
    void run(float x, float y, float w, float h, bool focused, bool gutter = true) {
        float s = ui::in.scale, adv = ui::advance(), lh = ui::lineH() * 1.05f, gut = gutter ? adv * 6 : 8 * s;
        if (gutter) { ui::rect(x, y, gut, h, th::SUNKEN); ui::vline(x + gut, y, h, th::BORDER); }
        float textX = x + gut + (gutter ? adv : 0);
        int rows = (int)(h / lh), n = nlines();
        bool over = ui::hovered(x, y, w, h);
        if (over) { scroll += (int)ui::in.wheel; hscroll += ui::in.wheelX * adv; }
        float viewW = w - gut - 9 * s, hmax = 0;                                     // widest visible line -> horizontal scroll bound
        for (int r = 0; r < rows; r++) { int li = scroll + r; if (li >= n) break; if (li >= 0) hmax = std::max(hmax, (float)(*lines)[li].size() * adv + adv * 2); }
        if (hscroll > hmax - viewW) hscroll = std::max(0.0f, hmax - viewW); if (hscroll < 0) hscroll = 0;
        float tx = textX - hscroll;                                                  // scrolled text origin (gutter/line-numbers stay put)
        rTx = tx; rY = y; rAdv = adv; rLh = lh;                                       // remember for placeCaretAt (right-click go-to-def / fix)
        // mouse selection / caret placement
        if (!ui::menuActive && ui::inside(x, y, w - 8 * s, h) && ui::in.lPress) {
            int L, C; hitPx(ui::in.mx, ui::in.my, tx, y, adv, lh, L, C);
            if (ui::in.shift && aL >= 0) { caretL = bL = L; caretC = bC = C; }            // shift+click -> extend selection from the anchor
            else { caretL = L; caretC = C; aL = L; aC = C; bL = L; bC = C; }              // plain click -> start a collapsed selection
            g.focus = F_DOC; activeDoc = this; focused = true;
        } else if (focused && ui::in.lDown && ui::inside(x, y, w, h)) {       // drag -> extend head
            int L, C; hitPx(ui::in.mx, ui::in.my, tx, y, adv, lh, L, C); caretL = bL = L; caretC = bC = C; }
        if (focused && followCaret) { if (caretL < scroll) scroll = caretL; if (caretL >= scroll + rows) scroll = caretL - rows + 1; followCaret = false; }   // re-center only after a keyboard move, so the wheel can scroll freely
        if (scroll > n - 1) scroll = std::max(0, n - 1); if (scroll < 0) scroll = 0;
        ui::pushClip(x, y, w, h);
        if (gutter) { ui::beginText();                                                                     // line numbers — FIXED x, never horizontally scrolled
            for (int r = 0; r < rows; r++) { int li = scroll + r; if (li >= n) break; float ry = y + r * lh;
                char ln[12]; snprintf(ln, sizeof ln, "%4d", li + 1); ui::emitRight(x + gut - 5 * s, ry, ln, (focused && li == caretL) ? th::TEXT_DIM : th::TEXT_MUT); } ui::endText(); }
        ui::pushClip(x + gut, y, w - gut - 7 * s, h);                                                       // content region: text/sel/caret scroll here (clipped off the gutter)
        int l0 = 0, c0 = 0, l1 = -1, c1 = 0; if (hasSel()) ordered(l0, c0, l1, c1);
        for (int r = 0; r < rows; r++) { int li = scroll + r; if (li >= n) break; float ry = y + r * lh;   // pass 1: caret-line + selection rects
            if (focused && li == caretL && !hasSel()) ui::rect(x + gut, ry, w - gut, lh, th::Col{0.16f,0.17f,0.20f});
            if (hasSel() && li >= l0 && li <= l1) { const string& str = (*lines)[li];
                int a = li == l0 ? c0 : 0, b = li == l1 ? c1 : (int)str.size();
                float rx = tx + a * adv, rw = std::max(adv * 0.4f, (b - a) * adv); ui::rect(rx, ry, rw, lh, th::SEL_DIM); }
            if (focused && li == caretL) { float cxp = tx + caretC * adv; ui::rect(cxp, ry + 1 * s, std::max(1.5f * s, 1.0f), lh - 2 * s, th::ACCENT);
                if (g_compl.open && g_compl.owner == this) { g_compl.caretX = tx + g_compl.anchorC * adv; g_compl.caretY = ry + lh; g_compl.lineH = lh; } } }
        if (findOn && !findQ.empty() && findQ[0] != ':') for (int hi = 0; hi < (int)hits.size(); hi++) {   // highlight all find matches
            int li = hits[hi].first; if (li < scroll || li >= scroll + rows) continue; float ry = y + (li - scroll) * lh;
            ui::rect(tx + hits[hi].second * adv, ry, findQ.size() * adv, lh, hi == hitIdx ? th::Col{0.85f,0.55f,0.15f,0.55f} : th::Col{0.85f,0.75f,0.2f,0.28f}); }
        std::vector<lsp::Diag> diags; if (!lspPath.empty()) diags = lsp::client.diagnostics(lspPath);      // clangd errors/warnings
        for (auto& d : diags) { if (d.line < scroll || d.line >= scroll + rows) continue; float ry = y + (d.line - scroll) * lh;
            const string& str = d.line < n ? (*lines)[d.line] : *lines->begin();
            float uw = std::max(adv * 3, (float)str.size() * adv); th::Col uc = d.severity == 1 ? th::RED : th::Col{0.85f,0.72f,0.22f};
            ui::rect(tx, ry + lh - 2.5f * s, uw, 1.6f * s, th::Col{uc.r, uc.g, uc.b, 0.8f}); }                // underline the diagnostic line
        ui::beginText();                                                                                   // pass 2: code text (horizontally scrolled by tx)
        for (int r = 0; r < rows; r++) { int li = scroll + r; if (li >= n) break; float ry = y + r * lh;
            if (syntax) drawCode(tx, ry, (*lines)[li], adv); else ui::emit(tx, ry, (*lines)[li], th::TEXT); }
        ui::endText();
        ui::popClip();                                                                                     // content
        ui::popClip();                                                                                     // overall
        for (auto& d : diags) { if (d.line < scroll || d.line >= scroll + rows) continue; float ry = y + (d.line - scroll) * lh;   // gutter dot + fix tag + hover tooltip
            th::Col uc = d.severity == 1 ? th::RED : th::Col{0.85f,0.72f,0.22f}; ui::rect(x + 2 * s, ry + lh * 0.32f, 3.5f * s, lh * 0.38f, uc);
            if (!d.fix.empty()) { string tag = "  Fix available  ";                                            // a clangd quick-fix exists -> a CLICKABLE tag at the line's right edge
                float tw = ui::textW(tag) + 12 * s, tx2 = x + w - tw - 9 * s; bool hov = ui::in.mx >= tx2 && ui::in.mx <= tx2 + tw && ui::in.my >= ry && ui::in.my < ry + lh;
                ui::rect(tx2, ry + 1 * s, tw, lh - 2 * s, hov ? th::Col{0.32f,0.24f,0.07f,0.96f} : th::Col{0.20f,0.16f,0.06f,0.92f}); ui::rectLine(tx2, ry + 1 * s, tw, lh - 2 * s, th::ACCENT);
                ui::text(tx2 + 6 * s, ry, tag, th::ACCENT);
                if (ui::clicked(tx2, ry + 1 * s, tw, lh - 2 * s)) { caretL = d.line; activeDoc = this; applyAvailableFix(); } }   // click -> apply the fix
            if (ui::in.mx >= x + gut && ui::in.mx <= x + w && ui::in.my >= ry && ui::in.my < ry + lh)
                ui::setTip(ui::in.mx, ry + lh, (d.severity == 1 ? "error: " : "warning: ") + d.msg + (d.fix.empty() ? "" : "   (click \"Fix available\" -> " + d.fixTitle + ")")); }
        ui::scrollbar(x + w - 7 * s, y, 7 * s, h, n, rows, &scroll);
        if (hmax > viewW) { float fr = viewW / hmax, bw2 = std::max(24 * s, fr * viewW), bx2 = x + gut + (hmax > viewW ? (hscroll / (hmax - viewW)) * (viewW - bw2) : 0);   // horizontal scrollbar (bottom)
            ui::rect(x + gut, y + h - 5 * s, viewW, 5 * s, th::Col{0.09f,0.09f,0.11f,0.85f}); ui::rect(bx2, y + h - 5 * s, bw2, 5 * s, th::Col{0.38f,0.38f,0.48f,0.95f}); }
        if (findOn) {                                                                                      // find bar overlay (top-right)
            float bw = 230 * s, bh = ui::lineH() + 10 * s, bx = x + w - bw - 14 * s, byf = y + 6 * s;
            ui::rect(bx, byf, bw, bh, th::Col{0.12f,0.12f,0.15f,0.98f}); ui::rectLine(bx, byf, bw, bh, th::ACCENT);
            ui::icon(bx + 6 * s, byf + (bh - 12 * s) / 2, 12 * s, ui::IC_SEARCH, th::TEXT_MUT);
            ui::text(bx + 24 * s, byf + (bh - ui::lineH()) / 2, findQ.empty() ? "find  (or :line)" : findQ, findQ.empty() ? th::TEXT_MUT : th::TEXT);
            char cnt[24]; snprintf(cnt, sizeof cnt, "%d/%d", hits.empty() ? 0 : hitIdx + 1, (int)hits.size());
            ui::textRight(bx + bw - 8 * s, byf + (bh - ui::lineH()) / 2, cnt, th::TEXT_MUT);
        }
        if (renameOn) {                                                                                   // rename input overlay (top-right)
            float bw = std::min(390 * s, w - 16 * s), bh = ui::lineH() * 2 + 12 * s;                       // fit within the panel even when narrow
            float bx = std::max(x + 4 * s, x + w - bw - 14 * s), byf = y + 6 * s;                          // clamp so a long name never pushes it off-panel
            ui::rect(bx, byf, bw, bh, th::Col{0.12f,0.12f,0.15f,0.98f}); ui::rectLine(bx, byf, bw, bh, th::ACCENT2);
            ui::textClip(bx + 8 * s, byf + 5 * s, bw - 16 * s, "rename " + renameOld + " \xe2\x86\x92 " + renameNew + "_", th::TEXT);   // clip: never spill out of bounds
            ui::textClip(bx + 8 * s, byf + 5 * s + ui::lineH(), bw - 16 * s, "Enter = this fn   \xc2\xb7   \xe2\x8c\x98/Ctrl+Enter = everywhere", th::TEXT_MUT);
        }
    }
};
inline DocView docPseudo, docFile, docDisasm;              // the three text views

// ── autocomplete: trigger / filter / accept / render ─────────────────────────
inline void complRefilter() { g_compl.filt.clear();
    for (int i = 0; i < (int)g_compl.items.size(); i++) { const string& l = !g_compl.items[i].insert.empty() ? g_compl.items[i].insert : g_compl.items[i].label;
        if (g_compl.prefix.empty() || strncasecmp(l.c_str(), g_compl.prefix.c_str(), g_compl.prefix.size()) == 0) g_compl.filt.push_back(i); }
    if (g_compl.sel >= (int)g_compl.filt.size()) g_compl.sel = 0; }
inline void complTrigger(DocView& dv) {
    if (!dv.lines || dv.caretL >= dv.nlines() || dv.lspPath.empty()) return;
    const string& L = (*dv.lines)[dv.caretL]; int c = std::min(dv.caretC, (int)L.size());
    int a = c; while (a > 0 && (isalnum((unsigned char)L[a - 1]) || L[a - 1] == '_')) a--;
    g_compl.anchorL = dv.caretL; g_compl.anchorC = a; g_compl.prefix = L.substr(a, c - a); g_compl.owner = &dv;
    complRefilter();                                                                   // narrow the existing list instantly while clangd re-queries
    string text; for (size_t i = 0; i < dv.lines->size(); i++) { text += (*dv.lines)[i]; if (i + 1 < dv.lines->size()) text += "\n"; }
    lsp::client.requestCompletion(dv.lspPath, text, dv.caretL, dv.caretC);
}
inline void complAccept(DocView& dv) {
    if (g_compl.filt.empty() || !dv.lines || g_compl.anchorL >= dv.nlines()) { g_compl.close(); return; }
    const lsp::Compl& it = g_compl.items[g_compl.filt[g_compl.sel]]; string ins = !it.insert.empty() ? it.insert : it.label;
    string& L = (*dv.lines)[g_compl.anchorL]; int a = g_compl.anchorC, c = std::min(dv.caretC, (int)L.size());
    if (a >= 0 && a <= c && c <= (int)L.size()) { dv.pushUndo(); L.erase(a, c - a); L.insert(a, ins); dv.caretC = a + (int)ins.size(); dv.caretL = g_compl.anchorL; dv.dirty = true; }
    g_compl.close();
}
inline bool complHandleKey(DocView& dv) {
    if (!g_compl.open) return false; int k = ui::in.key;
    if (k == ui::K_ESC) { g_compl.close(); return true; }
    if (k == ui::K_UP)   { if (!g_compl.filt.empty()) g_compl.sel = (g_compl.sel - 1 + (int)g_compl.filt.size()) % (int)g_compl.filt.size(); return true; }
    if (k == ui::K_DOWN) { if (!g_compl.filt.empty()) g_compl.sel = (g_compl.sel + 1) % (int)g_compl.filt.size(); return true; }
    if ((k == ui::K_ENTER || ui::in.ch == 9) && !g_compl.filt.empty()) { complAccept(dv); return true; }
    if (k == ui::K_LEFT || k == ui::K_RIGHT || k == ui::K_HOME || k == ui::K_END || k == ui::K_PGUP || k == ui::K_PGDN) g_compl.close();
    return false;                                                                     // typing/backspace fall through to edit, then re-trigger
}
inline int g_complSeen = 0;
inline void pumpCompletion() {
    int e = lsp::client.complEpoch.load(); if (e == g_complSeen) return; g_complSeen = e;
    if (!g_compl.owner) return; g_compl.items = lsp::client.completions(); complRefilter(); g_compl.open = !g_compl.filt.empty();
}
inline void drawCompletion() {
    if (!g_compl.open || g_compl.filt.empty() || g_compl.lineH <= 0) return; float s = ui::in.scale, lh = g_compl.lineH, adv = ui::advance();
    int total = (int)g_compl.filt.size(), maxRows = std::min(total, 12);
    float bw = 150 * s; for (int r = 0; r < total; r++) { const auto& it = g_compl.items[g_compl.filt[r]]; bw = std::max(bw, (it.label.size() + (it.detail.empty() ? 0 : it.detail.size() + 2)) * adv + 22 * s); }
    bw = std::min(bw, 560 * s); float bh = maxRows * lh + 8 * s, bx = g_compl.caretX, by = g_compl.caretY + 2 * s;
    if (bx + bw > ui::winW) bx = ui::winW - bw - 4 * s; if (bx < 0) bx = 4 * s;
    if (by + bh > ui::winH) by = g_compl.caretY - lh - bh - 2 * s;                     // flip above the caret if no room below
    ui::rect(bx, by, bw, bh, th::Col{0.13f, 0.13f, 0.16f, 0.99f}); ui::rectLine(bx, by, bw, bh, th::ACCENT);
    int top = std::max(0, std::min(g_compl.sel - maxRows / 2, total - maxRows));
    for (int r = 0; r < maxRows; r++) { int idx = top + r; if (idx >= total) break; const auto& it = g_compl.items[g_compl.filt[idx]]; float ry = by + 4 * s + r * lh;
        if (idx == g_compl.sel) ui::rect(bx + 2 * s, ry, bw - 4 * s, lh, th::SEL_DIM);
        th::Col kc = it.kind == 3 ? th::FNC : it.kind == 7 || it.kind == 22 ? th::TYPE : it.kind == 5 ? th::ACCENT2 : th::Col{0.74f, 0.78f, 0.82f};   // fn/type/field tint
        ui::text(bx + 8 * s, ry, it.label, kc);
        if (!it.detail.empty()) ui::textRight(bx + bw - 8 * s, ry, it.detail, th::TEXT_MUT); }
    if (total > maxRows) ui::scrollbar(bx + bw - 6 * s, by, 6 * s, bh, total, maxRows, &top);
}
// ── go to definition (Cmd+click) ─────────────────────────────────────────────
inline int g_defSeen = 0;
inline void gotoDefinition(DocView& dv) { if (dv.lspPath.empty() || !dv.lines || dv.caretL >= dv.nlines()) return; lsp::client.requestDefinition(dv.lspPath, dv.caretL, dv.caretC); g.log("go to definition…"); }
inline void pumpDefinition() {
    int e = lsp::client.defEpoch.load(); if (e == g_defSeen) return; g_defSeen = e;
    lsp::Loc l = lsp::client.definition(); if (!l.ok) { g.log("definition: not found"); return; }
    if (l.path != g.fileViewPath && store::exists(l.path)) openFileInViewer(l.path);
    int n = (int)g.editLines.size(); docFile.caretL = std::max(0, std::min(l.line, n - 1)); docFile.caretC = std::max(0, l.col);
    docFile.scroll = std::max(0, docFile.caretL - 6); docFile.clearSel(); g.mainTab = MAIN_FILE; g.focus = F_DOC; activeDoc = &docFile;
    g.log("-> definition at " + store::base(l.path) + ":" + std::to_string(l.line + 1));
}
// ── "fix available" — apply a clangd quick-fix (⌘.) ──────────────────────────
inline void applyEditsToLines(vector<string>& L, const vector<lsp::TextEdit>& edits) {   // pure: apply LSP text edits to a line buffer
    if (edits.empty()) return; if (L.empty()) L.push_back("");
    vector<lsp::TextEdit> es = edits;                                                     // bottom-up so earlier edits don't shift later positions
    std::sort(es.begin(), es.end(), [](const lsp::TextEdit& a, const lsp::TextEdit& b) { return a.line != b.line ? a.line > b.line : a.sc > b.sc; });
    for (auto& e : es) { int n = (int)L.size(); int l0 = e.line, c0 = e.sc, l1 = std::max(e.eline, e.line), c1 = e.ec; if (l0 < 0 || l0 >= n) continue; l1 = std::min(l1, n - 1);
        c0 = std::min(std::max(0, c0), (int)L[l0].size()); c1 = std::min(std::max(0, c1), (int)L[l1].size());
        string prefix = L[l0].substr(0, c0), suffix = L[l1].substr(c1);
        L.erase(L.begin() + l0, L.begin() + l1 + 1);                                       // remove the replaced span (l0..l1)
        string combined = prefix + e.newText + suffix; vector<string> nl; size_t i = 0;    // SPLIT on '\n' so a multi-line newText (e.g. "add #include <x>\n") becomes real lines, not one mashed line
        while (true) { size_t p = combined.find('\n', i); if (p == string::npos) { nl.push_back(combined.substr(i)); break; } nl.push_back(combined.substr(i, p - i)); i = p + 1; }
        L.insert(L.begin() + l0, nl.begin(), nl.end()); }
}
inline void applyTextEdits(DocView& dv, const vector<lsp::TextEdit>& edits) { if (!dv.lines || edits.empty()) return; dv.pushUndo(); applyEditsToLines(*dv.lines, edits); dv.dirty = true; dv.clampCaret(); }
inline int g_renameSeen = 0;
inline void pumpRename() {   // apply a finished clangd semantic-rename WorkspaceEdit (current file in the editor + any other files on disk)
    int e = lsp::client.renameEpoch.load(); if (e == g_renameSeen) return; g_renameSeen = e;
    auto edits = lsp::client.renameEdits(); if (edits.empty()) { g.log("rename: clangd found nothing to change (not a renameable symbol here?)"); return; }
    auto realPath = [](const string& p) -> string {
#ifndef _WIN32
        char rp[4096]; if (realpath(p.c_str(), rp)) return string(rp);
#endif
        return p; };
    string openRP = realPath(docFile.lspPath.empty() ? g_renamePath : docFile.lspPath); std::set<string> done;
    int files = 0, total = 0;
    for (auto& kv : edits) { string rp = realPath(kv.first); if (done.count(rp)) continue; done.insert(rp);   // dedupe symlink-aliased paths so we never double-apply
        total += (int)kv.second.size(); files++;
        if (rp == openRP) { applyTextEdits(docFile, kv.second);                            // the file open in the editor -> patch the live buffer
            string t; for (size_t i = 0; i < g.editLines.size(); i++) { t += g.editLines[i]; if (i + 1 < g.editLines.size()) t += "\n"; }
            store::writeFile(docFile.lspPath, t); docFile.dirty = false; g.fileDirty = false; lspChange(docFile.lspPath, t); }
        else if (store::exists(kv.first)) {                                               // a different file clangd wants to touch -> read, apply, write
            string raw = store::readFile(kv.first); vector<string> ls; size_t i = 0; while (true) { size_t nn = raw.find('\n', i); if (nn == string::npos) { ls.push_back(raw.substr(i)); break; } ls.push_back(raw.substr(i, nn - i)); i = nn + 1; }
            applyEditsToLines(ls, kv.second); string out; for (size_t k = 0; k < ls.size(); k++) { out += ls[k]; if (k + 1 < ls.size()) out += "\n"; } store::writeFile(kv.first, out); } }
    // PROPAGATE: also rename old->new in the decompiler's symbol model (pseudocode funcs + FLIRT log), so the
    // rename shows everywhere AND survives a re-analyze / re-decompile — not just in the edited file text.
    if (!g_renameOld.empty() && g_renameOld != g_renameNew) { applyRenames({ { g_renameOld, g_renameNew } }); g.renameLog.push_back({ g_renameOld, g_renameNew }); g.bin.buildXrefs(); }
    g_renameOld.clear(); g_renameNew.clear();
    g.log("\xe2\x9c\x93 renamed " + std::to_string(total) + " occurrence(s) across " + std::to_string(files) + " file(s)  (+ propagated to the symbol model)");
}
inline void applyAvailableFix() {
    if (activeDoc != &docFile || docFile.lspPath.empty()) return;
    auto diags = lsp::client.diagnostics(docFile.lspPath); const lsp::Diag* pick = nullptr;
    for (auto& d : diags) if (d.line == docFile.caretL && !d.fix.empty()) { pick = &d; break; }    // prefer the fix on the caret line
    if (!pick) for (auto& d : diags) if (!d.fix.empty()) { pick = &d; break; }
    if (!pick) { g.log("no quick-fix available here"); return; }
    string title = pick->fixTitle; applyTextEdits(docFile, pick->fix);
    lsp::client.clearDiagnostics(docFile.lspPath);                                        // hide the stale tag NOW so a second click can't re-apply
    string t; for (size_t i = 0; i < g.editLines.size(); i++) { t += g.editLines[i]; if (i + 1 < g.editLines.size()) t += "\n"; }
    lspChange(docFile.lspPath, t); g.fileDirty = docFile.dirty = true;
    g.log("✓ applied fix: " + title);
}
// ── drag-to-select + copy for the plain text panels (log / terminal / debugger) ──
// These aren't DocViews; this lightweight helper gives them char-granular mouse selection + Cmd+C copy,
// without disturbing their existing rendering or (for the PTYs) keyboard routing.
struct LineSel { int aL = -1, aC = 0, bL = 0, bC = 0; bool drag = false;
    bool on() const { return aL >= 0 && (aL != bL || aC != bC); }
    void ord(int& l0, int& c0, int& l1, int& c1) const {
        if (aL < bL || (aL == bL && aC <= bC)) { l0 = aL; c0 = aC; l1 = bL; c1 = bC; } else { l0 = bL; c0 = bC; l1 = aL; c1 = aC; } }
    void clear() { aL = -1; drag = false; } };
inline LineSel* g_lineSel = nullptr; inline const vector<string>* g_lineSelLines = nullptr;   // the panel that owns the live selection (last drag-start wins)
inline string lineSelText(const LineSel& s, const vector<string>& lines) { if (!s.on()) return ""; int l0, c0, l1, c1; s.ord(l0, c0, l1, c1); string r;
    for (int L = l0; L <= l1 && L < (int)lines.size(); L++) { if (L < 0) continue; const string& ln = lines[L];
        int a = L == l0 ? c0 : 0, b = L == l1 ? c1 : (int)ln.size(); a = std::min(a, (int)ln.size()); b = std::min(b, (int)ln.size());
        r += ln.substr(a, std::max(0, b - a)); if (L < l1) r += "\n"; } return r; }
// handle mouse press/drag over a panel and draw the highlight. (px,py,pw,ph)=panel rect; (tx,ty)=origin of the
// first VISIBLE row; firstLine=its line index; adv/lh=metrics. Call inside the panel's clip, before its text.
inline void runLineSel(LineSel& sel, const vector<string>& lines, float px, float py, float pw, float ph,
                       float tx, float ty, int firstLine, float adv, float lh) {
    float s = ui::in.scale; (void)s;
    auto hit = [&](float mx, float my, int& L, int& C) { L = firstLine + (int)((my - ty) / lh); if (L < 0) L = 0; if (L >= (int)lines.size()) L = (int)lines.size() - 1;
        C = (int)((mx - tx) / adv + 0.5f); if (C < 0) C = 0; if (L >= 0 && L < (int)lines.size() && C > (int)lines[L].size()) C = (int)lines[L].size(); };
    bool inside = ui::in.mx >= px && ui::in.mx <= px + pw && ui::in.my >= py && ui::in.my <= py + ph;
    if (ui::in.lPress && inside && !ui::menuActive) {
        if (ui::in.shift && sel.aL >= 0) hit(ui::in.mx, ui::in.my, sel.bL, sel.bC);                            // shift+click -> extend from the existing anchor
        else { hit(ui::in.mx, ui::in.my, sel.aL, sel.aC); sel.bL = sel.aL; sel.bC = sel.aC; }
        sel.drag = true; g_lineSel = &sel; g_lineSelLines = &lines; }
    if (sel.drag && ui::in.lDown) { float my = std::max(py, std::min(py + ph, ui::in.my)); hit(ui::in.mx, my, sel.bL, sel.bC); }
    if (!ui::in.lDown) sel.drag = false;
    if (sel.on()) { int l0, c0, l1, c1; sel.ord(l0, c0, l1, c1);
        for (int L = std::max(l0, firstLine); L <= l1 && L < (int)lines.size(); L++) { float ry = ty + (L - firstLine) * lh; if (ry > py + ph) break;
            int a = L == l0 ? c0 : 0, b = L == l1 ? c1 : (int)lines[L].size(); ui::rect(tx + a * adv, ry, std::max(adv * 0.4f, (b - a) * adv), lh, th::SEL_DIM); } }
}
inline void copySelection() {
    if (g.mainTab == MAIN_HEX && g.hexSelEnd >= 0 && g.hexSel >= 0 && setClipboard) {   // hex byte range -> copy as hex
        int lo = std::min(g.hexSel, g.hexSelEnd), hi = std::max(g.hexSel, g.hexSelEnd); string t;
        for (int i = lo; i <= hi && i < (int)g.bin.bytes.size(); i++) { char b[4]; snprintf(b, sizeof b, "%02x ", g.bin.bytes[i]); t += b; }
        if (!t.empty()) { t.pop_back(); setClipboard(t); } return; }
    if (activeDoc && activeDoc->hasSel()) { string t = activeDoc->selectedText(); if (!t.empty() && setClipboard) setClipboard(t); return; }
    if (g_lineSel && g_lineSel->on() && g_lineSelLines && setClipboard) { string t = lineSelText(*g_lineSel, *g_lineSelLines); if (!t.empty()) setClipboard(t); }
}
inline void selectAllActive() { if (activeDoc) activeDoc->selectAll(); }
inline void toggleFind() { if (!activeDoc) activeDoc = &docPseudo; activeDoc->findOn = !activeDoc->findOn; if (activeDoc->findOn) g.focus = F_DOC; else activeDoc->closeFind(); }
inline void findNext() { if (!activeDoc) return; if (!activeDoc->findOn) toggleFind(); else activeDoc->stepHit(1); }
inline void startRenameActive() { if (activeDoc && activeDoc->editable) { g.focus = F_DOC; activeDoc->startRename(); } }
inline void pasteActive() { if (g.focus == F_TERM) { if (getClipboard) term::input(getClipboard()); return; }
    if (g.focus == F_DEBUG) { if (getClipboard) term::dbg.input(getClipboard()); return; }                    // Cmd+V in the debugger -> paste into lldb
    if (activeDoc && activeDoc->editable && getClipboard) activeDoc->paste(getClipboard()); }
inline void cutActive() { if (activeDoc && activeDoc->editable && activeDoc->hasSel()) { copySelection(); activeDoc->pushUndo(); activeDoc->deleteSel(); } }
inline void undoPatch(); inline void redoPatch();   // fwd (defined with the byte-patcher)
inline void undoActive() { if (g.mainTab == MAIN_HEX || g.mainTab == MAIN_DIFF) { undoPatch(); return; } if (activeDoc && activeDoc->editable) activeDoc->undo(); }
inline void redoActive() { if (g.mainTab == MAIN_HEX || g.mainTab == MAIN_DIFF) { redoPatch(); return; } if (activeDoc && activeDoc->editable) activeDoc->redo(); }

// ── clangd LSP: live errors/warnings in the editor ───────────────────────────
// lazily start the bundled clangd, told how to compile via the bundled SDK, and open/refresh the file.
inline bool isCodeFile(const string& p) { string e = extOf(p); return e=="c"||e=="cpp"||e=="cc"||e=="cxx"||e=="h"||e=="hpp"||e=="hh"; }
inline void lspOpen(const string& path, const string& text) {
    if (!isCodeFile(path)) return;
    static bool started = false;
    if (!started) { string d = toolchainDir(); string fb = "-std=c++17" + (d.empty() ? string() : " -isysroot " + d + "/MacOSX.sdk");
        started = lsp::client.start(clangdPath(), fb); }
    lsp::client.didOpen(path, text);
}
inline void lspChange(const string& path, const string& text) { if (isCodeFile(path)) lsp::client.didChange(path, text); }

inline void openFileInViewer(const string& path) {   // explorer click -> the editable file editor (docFile)
    g.fileViewName = store::base(path); g.fileViewPath = path; g.tabOpen[MAIN_FILE] = true; g.mainTab = MAIN_FILE; g.focus = F_DOC;
    string t = store::readFile(path); g.editLines.clear(); size_t i = 0;
    while (true) { size_t e = t.find('\n', i); if (e == string::npos) { g.editLines.push_back(t.substr(i)); break; } g.editLines.push_back(t.substr(i, e - i)); i = e + 1; }
    if (g.editLines.empty()) g.editLines.push_back("");
    docFile.bind(&g.editLines, "file:" + path); docFile.editable = true; docFile.dirty = false; g.fileDirty = false; activeDoc = &docFile;
    docFile.lspPath = path; lspOpen(path, t);                                  // start live diagnostics for this file
}
inline void savePatches();   // fwd
inline void setPatchByte(size_t off, unsigned char val);   // byte-patcher fwds (defined lower)
inline void patchCommit(); inline void undoPatch(); inline void redoPatch(); inline void refreshDisasmFromPatched();
inline uint64_t anchorPseudoLine(const string& line, int fi);                         // fwd (per-line asm map — defined with the patch helpers)
inline bool resolveInstrAtVa(uint64_t va, size_t& off, int& len, string& text);       // fwd
inline void saveFile() {
    if (g.mainTab == MAIN_HEX && !g.patched.empty()) { savePatches(); return; }   // ⌘S in the hex view writes byte patches to the binary
    if (g.fileViewPath.empty() || g.editLines.empty()) return;
    string t; for (size_t i = 0; i < g.editLines.size(); i++) { t += g.editLines[i]; if (i + 1 < g.editLines.size()) t += "\n"; }
    store::writeFile(g.fileViewPath, t); docFile.dirty = false; g.fileDirty = false; g.log("saved " + g.fileViewName);
    lspChange(g.fileViewPath, t);                                              // re-check errors/warnings after a save
}

// generic: a scrollable header+list panel. onRow draws one row; returns rows drawn region top.
inline float panelHeader(float x, float y, float w, ui::Icon ic, const string& title, const string& count) {
    float h = ui::lineH() + 8 * ui::in.scale;
    ui::rect(x, y, w, h, th::PANEL_HI); ui::hline(x, y + h - ui::in.scale, w, th::BORDER);
    float s = ui::in.scale;
    ui::icon(x + 8 * s, y + (h - 13 * s) / 2, 13 * s, ic, th::TEXT_DIM);
    ui::text(x + 26 * s, y + (h - ui::lineH()) / 2, title, th::TEXT);
    if (!count.empty()) ui::textRight(x + w - 8 * s, y + (h - ui::lineH()) / 2, count, th::TEXT_MUT);
    return h;
}

// ── filter box ───────────────────────────────────────────────────────────────
inline void filterBox(float x, float y, float w) {
    float s = ui::in.scale, h = ui::lineH() + 6 * s; bool foc = g.focus == F_FILTER;
    // consume typed input BEFORE drawing, so the box shows this frame's keystroke (no one-frame lag)
    if (foc && !g.paletteOpen && ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) { if (!g.filter.empty()) g.filter.pop_back(); } else if (ui::in.ch >= 32 && ui::in.ch < 127) g.filter += (char)ui::in.ch; }
    if (foc && ui::in.key == ui::K_ESC) { g.filter.clear(); g.focus = F_NONE; }
    ui::rect(x, y, w, h, th::SUNKEN); ui::rectLine(x, y, w, h, foc ? th::ACCENT : th::BORDER);
    ui::icon(x + 5 * s, y + (h - 12 * s) / 2, 12 * s, ui::IC_SEARCH, th::TEXT_MUT);
    string shown = g.filter.empty() ? "filter" : g.filter;
    ui::text(x + 22 * s, y + (h - ui::lineH()) / 2, shown, (g.filter.empty() && !foc) ? th::TEXT_MUT : th::TEXT);
    if (ui::clicked(x, y, w, h)) g.focus = F_FILTER;
}

// match the active filter
inline bool match(const string& s) { if (g.filter.empty()) return true; auto low = [](string t){ for (auto& c : t) c = tolower(c); return t; }; return low(s).find(low(g.filter)) != string::npos; }

// ── side panels ──────────────────────────────────────────────────────────────
inline void panelFunctions(float x, float y, float w, float h) {
    float s = ui::in.scale, rh = ui::lineH() + th::ROW_PAD * s;
    ui::pushClip(x, y, w, h);
    vector<int> idx; for (int i = 0; i < (int)g.bin.funcs.size(); i++) if (match(g.bin.funcs[i].name)) idx.push_back(i);
    int rows = (int)(h / rh);
    if (ui::hovered(x, y, w, h)) g.sideScroll += (int)ui::in.wheel;   // wheel FIRST, THEN clamp — clamping first let a fast up-scroll drive sideScroll negative -> idx[li] read out of bounds -> crash
    if (g.sideScroll > (int)idx.size() - rows) g.sideScroll = std::max(0, (int)idx.size() - rows); if (g.sideScroll < 0) g.sideScroll = 0;
    for (int r = 0; r < rows; r++) { int li = g.sideScroll + r; if (li < 0 || li >= (int)idx.size()) break; int fi = idx[li]; auto& f = g.bin.funcs[fi];
        float ry = y + r * rh; bool sel = fi == g.selFn, hov = ui::hovered(x, ry, w, rh);
        if (sel) { ui::rect(x, ry, w, rh, th::SEL); ui::rect(x, ry, 2.5f * s, rh, th::ACCENT); } else if (hov) ui::rect(x, ry, w, rh, th::HOVER);
        Col chip = f.kind == 1 ? th::CHIP_CLASS : f.kind == 2 ? th::CHIP_STRUCT : f.kind == 3 ? th::CHIP_DATA : th::CHIP_FN;
        ui::rect(x + 8 * s, ry + (rh - 8 * s) / 2, 8 * s, 8 * s, chip);
        ui::textClip(x + 22 * s, ry + (rh - ui::lineH()) / 2, w - 28 * s, f.name, sel ? th::TEXT : th::Col{0.72f,0.74f,0.79f});
        if (ui::clicked(x, ry, w, rh)) { g.selFn = fi; g.codeScroll = 0; g.selA = g.selB = -1; g.pseudoJump = true; }
        if (ui::rclicked(x, ry, w, rh)) { g.selFn = fi; g.ctxTarget = fi; ui::openMenu(ui::in.mx, ui::in.my, { {"Go to definition", ACT_GOTO}, {"Copy name", ACT_COPY_NAME}, {"Show xrefs", ACT_XREF}, {"", 0, true}, {"Export decomp/", ACT_EXPORT} }); }
    }
    ui::popClip();
    ui::scrollbar(x + w - 7 * s, y, 7 * s, h, (int)idx.size(), rows, &g.sideScroll);
}
inline void panelSymbols(float x, float y, float w, float h) {
    float s = ui::in.scale, rh = ui::lineH() + th::ROW_PAD * s; ui::pushClip(x, y, w, h);
    vector<int> idx; for (int i = 0; i < (int)g.bin.syms.size(); i++) if (match(g.bin.syms[i].name)) idx.push_back(i);
    int rows = (int)(h / rh);
    if (ui::hovered(x, y, w, h)) g.sideScroll += (int)ui::in.wheel;   // wheel FIRST, THEN clamp — clamping first let a fast up-scroll drive sideScroll negative -> idx[li] read out of bounds -> crash
    if (g.sideScroll > (int)idx.size() - rows) g.sideScroll = std::max(0, (int)idx.size() - rows); if (g.sideScroll < 0) g.sideScroll = 0;
    for (int r = 0; r < rows; r++) { int li = g.sideScroll + r; if (li < 0 || li >= (int)idx.size()) break; auto& sym = g.bin.syms[idx[li]];
        float ry = y + r * rh; bool hov = ui::hovered(x, ry, w, rh); if (hov) ui::rect(x, ry, w, rh, th::HOVER);
        char ad[20]; snprintf(ad, sizeof ad, "%llx", (unsigned long long)sym.addr); ui::text(x + 8 * s, ry + (rh - ui::lineH()) / 2, ad, th::NUM);
        ui::textClip(x + 8 * s + 11 * ui::advance(), ry + (rh - ui::lineH()) / 2, w - 8 * s - 11 * ui::advance(), sym.name, th::TEXT);
        if (ui::clicked(x, ry, w, rh)) { for (int k = 0; k < (int)g.bin.funcs.size(); k++) if (g.bin.funcs[k].name == sym.name) { g.selFn = k; g.codeScroll = 0; g.pseudoJump = true; break; } }
    }
    ui::popClip(); ui::scrollbar(x + w - 7 * s, y, 7 * s, h, (int)idx.size(), rows, &g.sideScroll);
}
inline void panelStrings(float x, float y, float w, float h) {
    float s = ui::in.scale, rh = ui::lineH() + th::ROW_PAD * s; ui::pushClip(x, y, w, h);
    vector<int> idx; for (int i = 0; i < (int)g.bin.strings.size(); i++) if (match(g.bin.strings[i].text)) idx.push_back(i);
    int rows = (int)(h / rh);
    if (ui::hovered(x, y, w, h)) g.sideScroll += (int)ui::in.wheel;   // wheel FIRST, THEN clamp — clamping first let a fast up-scroll drive sideScroll negative -> idx[li] read out of bounds -> crash
    if (g.sideScroll > (int)idx.size() - rows) g.sideScroll = std::max(0, (int)idx.size() - rows); if (g.sideScroll < 0) g.sideScroll = 0;
    for (int r = 0; r < rows; r++) { int li = g.sideScroll + r; if (li < 0 || li >= (int)idx.size()) break; auto& st = g.bin.strings[idx[li]];
        float ry = y + r * rh; bool hov = ui::hovered(x, ry, w, rh); if (hov) ui::rect(x, ry, w, rh, th::HOVER);
        ui::textClip(x + 8 * s, ry + (rh - ui::lineH()) / 2, w - 14 * s, "\"" + st.text + "\"", th::STR);
        if (ui::rclicked(x, ry, w, rh)) { g.ctxText = st.text; ui::openMenu(ui::in.mx, ui::in.my, { {"Copy string", ACT_COPY} }); }
    }
    ui::popClip(); ui::scrollbar(x + w - 7 * s, y, 7 * s, h, (int)idx.size(), rows, &g.sideScroll);
}

// ── main views ───────────────────────────────────────────────────────────────
// the pseudocode right-click menu. Anchors the clicked LINE to its instruction (per-line NOP / assemble-patch,
// reusing the disasm patch flow via g.asmOff/asmLen/asmText) and offers "Show in Disassembly" + whole-function patch.
inline void openPseudoMenu(float cy, int fi, bool single) {
    float lh = ui::lineH() * 1.05f; int li = docPseudo.scroll + (int)((ui::in.my - cy) / lh);
    const vector<string>* buf = docPseudo.lines;
    string lineText = (buf && li >= 0 && li < (int)buf->size()) ? (*buf)[li] : "";
    g.asmOff = SIZE_MAX; g.asmLen = 0; g.asmText.clear();
    uint64_t va = anchorPseudoLine(lineText, fi);                  // map this C line -> the instruction it came from
    if (va != UINT64_MAX) { size_t off; int len; string was; if (resolveInstrAtVa(va, off, len, was) && len > 0) { g.asmOff = off; g.asmLen = len; g.asmText = was; } }
    vector<ui::MenuItem> m = { {"Rename symbol at caret (F2)", ACT_RENAME}, {"Copy selection", ACT_COPY}, {"Select all", ACT_SELALL} };
    if (single) m.push_back({ "Copy function", ACT_COPY_FUNC });
    if (g.asmOff != SIZE_MAX) { m.push_back({ "", 0, true });       // we pinned this line to a real instruction
        m.push_back({ "NOP this line's instruction   (" + g.asmText + ")", ACT_NOP });
        m.push_back({ "Patch this line's instruction\xe2\x80\xa6 (assemble)", ACT_ASMPATCH }); }
    if (fi >= 0 && fi < (int)g.bin.funcs.size() && g.bin.funcs[fi].addr) { m.push_back({ "", 0, true }); m.push_back({ "Show this function in Disassembly", ACT_TODISASM }); }
    m.push_back({ "", 0, true });
    m.push_back({ "Patch this function from pseudocode", ACT_PATCHFN });
    m.push_back({ "Export decomp/", ACT_EXPORT });
    ui::openMenu(ui::in.mx, ui::in.my, m);
}
inline void panelPseudo(float x, float y, float w, float h) {   // editable, selectable, scrollable decompiled output
    float s = ui::in.scale;
    if (g.bin.funcs.empty()) { ui::text(x + 16 * s, y + 16 * s, "no functions", th::TEXT_MUT); return; }
    float lh = ui::lineH() * 1.05f, sigH = lh + 8 * s;            // header / signature bar
    ui::rect(x, y, w, sigH, th::PANEL_HI); ui::hline(x, y + sigH - s, w, th::BORDER);
    // whole-program  <->  single-function toggle (top-right)
    float tgw = std::max(ui::textW("whole program"), ui::textW("function")) + 22 * s;
    if (ui::button(x + w - tgw - 8 * s, y + 4 * s, tgw, sigH - 8 * s, g.pseudoWhole ? "whole program" : "function")) { g.pseudoWhole = !g.pseudoWhole; g.wholeVerBuilt = -1; }
    float cy = y + sigH, ch = h - sigH;
    if (g.pseudoWhole) {                                          // DEFAULT: the entire program, one scroll, jump via the sidebar
        if (g.wholeVerBuilt != g.wholeVer) rebuildWhole();
        ui::text(x + 10 * s, y + (sigH - ui::lineH()) / 2, std::to_string((int)g.bin.funcs.size()) + " functions  ·  whole program", th::FNC);
        docPseudo.bind(&g.wholeLines, "whole:" + g.bin.name + "#" + std::to_string(g.wholeVer)); docPseudo.editable = true; docPseudo.syntax = true; docPseudo.propagateGlobal = true;
        if (g.pseudoJump) { g.pseudoJump = false; int fi = g.selFn;   // a sidebar click -> scroll to that function
            if (fi >= 0 && fi < (int)g.wholeFnLine.size()) { docPseudo.scroll = g.wholeFnLine[fi]; docPseudo.caretL = g.wholeFnLine[fi]; docPseudo.caretC = 0; docPseudo.clearSel(); } }
        docPseudo.run(x, cy, w, ch, activeDoc == &docPseudo);
        { int fi = 0; for (size_t i = 0; i < g.wholeFnLine.size(); i++) { if (g.wholeFnLine[i] <= docPseudo.caretL) fi = (int)i; else break; } g.selFn = fi; }   // sidebar + xrefs follow the caret's function; F2 propagation gets the right current fn
        if (ui::rclicked(x, cy, w, ch)) { activeDoc = &docPseudo; g.focus = F_DOC; g.ctxTarget = g.selFn; openPseudoMenu(cy, g.selFn, false); }
        return;
    }
    auto& f = g.bin.funcs[std::min(g.selFn, (int)g.bin.funcs.size() - 1)];
    const char* tag = f.kind == 1 ? "class" : f.kind == 2 ? "struct" : f.kind == 3 ? "" : "function";
    float tx = x + 10 * s; if (*tag) tx = ui::text(tx, y + (sigH - ui::lineH()) / 2, string(tag) + " ", th::TEXT_DIM);
    ui::text(tx, y + (sigH - ui::lineH()) / 2, f.sig, f.kind == 1 ? th::TYPE : f.kind == 2 ? th::ACCENT2 : th::FNC);
    docPseudo.bind(&f.lines, "fn:" + g.bin.name + "#" + std::to_string(g.selFn)); docPseudo.editable = true; docPseudo.syntax = true; docPseudo.propagateGlobal = true;   // renaming a function/type here propagates across the program
    docPseudo.run(x, cy, w, ch, activeDoc == &docPseudo);
    if (ui::rclicked(x, cy, w, ch)) { activeDoc = &docPseudo; g.focus = F_DOC; g.ctxTarget = g.selFn; openPseudoMenu(cy, g.selFn, true); }
}
// parse the leading `<hex>:` virtual address of a disasm line (UINT64_MAX if the line isn't an instruction)
inline uint64_t parseAsmAddr(const string& l) {
    size_t i = 0; while (i < l.size() && (l[i] == ' ' || l[i] == '\t')) i++;
    size_t st = i; while (i < l.size() && isxdigit((unsigned char)l[i])) i++;
    if (i > st && i < l.size() && l[i] == ':') return strtoull(l.substr(st, i - st).c_str(), nullptr, 16);
    return UINT64_MAX;
}
// map disasm line `li` -> its byte slot (sets g.asmOff/asmLen/asmText) for instruction-level patching.
// asmOff stays SIZE_MAX if the line has no address; asmLen stays 0 if the x86 length can't be derived.
inline void setAsmAt(const vector<string>& disLines, int li) {
    g.asmOff = SIZE_MAX; g.asmLen = 0; g.asmText.clear();
    if (li < 0 || li >= (int)disLines.size()) return;
    uint64_t a = parseAsmAddr(disLines[li]); if (a == UINT64_MAX) return;
    size_t off = g.bin.addrToOffset(a); if (off == SIZE_MAX || off >= g.bin.bytes.size()) return;
    int len = (g.bin.arch == "arm64") ? 4 : 0;   // x86: derive from the next instruction's address (never guess)
    if (g.bin.arch != "arm64") for (int n = li + 1; n < (int)disLines.size(); n++) { uint64_t na = parseAsmAddr(disLines[n]); if (na != UINT64_MAX) { if (na > a && na - a <= 15) len = (int)(na - a); break; } }
    if (len > 0 && off + (size_t)len > g.bin.bytes.size()) len = 0;
    g.asmOff = off; g.asmLen = len;
    size_t c = disLines[li].find(':'); g.asmText = (c != string::npos) ? model::trim(disLines[li].substr(c + 1)) : model::trim(disLines[li]);
}
// open the inline "edit this instruction" assembler for disasm line `li` (Enter / double-click / menu).
inline void editAsmLine(const vector<string>& disLines, int li) {
    setAsmAt(disLines, li);
    if (g.asmOff != SIZE_MAX && g.asmLen > 0) { g.asmPatchPrompt = true; asmPatchJustOpened = true; g.asmPatchText = g.asmText; g.asmPatchErr.clear(); g.focus = F_NONE; }
}
inline void panelDisasm(float x, float y, float w, float h) {   // selectable + scrollable disassembly; asm<->bytes patching
    g.bin.runDisasm();
    static string lastDis; static vector<string> disLines;
    if (g.bin.disasm.size() != lastDis.size() || g.bin.disasm != lastDis) {   // re-split only when the listing changes
        lastDis = g.bin.disasm; disLines.clear(); size_t i = 0; const string& d = g.bin.disasm;
        while (i < d.size()) { size_t e = d.find('\n', i); disLines.push_back(d.substr(i, e == string::npos ? string::npos : e - i)); if (e == string::npos) break; i = e + 1; } }
    docDisasm.bind(&disLines, "dis:" + g.bin.name); docDisasm.editable = false; docDisasm.syntax = false;
    if (g.disasmGotoAddr) { for (int li = 0; li < (int)disLines.size(); li++) if (parseAsmAddr(disLines[li]) == g.disasmGotoAddr) { docDisasm.scroll = std::max(0, li - 2); docDisasm.caretL = li; docDisasm.caretC = 0; break; } g.disasmGotoAddr = 0; }   // a pseudo-line "Show in Disassembly" jump
    docDisasm.run(x, y, w, h, activeDoc == &docDisasm, false);
    // EDITABLE DISASSEMBLY: Enter on the caret line opens its instruction for editing -> assemble + patch the bytes (no recompile)
    if (activeDoc == &docDisasm && ui::in.key == ui::K_ENTER && !g.asmPatchPrompt) editAsmLine(disLines, docDisasm.caretL);
    if (ui::rclicked(x, y, w, h)) { activeDoc = &docDisasm; g.focus = F_DOC;
        float lh = ui::lineH() * 1.05f; int li = docDisasm.scroll + (int)((ui::in.my - y) / lh); setAsmAt(disLines, li);   // map this instruction -> its file bytes
        vector<ui::MenuItem> items = { {"Copy selection", ACT_COPY}, {"Select all", ACT_SELALL} };
        if (g.asmOff != SIZE_MAX) { items.push_back({"", 0, true}); items.push_back({"Show bytes in Hex editor", ACT_TOBYTES});
            if (g.asmLen > 0) { items.push_back({"Edit instruction\xe2\x80\xa6 (assemble + patch)", ACT_ASMPATCH}); items.push_back({"NOP out this instruction", ACT_NOP}); } }
        ui::openMenu(ui::in.mx, ui::in.my, items); }
}
inline void panelHex(float x, float y, float w, float h) {   // editable: click a byte, type hex -> patch; ⌘S writes the binary (with backup)
    float s = ui::in.scale, lh = ui::lineH(), adv = ui::advance();
    float hx0 = x + 8 * s + 9 * adv; auto bx = [&](int i) { return hx0 + i * 3 * adv + (i >= 8 ? adv : 0); };
    float ax0 = hx0 + 50 * adv; auto ax = [&](int i) { return ax0 + i * adv; };   // ASCII column (16 chars, after the hex + gap)
    float hintH = lh + 4 * s;
    ui::pushClip(x, y, w, h - hintH); int rows = (int)((h - hintH) / lh), total = (int)((g.bin.bytes.size() + 15) / 16);
    if (ui::hovered(x, y, w, h)) g.hexScroll += (int)ui::in.wheel; if (g.hexScroll > total - 1) g.hexScroll = std::max(0, total - 1); if (g.hexScroll < 0) g.hexScroll = 0;
    auto hitByte = [&]() -> int { int r = (int)((ui::in.my - y) / lh); if (r < 0 || r >= rows) return -1; size_t off = (size_t)(g.hexScroll + r) * 16;
        for (int i = 0; i < 16; i++) { if (off + i >= g.bin.bytes.size()) break;
            bool hh = ui::in.mx >= bx(i) - adv * 0.3f && ui::in.mx < bx(i) + 2.3f * adv;
            bool ah = ui::in.mx >= ax(i) - adv * 0.2f && ui::in.mx < ax(i) + adv * 0.9f;
            if (hh || ah) { g.hexAscii = ah; return (int)(off + i); } } return -1; };
    if (!ui::menuActive && ui::inside(x, y, w - 8 * s, h - hintH) && ui::in.lPress) { int hb = hitByte();   // click a byte in EITHER column
        if (hb >= 0) { patchCommit();
            if (ui::in.shift && g.hexSel >= 0) g.hexSelEnd = hb;                       // shift-click -> extend a byte range from the anchor
            else { g.hexSel = hb; g.hexSelEnd = -1; g.hexNibble = 0; }                 // plain click -> a single editable byte
            g.focus = F_NONE; g.dbgBpFocus = false; } }
    else if (ui::in.lDown && !ui::menuActive && ui::inside(x, y, w - 8 * s, h - hintH)) { int hb = hitByte(); if (hb >= 0 && g.hexSel >= 0 && hb != g.hexSel) g.hexSelEnd = hb; }   // drag -> range
    int rlo = -1, rhi = -1; if (g.hexSelEnd >= 0 && g.hexSel >= 0) { rlo = std::min(g.hexSel, g.hexSelEnd); rhi = std::max(g.hexSel, g.hexSelEnd); }
    for (int r = 0; r < rows; r++) { int row = g.hexScroll + r; if ((size_t)row * 16 >= g.bin.bytes.size()) break; float ry = y + r * lh; size_t off = (size_t)row * 16;   // pass 1: patched/selected byte rects
        for (int i = 0; i < 16; i++) { size_t o = off + i; if (o >= g.bin.bytes.size()) break;
            if (rlo >= 0 && (int)o >= rlo && (int)o <= rhi) { ui::rect(bx(i) - 1 * s, ry, 2 * adv + 2 * s, lh, th::SEL_DIM); ui::rect(ax(i) - 0.5f * s, ry, adv + 1 * s, lh, th::SEL_DIM); }   // drag/shift range
            if (g.patched.count(o)) { ui::rect(bx(i) - 1 * s, ry, 2 * adv + 2 * s, lh, th::Col{0.55f,0.28f,0.10f,0.55f}); ui::rect(ax(i) - 0.5f * s, ry, adv + 1 * s, lh, th::Col{0.55f,0.28f,0.10f,0.40f}); }
            if ((int)o == g.hexSel) { th::Col sel = g.hexAscii ? th::Col{0.4f,0.7f,1.0f,0.5f} : th::Col{1.0f,0.6f,0.2f,0.6f};   // caret column tinted by edit mode
                ui::rect(bx(i) - 1 * s, ry, 2 * adv + 2 * s, lh, g.hexAscii ? th::Col{1.0f,0.6f,0.2f,0.25f} : sel);
                ui::rect(ax(i) - 0.5f * s, ry, adv + 1 * s, lh, g.hexAscii ? sel : th::Col{1.0f,0.6f,0.2f,0.25f}); } } }
    ui::beginText();
    for (int r = 0; r < rows; r++) { int row = g.hexScroll + r; if ((size_t)row * 16 >= g.bin.bytes.size()) break; float ry = y + r * lh; size_t off = (size_t)row * 16;
        char addr[16]; snprintf(addr, sizeof addr, "%08zx", off); float cx = ui::emit(x + 8 * s, ry, addr, th::TEXT_MUT) + adv;
        string hex, asc; for (int i = 0; i < 16; i++) { if (off + i < g.bin.bytes.size()) { unsigned char b = g.bin.bytes[off + i]; char hb[4]; snprintf(hb, sizeof hb, "%02x ", b); hex += hb; asc += (b >= 32 && b < 127) ? (char)b : '.'; } else hex += "   "; if (i == 7) hex += " "; }
        cx = ui::emit(cx, ry, hex, th::NUM); ui::emit(cx + adv, ry, asc, th::Col{0.6f,0.62f,0.66f}); }
    ui::endText();
    ui::popClip(); ui::scrollbar(x + w - 7 * s, y, 7 * s, h - hintH, total, rows, &g.hexScroll);
    ui::rect(x, y + h - hintH, w, hintH, th::PANEL_HI);
    string hint = g.hexSel < 0 ? string("click a byte to edit  ·  ") + (g.hexAscii ? "ASCII" : "HEX") + " mode  ·  Tab switches column"
        : string(g.hexAscii ? "ASCII edit: type a character" : "HEX edit: type 0-9 a-f") + "  ·  Tab: " + (g.hexAscii ? "hex" : "ASCII") + "  ·  \xe2\x8c\x98Z undo";
    if (!g.patched.empty()) hint = "patched " + std::to_string((int)g.patched.size()) + " byte(s)  ·  " + (g.hexAscii ? "ASCII" : "HEX") + " (Tab)  ·  \xe2\x8c\x98Z undo  ·  \xe2\x8c\x98S write (first save keeps .bak)";
    ui::text(x + 8 * s, y + h - hintH + 2 * s, hint, g.patched.empty() ? th::TEXT_MUT : th::ACCENT2);
}
// ── side-by-side ORIGINAL vs PATCHED diff (every byte you changed this session) ──
inline void panelDiff(float x, float y, float w, float h) {
    float s = ui::in.scale, lh = ui::lineH() * 1.15f, adv = ui::advance();
    ui::rect(x, y, w, h, th::PANEL);
    if (g.patched.empty()) { ui::text(x + 16 * s, y + 16 * s, "No patches yet — right-click an instruction to Patch / NOP it, or edit bytes in the Hex view.", th::TEXT_MUT); return; }
    vector<std::pair<size_t,size_t>> runs; { size_t prev = SIZE_MAX, start = 0;     // contiguous changed-byte runs
        for (size_t o : g.patched) { if (prev == SIZE_MAX || o != prev + 1) { if (prev != SIZE_MAX) runs.push_back({ start, prev + 1 }); start = o; } prev = o; }
        if (prev != SIZE_MAX) runs.push_back({ start, prev + 1 }); }
    const int perRow = 8;
    int totalRows = 0; for (auto& r : runs) totalRows += 1 + (int)((r.second - r.first + perRow - 1) / perRow);
    float colX = x + 12 * s, midX = x + w * 0.52f;
    ui::rect(x, y, w, lh + 6 * s, th::PANEL_HI); ui::hline(x, y + lh + 6 * s, w, th::BORDER);
    ui::text(colX, y + 4 * s, "ORIGINAL", th::RED); ui::text(midX + 14 * s, y + 4 * s, "PATCHED", th::GREEN);
    char cnt[64]; snprintf(cnt, sizeof cnt, "%d region(s) · %d byte(s)", (int)runs.size(), (int)g.patched.size());
    ui::textRight(x + w - 10 * s, y + 4 * s, cnt, th::TEXT_MUT);
    float by = y + lh + 8 * s, bh = h - (lh + 8 * s);
    if (ui::hovered(x, by, w, bh)) g.diffScroll -= (int)ui::in.wheel;
    int viewRows = (int)(bh / lh); if (g.diffScroll > totalRows - viewRows) g.diffScroll = std::max(0, totalRows - viewRows); if (g.diffScroll < 0) g.diffScroll = 0;
    ui::pushClip(x, by, w, bh); ui::vline(midX, by, bh, th::BORDER);
    auto inText = [&](size_t off) { return g.bin.textSize && off >= g.bin.textOff && off < g.bin.textOff + g.bin.textSize; };
    int row = 0; auto rowY = [&](int r) { return by + (r - g.diffScroll) * lh; };
    for (auto& rn : runs) { size_t a = rn.first, b = rn.second; float cy = rowY(row);
        if (cy + lh > by && cy < by + bh) { char hd[112]; uint64_t va = inText(a) ? (g.bin.textVaddr + (a - g.bin.textOff)) : 0;
            if (va) snprintf(hd, sizeof hd, "+0x%zx    0x%llx    (%d byte%s)", a, (unsigned long long)va, (int)(b - a), b - a == 1 ? "" : "s");
            else snprintf(hd, sizeof hd, "+0x%zx    (%d byte%s)", a, (int)(b - a), b - a == 1 ? "" : "s");
            ui::text(colX, cy, hd, th::ACCENT2); }
        row++;
        for (size_t o = a; o < b; o += perRow) { size_t e = std::min(o + (size_t)perRow, b); cy = rowY(row);
            if (cy + lh > by && cy < by + bh) { ui::beginText(); float ox = colX + 6 * s, px = midX + 14 * s;
                for (size_t k = o; k < e; k++) { char hb[4];
                    snprintf(hb, sizeof hb, "%02x", g.patchOrig.count(k) ? g.patchOrig[k] : g.bin.bytes[k]); ui::emit(ox, cy, hb, th::RED); ox += 3 * adv;
                    snprintf(hb, sizeof hb, "%02x", g.bin.bytes[k]); ui::emit(px, cy, hb, th::GREEN); px += 3 * adv; }
                ui::endText(); ui::text(midX - 3 * adv, cy, "->", th::TEXT_MUT); }
            row++; }
    }
    ui::popClip();
}
inline void handleHexInput() {
    if (g.mainTab != MAIN_HEX || g.hexSel < 0 || g.paletteOpen || g.savePrompt || g.asmPatchPrompt || g.aiPatchPrompt || g.dbgBpFocus || g.focus == F_DOC || g.focus == F_TERM || g.focus == F_FILTER || g.focus == F_AI || g.focus == F_DEBUG) return;
    if (g.hexSel >= (int)g.bin.bytes.size()) { g.hexSel = -1; return; }
    int k = ui::in.key;
    if (k == ui::K_LEFT || k == ui::K_RIGHT || k == ui::K_UP || k == ui::K_DOWN) g.hexSelEnd = -1;   // arrow nav drops the copy range
    if (k == ui::K_TAB) { patchCommit(); g.hexAscii = !g.hexAscii; g.hexNibble = 0; return; }   // Tab toggles hex<->ASCII column
    if (k == ui::K_LEFT) { patchCommit(); if (g.hexSel > 0) g.hexSel--; g.hexNibble = 0; }
    else if (k == ui::K_RIGHT) { patchCommit(); if (g.hexSel + 1 < (int)g.bin.bytes.size()) g.hexSel++; g.hexNibble = 0; }
    else if (k == ui::K_ESC) { patchCommit(); g.hexSel = -1; return; }
    unsigned c = ui::in.ch;
    if (g.hexAscii) {                                                   // ASCII column: a printable char patches the byte to that character
        if (c >= 32 && c < 127) { g.patchDirty = true; setPatchByte((size_t)g.hexSel, (unsigned char)c); patchCommit();
            if (g.hexSel + 1 < (int)g.bin.bytes.size()) g.hexSel++; refreshDisasmFromPatched(); }
        else if (c == 8 || c == 127) { if (g.hexSel > 0) g.hexSel--; patchCommit(); undoPatch(); }   // backspace = step back + revert
        return;
    }
    int v = c >= '0' && c <= '9' ? (int)(c - '0') : c >= 'a' && c <= 'f' ? (int)(c - 'a' + 10) : c >= 'A' && c <= 'F' ? (int)(c - 'A' + 10) : -1;
    if (v >= 0) { g.patchDirty = true; unsigned char cur = g.bin.bytes[g.hexSel];   // track from the FIRST nibble — a half-typed byte is still a real edit
        unsigned char nv = g.hexNibble == 0 ? (unsigned char)((cur & 0x0f) | (v << 4)) : (unsigned char)((cur & 0xf0) | v);
        setPatchByte((size_t)g.hexSel, nv);
        if (g.hexNibble == 0) g.hexNibble = 1;
        else { g.hexNibble = 0; patchCommit(); if (g.hexSel + 1 < (int)g.bin.bytes.size()) g.hexSel++; refreshDisasmFromPatched(); } }
}
// ── EmberRun: execute the binary (native directly, foreign via NXRT) ──────────
inline void runEmberRun() {
    if (!g.bin.loaded) return;
    g.showBottom = true; g.bottomTab = BOTTOM_TERM; if (!term::started || !term::alive) term::start(); g.focus = F_TERM;
    bool macho = g.bin.bytes.size() >= 4 && g.bin.bytes[0] == 0xCF && g.bin.bytes[1] == 0xFA && g.bin.bytes[2] == 0xED && g.bin.bytes[3] == 0xFE;
    string bundledNx = g.dir + "nxrt";                              // shipped inside the .app (Contents/MacOS) — self-contained
    string nxrt = store::exists(bundledNx) ? bundledNx
                : store::exists("/usr/local/bin/nxrt") ? "/usr/local/bin/nxrt"
                : (string(getenv("HOME") ? getenv("HOME") : "") + "/.local/bin/nxrt");
    string cmd = macho ? model::shq(g.bin.path) : (model::shq(nxrt) + " " + model::shq(g.bin.path));   // native Mach-O runs directly; PE/ELF via NXRT
    term::input(cmd + "\r");
    g.log(string("EmberRun: running ") + g.bin.name + (macho ? "" : " via NXRT") + " (see Terminal)");
}
// ── debugger: its own dock tab, driving a DEDICATED lldb PTY (term::dbg) ───────
// (re)launch lldb on the current binary in the dedicated debugger PTY — NEVER a bare
// shell. Every restart path funnels through here; a no-arg term::dbg.start() would
// exec $SHELL and silently turn the Debugger into a plain terminal.
// Find a usable lldb. The .app launched from Finder doesn't inherit a dev-tool PATH,
// so prefer an app-bundled copy, then the user's Xcode in ~/Applications, then xcrun,
// then the /usr/bin shim. Cached after first resolve.
inline string resolveLldb() {
    static string cached;
    if (!cached.empty()) return cached;
    if (store::exists(g.dir + "lldb")) return cached = g.dir + "lldb";          // bundled next to the GUI
    const char* home = getenv("HOME");
    if (home) for (const char* xc : { "/Applications/Xcode-beta.app", "/Applications/Xcode.app" }) {
        string p = string(home) + xc + "/Contents/Developer/usr/bin/lldb";
        if (store::exists(p)) return cached = p;
    }
    if (FILE* f = popen("xcrun -f lldb 2>/dev/null", "r")) {                     // respects xcode-select
        char buf[1024]; string out; size_t n; while ((n = fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n); pclose(f);
        while (!out.empty() && (out.back() == '\n' || out.back() == '\r' || out.back() == ' ')) out.pop_back();
        if (!out.empty() && store::exists(out)) return cached = out;
    }
    return cached = "lldb";                                                      // last resort: PATH / /usr/bin shim
}
inline void dbgLaunch() { if (g.bin.loaded) term::dbg.start((resolveLldb() + " " + model::shq(g.bin.path)).c_str()); }
inline void runDebug() {
    if (!g.bin.loaded) return;
    g.showBottom = true; g.bottomTab = BOTTOM_DEBUG; g.focus = F_DEBUG; g.dbgScroll = 0;
    if (!term::dbg.started || !term::dbg.alive) dbgLaunch();                       // launch lldb directly in its own PTY
    else term::dbg.input("file " + model::shq(g.bin.path) + "\r");                 // already running -> retarget
    g.log("debug: lldb on " + g.bin.name + " — use the Debugger tab (Run / Step / breakpoints)");
}
inline void dbgCmd(const string& c) { if (!term::dbg.started || !term::dbg.alive) dbgLaunch(); g.bottomTab = BOTTOM_DEBUG; g.focus = F_DEBUG; g.dbgScroll = 0; term::dbg.input(c + "\r"); }
// write the patched bytes to `path`. Overwriting the original keeps a one-time .bak.
inline void writePatched(const string& path, bool overwrite) {
    if (g.bin.bytes.empty()) return;
    bool wroteBak = false; if (overwrite) { string bak = path + ".bak", orig = store::readFile(path); if (!store::exists(bak) && !orig.empty()) { store::writeFile(bak, orig); wroteBak = true; } }   // never write an empty .bak (original missing)
    store::writeFile(path, string((const char*)g.bin.bytes.data(), g.bin.bytes.size()));
    g.log("patched " + std::to_string((int)g.patched.size()) + " byte(s) -> " + store::base(path) +
          (overwrite ? (wroteBak ? "   (original backed up to .bak)" : "   (kept existing .bak)") : "   (saved as new binary)"));
    g.patchDirty = false;
}
inline void savePatches() {   // ⌘S in hex: ask whether to overwrite the original or save a new binary
    if (g.patched.empty() || !g.bin.loaded) { g.log("no byte patches to save"); return; }
    g.savePrompt = true;
}
// ── instruction-level patching (assemble a replacement, NOP, live re-disasm) ───
inline bool isX86() { return g.bin.arch == "x86-64" || g.bin.arch == "x86"; }
// after any byte patch, re-disassemble the PATCHED bytes (a temp) so the asm view is live, not stale-from-disk
inline void refreshDisasmFromPatched() {
    if (!g.bin.loaded) return;
    string tmp = store::root() + "/.patched.bin"; store::mkdirs(store::root());
    store::writeFile(tmp, string((const char*)g.bin.bytes.data(), g.bin.bytes.size()));
    g.bin.disasmSource = tmp; g.bin.disasm.clear();   // lazy: next time the Disasm tab draws, it re-runs on the patched temp
}
// assemble ONE instruction to its machine bytes via clang, extracted from the assembled .o's __text section
inline vector<unsigned char> assembleInstr(const string& instr, const string& arch) {
    string triple = arch == "arm64" ? "arm64-apple-macos" : arch == "x86-64" ? "x86_64-apple-macos" : "";   // only 64-bit (the only arches the .text reader maps)
    if (triple.empty() || model::trim(instr).empty()) return {};
    string tmpo = store::root() + "/.ember_asm.o"; store::mkdirs(store::root());
    string cmd = "printf '%s\\n' " + model::shq(instr) + " | clang -x assembler -c -target " + triple + " - -o " + model::shq(tmpo) + " 2>/dev/null";
    if (system(cmd.c_str()) != 0) return {};
    vector<uint8_t> o; bool ok = nx::readFile(tmpo.c_str(), o); remove(tmpo.c_str());
    if (!ok) return {};
    size_t off = 0, sz = 0; uint64_t va = 0; if (!nx::machoText(o.data(), o.size(), off, sz, va) || off + sz > o.size() || sz == 0) return {};
    return vector<unsigned char>(o.begin() + off, o.begin() + off + sz);
}
// ── byte-patch undo/redo ──────────────────────────────────────────────────────
// Each logical edit (a typed byte, an assembled instruction, a NOP) accumulates into
// `patchPending`; patchCommit() seals it as one undo step. undo/redo restore the bytes
// and recompute which offsets are still "patched" (differ from the original).
inline void patchRecord(size_t off) { if (off >= g.bin.bytes.size()) return; for (auto& p : g.patchPending) if (p.first == off) return; g.patchPending.push_back({ off, g.bin.bytes[off] }); }
inline void patchCommit() { if (!g.patchPending.empty()) { g.patchUndo.push_back(g.patchPending); g.patchPending.clear(); g.patchRedo.clear(); } }
inline void refreshDisasmFromPatched();   // fwd
inline void undoPatch() {
    patchCommit();                                                   // seal any half-typed edit first
    if (g.patchUndo.empty()) { g.log("nothing to undo"); return; }
    auto grp = g.patchUndo.back(); g.patchUndo.pop_back();
    std::vector<std::pair<size_t, unsigned char>> inv;
    for (auto& p : grp) { if (p.first >= g.bin.bytes.size()) continue; inv.push_back({ p.first, g.bin.bytes[p.first] }); g.bin.bytes[p.first] = p.second;
        if (g.patchOrig.count(p.first) && g.bin.bytes[p.first] == g.patchOrig[p.first]) { g.patched.erase(p.first); g.patchOrig.erase(p.first); } }   // back to original -> not a patch anymore
    g.patchRedo.push_back(inv); g.patchDirty = !g.patched.empty(); refreshDisasmFromPatched();
    g.log("undo: reverted " + std::to_string(grp.size()) + " byte(s)" + (g.patched.empty() ? "  (binary now clean)" : ""));
}
inline void redoPatch() {
    if (g.patchRedo.empty()) { g.log("nothing to redo"); return; }
    auto grp = g.patchRedo.back(); g.patchRedo.pop_back();
    std::vector<std::pair<size_t, unsigned char>> inv;
    for (auto& p : grp) { if (p.first >= g.bin.bytes.size()) continue; inv.push_back({ p.first, g.bin.bytes[p.first] });
        if (!g.patchOrig.count(p.first)) g.patchOrig[p.first] = g.bin.bytes[p.first]; g.bin.bytes[p.first] = p.second; g.patched.insert(p.first); }
    g.patchUndo.push_back(inv); g.patchDirty = true; refreshDisasmFromPatched();
    g.log("redo: re-applied " + std::to_string(grp.size()) + " byte(s)");
}
// set one byte, remembering its ORIGINAL value (once) so the side-by-side diff can show before/after.
inline void setPatchByte(size_t off, unsigned char val) {
    if (off >= g.bin.bytes.size()) return;
    patchRecord(off);                                               // undo: capture the byte before we overwrite it
    if (!g.patchOrig.count(off)) g.patchOrig[off] = g.bin.bytes[off];
    g.bin.bytes[off] = val; g.patched.insert(off);
}
// write `nb` at file offset `off`, occupying `origLen` bytes; pad short with NOPs, reject if longer than the slot.
inline bool patchInstrAt(size_t off, const vector<unsigned char>& nb, int origLen) {
    if (nb.empty() || origLen <= 0 || off + (size_t)origLen > g.bin.bytes.size()) return false;
    if ((int)nb.size() > origLen) return false;                                   // would clobber the next instruction — refuse
    if (!isX86() && (nb.size() % 4 != 0 || origLen % 4 != 0)) return false;       // arm64 = whole 4-byte instructions only (clean NOP padding)
    if (!isX86() && g.bin.textSize && off >= g.bin.textOff && (off - g.bin.textOff) % 4 != 0) return false;   // arm64: must start ON a 4-byte instruction boundary
    for (size_t i = 0; i < nb.size(); i++) setPatchByte(off + i, nb[i]);
    static const unsigned char A64NOP[4] = { 0x1f, 0x20, 0x03, 0xd5 };
    for (int i = (int)nb.size(); i < origLen; i++) setPatchByte(off + i, isX86() ? 0x90 : A64NOP[i % 4]);
    patchCommit();   // the whole instruction = one undo step
    g.patchDirty = true; refreshDisasmFromPatched(); return true;
}
// a linker symbol -> its base function name: demangle C++ (`_Z6squarel` -> `square`), else strip the C `_` prefix.
inline string symBase(const string& sym) {
    int st = 0; char* d = abi::__cxa_demangle(sym.c_str(), 0, 0, &st);
    string full; if (st == 0 && d) full = d; else { full = sym; if (!full.empty() && full[0] == '_') full = full.substr(1); }
    if (d) free(d);
    size_t par = full.find('('); if (par != string::npos) full = full.substr(0, par);   // drop (args)
    while (!full.empty() && full.back() == ' ') full.pop_back();
    if (full.find('<') == string::npos) { size_t sp = full.rfind(' '); if (sp != string::npos) full = full.substr(sp + 1); }   // drop return type
    size_t cc = full.rfind("::"); if (cc != string::npos) full = full.substr(cc + 2);    // drop namespace/class qualifier
    return full;
}
// ── patch a whole function FROM its (edited) pseudocode ─────────────────────────
// Compile the decompiled C (whole program, so cross-references resolve), extract the
// selected function's machine bytes, and patch them over the original — if they fit.
// Works cleanly for self-contained functions; calls to other functions carry .o-relative
// offsets that an in-place patch can't relocate, so we warn and let you verify in Disasm.
inline void patchFnFromPseudo() {
    if (!g.bin.loaded || g.bin.funcs.empty()) { g.log("patch-from-pseudocode: load a binary first"); return; }
    int fi = g.selFn; if (fi < 0 || fi >= (int)g.bin.funcs.size()) { g.log("patch-from-pseudocode: select a function in the sidebar first"); return; }
    model::Func& F = g.bin.funcs[fi];
    if (F.addr == 0 || F.kind == 3) { g.log("patch-from-pseudocode: '" + F.name + "' has no recovered code address"); return; }
    string arch = g.bin.arch, triple = arch == "arm64" ? "arm64-apple-macos" : arch == "x86-64" ? "x86_64-apple-macos" : "";
    if (triple.empty()) { g.log("patch-from-pseudocode: only arm64 / x86-64 supported (this binary is " + arch + ")"); return; }
    size_t origOff = g.bin.addrToOffset(F.addr);
    if (origOff == SIZE_MAX) { g.log("patch-from-pseudocode: '" + F.name + "' isn't in .text"); return; }
    uint64_t origEndVa = g.bin.textVaddr + g.bin.textSize;                          // original slot = up to the next function
    for (auto& G : g.bin.funcs) if (G.addr > F.addr && G.addr < origEndVa) origEndVa = G.addr;
    size_t origLen = (size_t)(origEndVa - F.addr);
    // 1. compile the whole (edited) pseudocode so callees/structs/globals resolve
    string src; if (g.pseudoWhole && !g.wholeLines.empty()) { for (auto& l : g.wholeLines) src += l + "\n"; }
    else { for (auto& l : F.lines) src += l + "\n"; }
    string root = store::root(); store::mkdirs(root);
    string srcf = root + "/.patchfn.cpp", obj = root + "/.patchfn.o", errf = root + "/.patchfn.err";
    store::writeFile(srcf, src);
    string cmd = string("clang++ -std=c++17 -w -c -Os -fno-inline -fno-stack-protector -fno-asynchronous-unwind-tables -target ")
        + triple + " " + model::shq(srcf) + " -o " + model::shq(obj) + " 2>" + model::shq(errf);
    if (system(cmd.c_str()) != 0) { string e = model::trim(store::readFile(errf)); size_t nl = e.find('\n'); if (nl != string::npos) e = e.substr(0, nl);
        g.log("patch-from-pseudocode: source didn't compile — run Clean Up first, then edit. " + (e.empty() ? "" : "(" + e + ")")); return; }
    // 2. find F's bytes in the object: its symbol -> next symbol
    vector<uint8_t> o; if (!nx::readFile(obj.c_str(), o)) { g.log("patch-from-pseudocode: couldn't read the recompiled object"); return; }
    size_t toff, tsz; uint64_t tva; if (!nx::machoText(o.data(), o.size(), toff, tsz, tva)) { g.log("patch-from-pseudocode: no __text in the recompiled object"); return; }
    // `nm -n` lists ALL symbols sorted by address (including the offset-0 function), full mangled names
    string nmout; { FILE* p = popen(("nm -n " + model::shq(obj) + " 2>/dev/null").c_str(), "r"); if (p) { char ln[1024]; while (fgets(ln, sizeof ln, p)) nmout += ln; pclose(p); } }
    vector<std::pair<uint64_t, string>> syms;
    for (size_t i = 0; i < nmout.size();) { size_t e = nmout.find('\n', i); string ln = nmout.substr(i, e == string::npos ? string::npos : e - i); i = e == string::npos ? nmout.size() : e + 1;
        if (ln.size() < 19 || ln[0] == ' ') continue;                                  // undefined symbols carry no address
        uint64_t a = strtoull(ln.substr(0, 16).c_str(), nullptr, 16); size_t sp = ln.find(' ', 18); if (sp == string::npos) continue;
        syms.push_back({ a, model::trim(ln.substr(sp + 1)) }); }
    uint64_t fva = UINT64_MAX, nextva = tva + tsz;
    for (auto& s : syms) if (symBase(s.second) == F.name) { fva = s.first; break; }     // match by demangled base name
    if (fva == UINT64_MAX) { g.log("patch-from-pseudocode: couldn't find '" + F.name + "' in the recompiled object (inlined or renamed? give it a unique name)"); return; }
    for (auto& s : syms) if (s.first > fva && s.first < nextva) nextva = s.first;       // next symbol = end of this function
    size_t fb = toff + (size_t)(fva - tva), fe = toff + (size_t)(nextva - tva);
    if (fe > o.size() || fe <= fb) { g.log("patch-from-pseudocode: bad function bounds in the object"); return; }
    vector<unsigned char> nb(o.begin() + fb, o.begin() + fe);
    while (!nb.empty() && nb.back() == 0x00) nb.pop_back();                          // trim inter-function padding
    if (arch == "arm64") while (nb.size() % 4) nb.push_back(0);
    if (nb.empty()) { g.log("patch-from-pseudocode: recompiled '" + F.name + "' is empty"); return; }
    // 3. fit + patch
    if (nb.size() > origLen) { char e[200]; snprintf(e, sizeof e, "patch-from-pseudocode: recompiled %s is %d bytes — won't fit the %d-byte original (in-place patch must be same-or-smaller). Trampoline relocation is future work.", F.name.c_str(), (int)nb.size(), (int)origLen); g.log(e); return; }
    for (size_t i = 0; i < nb.size(); i++) setPatchByte(origOff + i, nb[i]);
    static const unsigned char A64NOP[4] = { 0x1f, 0x20, 0x03, 0xd5 };
    for (size_t i = nb.size(); i < origLen; i++) setPatchByte(origOff + i, isX86() ? 0x90 : A64NOP[i % 4]);   // pad the slack with NOPs
    patchCommit(); g.patchDirty = true; refreshDisasmFromPatched();
    g.mainTab = MAIN_HEX; g.tabOpen[MAIN_HEX] = true; g.hexSel = (int)origOff; g.hexNibble = 0;
    char b[240]; snprintf(b, sizeof b, "patched %s from pseudocode: %d bytes into a %d-byte slot (NOP-padded).  \xe2\x9a\xa0 calls to other functions aren't relocated — verify in Disassembly, then \xe2\x8c\x98S to write (.bak kept).", F.name.c_str(), (int)nb.size(), (int)origLen);
    g.log(b);
}
// resolve a virtual address -> (file offset, byte-slot length, original instruction text) using the disassembly
// listing. len=0 when the x86 length can't be derived from the next instruction (don't guess -> refuse the patch).
inline bool resolveInstrAtVa(uint64_t va, size_t& off, int& len, string& text) {
    off = g.bin.addrToOffset(va); len = 0; text.clear();
    if (off == SIZE_MAX || off >= g.bin.bytes.size()) return false;
    if (model::trim(g.bin.disasm).empty()) g.bin.runDisasm();
    const string& d = g.bin.disasm; vector<string> L; { size_t i = 0; while (i < d.size()) { size_t e = d.find('\n', i); L.push_back(d.substr(i, e == string::npos ? string::npos : e - i)); if (e == string::npos) break; i = e + 1; } }
    len = (g.bin.arch == "arm64") ? 4 : 0; bool found = false;
    for (size_t li = 0; li < L.size(); li++) if (parseAsmAddr(L[li]) == va) { found = true;
        size_t c = L[li].find(':'); text = (c != string::npos) ? model::trim(L[li].substr(c + 1)) : model::trim(L[li]);
        if (g.bin.arch != "arm64") for (size_t n = li + 1; n < L.size(); n++) { uint64_t na = parseAsmAddr(L[n]); if (na != UINT64_MAX) { if (na > va && na - va <= 15) len = (int)(na - va); break; } }
        break; }
    if (!found) len = 0;   // va is NOT the start of any disassembled instruction (e.g. a misaligned arm64 addr) -> refuse; never guess a slot
    if (len > 0 && off + (size_t)len > g.bin.bytes.size()) len = 0;
    return true;
}
// Best-effort map a pseudocode LINE -> the single instruction it most likely came from, within function
// fi's disasm range. The decompiled C is a reconstruction (no exact provenance survives collapse/edits),
// so we anchor on the line's salient operand: a call to a known function, a string literal, or a numeric
// constant. Returns that instruction's vaddr, or UINT64_MAX if it can't confidently pin one (caller then
// just offers "Show in Disassembly"). Nails the patch-relevant cases — kill a call, change a constant.
inline uint64_t anchorPseudoLine(const string& line, int fi) {
    if (fi < 0 || fi >= (int)g.bin.funcs.size()) return UINT64_MAX;
    uint64_t fa = g.bin.funcs[fi].addr; if (!fa) return UINT64_MAX;
    uint64_t fe = g.bin.textVaddr + g.bin.textSize; for (auto& G : g.bin.funcs) if (G.addr > fa && G.addr < fe) fe = G.addr;   // range = up to the next function
    if (model::trim(g.bin.disasm).empty()) g.bin.runDisasm();
    vector<std::pair<uint64_t,string>> ins; { const string& d = g.bin.disasm; size_t i = 0;
        while (i < d.size()) { size_t e = d.find('\n', i); string ln = d.substr(i, e == string::npos ? string::npos : e - i); i = e == string::npos ? d.size() : e + 1;
            uint64_t a = parseAsmAddr(ln); if (a == UINT64_MAX || a < fa || a >= fe) continue;
            size_t c = ln.find(':'); ins.push_back({ a, c == string::npos ? ln : ln.substr(c + 1) }); } }
    if (ins.empty()) return UINT64_MAX;
    auto low = [](string s) { for (auto& c : s) c = (char)tolower((unsigned char)c); return s; };
    auto idc = [](char c) { return isalnum((unsigned char)c) || c == '_'; };
    // 1) CALL: an identifier immediately followed by '(' that names another decompiled function -> its bl/call
    for (size_t i = 0; i < line.size();) { if (isalpha((unsigned char)line[i]) || line[i] == '_') { size_t j = i; while (j < line.size() && idc(line[j])) j++;
        if (j < line.size() && line[j] == '(') { string nm = line.substr(i, j - i);
            for (auto& G : g.bin.funcs) if (G.kind == 0 && G.addr && G.name == nm) { char hx[20]; snprintf(hx, sizeof hx, "%llx", (unsigned long long)G.addr);
                for (auto& p : ins) { string t = low(p.second); bool isCall = t.find("bl ") != string::npos || t.find("bl\t") != string::npos || t.find("call") != string::npos;
                    if (isCall && (t.find(nm) != string::npos || t.find(hx) != string::npos)) return p.first; } } }
        i = j; } else i++; }
    // 2) STRING literal -> the instruction that materializes that string's address
    { size_t q = line.find('"'); if (q != string::npos) { size_t q2 = line.find('"', q + 1); if (q2 != string::npos && q2 > q + 1) { string str = line.substr(q + 1, q2 - q - 1);
        for (auto& S : g.bin.strings) if (S.text == str) { char hx[20]; snprintf(hx, sizeof hx, "%llx", (unsigned long long)S.addr);
            for (auto& p : ins) if (low(p.second).find(hx) != string::npos) return p.first; } } } }
    // 3) numeric CONSTANT (>1, to skip noise) -> a mov/cmp/add/sub carrying that immediate
    for (size_t i = 0; i < line.size();) { if (isdigit((unsigned char)line[i]) && (i == 0 || !idc(line[i - 1]))) {
        uint64_t v = strtoull(line.c_str() + i, nullptr, 0); size_t j = i;
        if (line[i] == '0' && i + 1 < line.size() && (line[i + 1] == 'x' || line[i + 1] == 'X')) { j = i + 2; while (j < line.size() && isxdigit((unsigned char)line[j])) j++; }
        else while (j < line.size() && isdigit((unsigned char)line[j])) j++;
        if (v > 1) { char hx[24]; snprintf(hx, sizeof hx, "#0x%llx", (unsigned long long)v); char dc[24]; snprintf(dc, sizeof dc, "#%llu", (unsigned long long)v);
            for (auto& p : ins) { string t = low(p.second); bool movish = t.find("mov") != string::npos || t.find("cmp") != string::npos || t.find("add") != string::npos || t.find("sub") != string::npos || t.find("and") != string::npos || t.find("orr") != string::npos;
                if (movish && (t.find(hx) != string::npos || t.find(dc) != string::npos)) return p.first; } }
        i = j; } else i++; }
    return UINT64_MAX;
}
// queue a confirm-gated AI byte patch. `nop` fills the instruction slot with NOPs; otherwise assemble `ins`.
// EVERYTHING is validated up front (resolvable address, known length, assembles clean, fits the slot) and only
// then queued for the user to approve in drawAiPatch(). Nothing is written to the bytes here.
inline void queueAiPatch(uint64_t va, const string& ins, bool nop) {
    if (!g.bin.loaded) { aiPush("  (no binary loaded)"); return; }
    size_t off; int len; string was;
    if (!resolveInstrAtVa(va, off, len, was)) { char b[96]; snprintf(b, sizeof b, "  (0x%llx is not inside the .text section)", (unsigned long long)va); aiPush(b); return; }
    if (len <= 0) { char b[120]; snprintf(b, sizeof b, "  (couldn't determine the instruction length at 0x%llx — patch it by hand)", (unsigned long long)va); aiPush(b); return; }
    AiPatch p; p.off = off; p.slot = len; p.nop = nop;
    if (nop) {
        static const unsigned char A64NOP[4] = { 0x1f, 0x20, 0x03, 0xd5 };
        for (int k = 0; k < len; k++) p.bytes.push_back(isX86() ? 0x90 : A64NOP[k % 4]);
        char b[200]; snprintf(b, sizeof b, "NOP  @ 0x%llx   (%d byte%s, was: %s)", (unsigned long long)va, len, len == 1 ? "" : "s", was.c_str()); p.desc = b;
    } else {
        vector<unsigned char> nb = assembleInstr(ins, g.bin.arch);
        if (nb.empty()) { aiPush("  (couldn't assemble \"" + ins + "\")"); return; }
        if (!isX86() && nb.size() % 4 != 0) { aiPush("  (arm64 needs whole 4-byte instructions)"); return; }
        if ((int)nb.size() > len) { char b[128]; snprintf(b, sizeof b, "  (\"%s\" is %d bytes — won't fit the %d-byte slot at 0x%llx)", ins.c_str(), (int)nb.size(), len, (unsigned long long)va); aiPush(b); return; }
        p.bytes = nb;
        char b[224]; snprintf(b, sizeof b, "%s   @ 0x%llx   (was: %s)", ins.c_str(), (unsigned long long)va, was.c_str()); p.desc = b;
    }
    if (!g.aiPatchPrompt) aiPatchJustOpened = true;   // only on the false->true transition: ignore an Enter that's live on the open frame
    g.aiPatchQ.push_back(p); g.aiPatchPrompt = true; g.focus = F_NONE;   // take focus off the chat so Enter confirms the dialog
    aiPush("  - proposed: " + p.desc);
}

// ── CFG graph view (Ghidra/BN-style box-and-arrow rendering of the function) ──
// Splits the structured pseudocode into basic blocks at control-flow boundaries
// (if/else/while/for, loc_ labels, goto/return) and lays them out as connected
// boxes: fall-through edges down, goto edges to their label. Drag to pan, wheel to scroll.
inline void panelGraph(float x, float y, float w, float h) {
    float s = ui::in.scale, adv = ui::advance(), lh = ui::lineH() * 1.02f;
    ui::rect(x, y, w, h, th::Col{0.10f,0.10f,0.12f}); ui::pushClip(x, y, w, h);
    if (g.bin.funcs.empty()) { ui::text(x + 16 * s, y + 16 * s, "no functions", th::TEXT_MUT); ui::popClip(); return; }
    static int lastGraphFn = -1; if (g.selFn != lastGraphFn) { g.graphPanX = g.graphPanY = 0; lastGraphFn = g.selFn; }   // switching functions re-centers the graph
    auto& f = g.bin.funcs[std::min(g.selFn, (int)g.bin.funcs.size() - 1)];
    struct Blk { int depth = 0; vector<int> lines; bool isLabel = false; string label; bool isGoto = false; string gotoTo; };
    vector<Blk> blks; { int depth = 0; Blk cur; bool has = false;
        auto cnt = [](const string& t, char ch) { int n = 0; for (char c : t) if (c == ch) n++; return n; };
        auto flush = [&]() { if (has) blks.push_back(cur); cur = Blk{}; has = false; };
        for (int i = 0; i < (int)f.lines.size(); i++) { string t = model::trim(f.lines[i]); if (t.empty()) continue;
            int lineDepth = depth - (t[0] == '}' ? 1 : 0); if (lineDepth < 0) lineDepth = 0;
            bool isLabel = t.size() > 1 && t.back() == ':' && t.rfind("loc_", 0) == 0;
            bool isCtl = t.rfind("if", 0) == 0 || t.rfind("} else", 0) == 0 || t.rfind("else", 0) == 0 || t.rfind("while", 0) == 0 || t.rfind("for", 0) == 0 || t.rfind("switch", 0) == 0;
            bool isGoto = t.rfind("goto ", 0) == 0, isRet = t.rfind("return", 0) == 0;
            if (isLabel || isCtl || (has && cur.depth != lineDepth)) flush();
            if (!has) { cur.depth = lineDepth; has = true; }
            cur.lines.push_back(i); if (isLabel) { cur.isLabel = true; cur.label = t.substr(0, t.size() - 1); }
            depth += cnt(t, '{') - cnt(t, '}');
            if (isGoto) { cur.isGoto = true; size_t sp = t.find(' '); string gt = sp == string::npos ? "" : t.substr(sp + 1); if (!gt.empty() && gt.back() == ';') gt.pop_back(); cur.gotoTo = gt; flush(); }
            else if (isRet) flush(); }
        flush(); }
    // layout: vertical stack, indented by depth
    std::map<string, int> labelBlk; for (int i = 0; i < (int)blks.size(); i++) if (blks[i].isLabel) labelBlk[blks[i].label] = i;
    float boxW = std::min(w * 0.62f, 560 * s), gapY = 18 * s, indent = 26 * s;
    vector<float> bx(blks.size()), byv(blks.size()), bh(blks.size());
    float cy = y + g.graphPanY + 16 * s, contentW = 0;
    for (int i = 0; i < (int)blks.size(); i++) { float bhh = blks[i].lines.size() * lh + 10 * s;
        bx[i] = x + g.graphPanX + 24 * s + blks[i].depth * indent; byv[i] = cy; bh[i] = bhh; cy += bhh + gapY;
        contentW = std::max(contentW, 24 * s + blks[i].depth * indent + boxW); }
    float contentH = cy - (y + g.graphPanY + 16 * s);   // sum of box heights + gaps (pan-independent)
    // pan: only while a drag that STARTED inside the graph is held (latched — no jump when dragging in from elsewhere)
    if (ui::hovered(x, y, w, h)) g.graphPanY += (int)ui::in.wheel * lh;
    static bool gPanning = false; static float lmx = 0, lmy = 0;
    if (ui::in.lPress && ui::inside(x, y, w, h)) { gPanning = true; lmx = ui::in.mx; lmy = ui::in.my; }
    if (!ui::in.lDown) gPanning = false;
    if (gPanning) { g.graphPanX += ui::in.mx - lmx; g.graphPanY += ui::in.my - lmy; lmx = ui::in.mx; lmy = ui::in.my; }
    // CLAMP so you can't pan the graph off into the void — keep content reachable, with a small margin
    float marginY = 24 * s, minPanY = std::min(0.0f, (h - marginY) - contentH);
    g.graphPanY = std::max(minPanY, std::min(0.0f, g.graphPanY));
    float minPanX = std::min(0.0f, (w - 24 * s) - contentW);
    g.graphPanX = std::max(minPanX, std::min(24.0f * s, g.graphPanX));
    // edges first (under the boxes)
    for (int i = 0; i < (int)blks.size(); i++) {
        if (blks[i].isGoto && labelBlk.count(blks[i].gotoTo)) { int t = labelBlk[blks[i].gotoTo];   // goto -> label (accent)
            ui::line(bx[i] + 14 * s, byv[i] + bh[i], bx[t] + 14 * s, byv[t], 1.5f * s, th::ACCENT2);
            ui::tri(bx[t] + 10 * s, byv[t] - 7 * s, bx[t] + 18 * s, byv[t] - 7 * s, bx[t] + 14 * s, byv[t], th::ACCENT2); }
        else if (!blks[i].isGoto && i + 1 < (int)blks.size()) {                                    // fall-through -> next
            float x0 = bx[i] + 14 * s, x1 = bx[i + 1] + 14 * s;
            ui::line(x0, byv[i] + bh[i], x1, byv[i + 1], 1.3f * s, th::BORDER);
            ui::tri(x1 - 4 * s, byv[i + 1] - 6 * s, x1 + 4 * s, byv[i + 1] - 6 * s, x1, byv[i + 1], th::TEXT_MUT); }
    }
    // boxes
    for (int i = 0; i < (int)blks.size(); i++) {
        if (byv[i] + bh[i] < y || byv[i] > y + h) continue;
        Col edge = blks[i].isLabel ? th::ACCENT2 : blks[i].isGoto ? th::ACCENT : th::BORDER;
        ui::rect(bx[i], byv[i], boxW, bh[i], th::PANEL); ui::rectLine(bx[i], byv[i], boxW, bh[i], edge);
        ui::rect(bx[i], byv[i], 2.5f * s, bh[i], edge);
        ui::pushClip(bx[i], byv[i], boxW, bh[i]); ui::beginText();
        for (int r = 0; r < (int)blks[i].lines.size(); r++) drawCode(bx[i] + 8 * s, byv[i] + 5 * s + r * lh, model::trim(f.lines[blks[i].lines[r]]), adv);
        ui::endText(); ui::popClip();
    }
    ui::popClip();
    ui::text(x + 10 * s, y + h - lh - 4 * s, std::to_string((int)blks.size()) + " blocks · drag to pan · scroll to move", th::TEXT_MUT);
}

// ── file explorer (real on-disk project tree) ────────────────────────────────
// collect just the decompiled source/output files (skip folders, manifests, .DS_Store)
inline string extOf(const string& n) { size_t d = n.rfind('.'); return d == string::npos ? "" : n.substr(d + 1); }
inline bool srcExt(const string& e) { return e == "c" || e == "cpp" || e == "cc" || e == "h" || e == "hpp" || e == "s" || e == "asm" || e == "json"; }
// flatten the on-disk tree to the visible rows (respecting folder expand state); keep folders + their source files
inline void flattenTree(store::Node& n, int depth, bool isRoot, vector<std::pair<store::Node*,int>>& out) {
    if (!isRoot) { if (n.dir) out.push_back({ &n, depth }); else if (srcExt(extOf(n.name))) out.push_back({ &n, depth }); }
    if (n.dir && (isRoot || n.expanded)) { std::sort(n.children.begin(), n.children.end(), [](const store::Node& a, const store::Node& b) { if (a.dir != b.dir) return a.dir > b.dir; return a.name < b.name; });
        for (auto& c : n.children) flattenTree(c, isRoot ? depth : depth + 1, false, out); }
}
inline void panelFiles(float x, float y, float w, float h) {     // real project tree: folders (expandable) + source files
    float s = ui::in.scale, rh = ui::lineH() + th::ROW_PAD * s;
    vector<std::pair<store::Node*,int>> rowsv; if (g.bin.loaded) flattenTree(g.fileTree, 0, true, rowsv);
    ui::pushClip(x, y, w, h);
    int vis = (int)(h / rh);
    if (ui::hovered(x, y, w, h)) g.fileScroll += (int)ui::in.wheel;   // wheel before clamp (same fast-up-scroll crash as the other side panels)
    if (g.fileScroll > (int)rowsv.size() - vis) g.fileScroll = std::max(0, (int)rowsv.size() - vis); if (g.fileScroll < 0) g.fileScroll = 0;
    if (rowsv.empty()) ui::text(x + 10 * s, y + 8 * s, g.bin.loaded ? "no source files" : "no project open", th::TEXT_MUT);
    for (int r = 0; r < vis; r++) { int li = g.fileScroll + r; if (li < 0 || li >= (int)rowsv.size()) break; store::Node* nd = rowsv[li].first; int depth = rowsv[li].second; float ry = y + r * rh, ix = x + 8 * s + depth * 14 * s;
        bool hov = ui::hovered(x, ry, w, rh), sel = !nd->dir && nd->name == g.fileViewName && nd->path == g.fileViewPath && g.mainTab == MAIN_FILE;
        if (sel) { ui::rect(x, ry, w, rh, th::SEL); ui::rect(x, ry, 2.5f * s, rh, th::ACCENT); } else if (hov) ui::rect(x, ry, w, rh, th::HOVER);
        if (nd->dir) { ui::icon(ix, ry + (rh - 10 * s) / 2, 10 * s, nd->expanded ? ui::IC_OPEN : ui::IC_FILE, th::ACCENT2);   // folder
            ui::textClip(ix + 16 * s, ry + (rh - ui::lineH()) / 2, w - depth * 14 * s - 40 * s, nd->name, th::TEXT);
            if (ui::clicked(x, ry, w, rh)) nd->expanded = !nd->expanded; }
        else { string e = extOf(nd->name);
            Col chip = e == "c" ? th::CHIP_FN : (e == "cpp" || e == "cc") ? th::CHIP_CLASS : (e == "h" || e == "hpp") ? th::CHIP_STRUCT : th::TEXT_DIM;
            ui::rect(ix + 1 * s, ry + (rh - 8 * s) / 2, 8 * s, 8 * s, chip);
            ui::textClip(ix + 16 * s, ry + (rh - ui::lineH()) / 2, w - depth * 14 * s - 50 * s, nd->name, sel ? th::TEXT : th::Col{0.78f,0.8f,0.84f});
            if (ui::clicked(x, ry, w, rh)) openFileInViewer(nd->path); }
    }
    ui::popClip(); ui::scrollbar(x + w - 7 * s, y, 7 * s, h, (int)rowsv.size(), vis, &g.fileScroll);
}
inline void panelFileView(float x, float y, float w, float h) {   // editable file editor (DocView-backed: caret, select, copy, save)
    float s = ui::in.scale, lh = ui::lineH() * 1.05f, sigH = lh + 8 * s;
    g.fileDirty = docFile.dirty;
    ui::rect(x, y, w, sigH, th::PANEL_HI); ui::hline(x, y + sigH - s, w, th::BORDER);
    ui::icon(x + 8 * s, y + (sigH - 13 * s) / 2, 13 * s, ui::IC_FILE, th::TEXT_DIM);
    ui::text(x + 26 * s, y + (sigH - ui::lineH()) / 2, (g.fileDirty ? string("* ") : string()) + (g.fileViewName.empty() ? "(no file)" : g.fileViewName), th::TEXT);
    if (ui::iconButton(x + w - 30 * s, y + (sigH - 20 * s) / 2, 20 * s, ui::IC_SAVE, g.fileDirty)) saveFile();
    bool code = g.fileViewName.find(".c") != string::npos || g.fileViewName.find(".h") != string::npos || g.fileViewName.find(".s") != string::npos;
    docFile.bind(&g.editLines, "file:" + g.fileViewPath); docFile.editable = true; docFile.syntax = code;
    float cy = y + sigH, ch = h - sigH;
    docFile.run(x, cy, w, ch, activeDoc == &docFile);
    if (ui::rclicked(x, cy, w, ch)) { activeDoc = &docFile; g.focus = F_DOC;
        docFile.placeCaretAt(ui::in.mx, ui::in.my);                                       // put the caret under the right-click so Go-to-def / Fix act on THAT token
        vector<ui::MenuItem> m = { {"Copy selection", ACT_COPY}, {"Select all", ACT_SELALL} };
        if (!docFile.lspPath.empty() && lsp::client.isAlive()) { m.insert(m.begin(), { "Rename symbol (F2)", ACT_RENAME }); m.insert(m.begin(), { "Go to definition", ACT_DEF });
            for (auto& d : lsp::client.diagnostics(docFile.lspPath)) if (d.line == docFile.caretL && !d.fix.empty()) { m.push_back({ "", 0, true }); m.push_back({ "Apply fix: " + d.fixTitle, ACT_FIX }); break; } }
        ui::openMenu(ui::in.mx, ui::in.my, m); }
}
// ── command palette (Cmd/Ctrl+Shift+P) ───────────────────────────────────────
inline void runPaletteAction(int id) {
    switch (id) { case 1: goHome(); break; case 2: g.pending = P_OPEN; break;
        case 3: if (g.bin.loaded) { g.pending = P_OPENPATH; g.pendingArg = g.bin.path; } break;
        case 4: g.pending = P_EXPORT; break; case 5: saveFile(); break; case 6: runOptimize(); break;
        case 7: g.pseudoJump = true; break; case 8: g.tabOpen[MAIN_DISASM] = true; g.mainTab = MAIN_DISASM; break; case 9: g.tabOpen[MAIN_HEX] = true; g.mainTab = MAIN_HEX; break;
        case 10: g.showRight = !g.showRight; break; case 11: g.showBottom = !g.showBottom; break;
        case 12: g.tabOpen[MAIN_GRAPH] = true; g.mainTab = MAIN_GRAPH; break; case 13: toggleFind(); break; case 14: startRenameActive(); break; case 15: if (g.bin.loaded) runRecompile(); break;
        case 16: runUnderstand(); break; case 17: savePatches(); break; case 18: if (g.bin.loaded) runDebug(); break; case 19: toggleSettings(); break;
        case 20: if (g.bin.loaded) runEmberRun(); break;
        case 21: runAutoName(); break; case 22: runPropagate(); break;
        case 23: patchFnFromPseudo(); break;
        case 24: if (g.bin.loaded) {                                                     // Info: static binary intel via ember-info -> log panel
            string tool = store::exists(g.dir + "ember-info") ? g.dir + "ember-info" : "/usr/local/bin/ember-info";
            g.showBottom = true; string out = model::runCmd(model::shq(tool) + " " + model::shq(g.bin.path) + " 2>/dev/null");
            if (out.empty()) g.log("info: ember-info unavailable");
            else for (size_t i = 0; i < out.size();) { size_t e = out.find('\n', i); g.log(out.substr(i, (e == string::npos ? out.size() : e) - i)); if (e == string::npos) break; i = e + 1; }
        } break; }
}
// in-app "Decompiling…" popup — themed modal + sweeping progress bar (shown while the async decompile runs)
inline void drawDecompiling() {
    if (!analyzing.load()) return; float s = ui::in.scale;
    ui::rect(0, 0, ui::winW, ui::winH, th::Col{0, 0, 0, 0.55f});                          // dim the workspace
    float bw = std::min(ui::winW * 0.52f, 480 * s), bh = 172 * s, bx = (ui::winW - bw) / 2, by = (ui::winH - bh) / 2;
    ui::rect(bx, by, bw, bh, th::PANEL); ui::rectLine(bx, by, bw, bh, th::ACCENT); ui::rect(bx, by, bw, 3 * s, th::ACCENT);
    ui::pushClip(bx, by, bw, bh);                                                         // never let content spill off the popup
    drawLogo(bx + (bw - 42 * s) / 2, by + 18 * s, 42 * s);
    float inner = bw - 32 * s;                                                            // text must fit inside the box with padding
    auto elide = [&](string t) { if (ui::textW(t) <= inner) return t;                     // truncate with an ellipsis so centering can't go negative
        while (t.size() > 1 && ui::textW(t + "\xe2\x80\xa6") > inner) t.pop_back(); return t + "\xe2\x80\xa6"; };
    string title = elide("Decompiling " + anaName);
    ui::text(bx + (bw - ui::textW(title)) / 2, by + 74 * s, title, th::TEXT);
    int c = anaCount.load(); string sub = elide((c > 0 ? std::to_string(c) + " functions  \xc2\xb7  " : string()) + anaStatus);
    ui::text(bx + (bw - ui::textW(sub)) / 2, by + 74 * s + ui::lineH() + 5 * s, sub, th::TEXT_MUT);
    ui::popClip();
    static int tick = 0; tick++;                                                          // indeterminate sweep (total unknown until done)
    float pbx = bx + 28 * s, pby = by + bh - 28 * s, pbw = bw - 56 * s, pbh = 7 * s, seg = pbw * 0.32f, span = pbw + seg;
    ui::rect(pbx, pby, pbw, pbh, th::SUNKEN);
    float pos = (float)(((int)(tick * 4 * s)) % (int)(span < 1 ? 1 : span)) - seg;
    ui::pushClip(pbx, pby, pbw, pbh); ui::rect(pbx + pos, pby, seg, pbh, th::ACCENT); ui::popClip();
}
inline void drawPalette() {
    if (!g.paletteOpen) return; float s = ui::in.scale;
    ui::rect(0, 0, ui::winW, ui::winH, th::Col{0, 0, 0, 0.45f});
    struct PA { const char* label; int id; }; static const PA A[] = { {"Go to Home", 1}, {"Open Binary...", 2}, {"Reload", 3}, {"Export decomp/", 4}, {"Save File", 5}, {"Analyze (deep clean-up -> readable source)", 6}, {"Auto-Name functions (analyze + rename all)", 21}, {"Propagate my renames across program", 22}, {"Understand program (AI rename all)", 16}, {"Patch function from pseudocode (assemble + apply)", 23}, {"Info: header + sections + entropy + imports", 24}, {"Recompile source -> binary", 15}, {"Find in view", 13}, {"Rename symbol", 14}, {"Patch: save bytes to binary", 17}, {"EmberRun: execute binary", 20}, {"Debug with lldb", 18}, {"Settings", 19}, {"View: Pseudocode", 7}, {"View: Graph (CFG)", 12}, {"View: Disassembly", 8}, {"View: Hex (byte editor)", 9}, 
#ifdef EMBER_AI
        {"Toggle AI Panel", 10},
#else
        {"Toggle xrefs panel", 10},
#endif
        {"Toggle Log Panel", 11} };
    // ── input FIRST, so the query text drawn below reflects this frame's keystroke (no one-frame lag) ──
    if (ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) { if (!g.paletteQuery.empty()) g.paletteQuery.pop_back(); } else if (ui::in.ch >= 32 && ui::in.ch < 127) g.paletteQuery += (char)ui::in.ch; g.paletteSel = 0; }
    auto low = [](string t) { for (auto& c : t) c = tolower(c); return t; }; string q = low(g.paletteQuery);
    vector<int> idx; for (int i = 0; i < (int)(sizeof(A) / sizeof(A[0])); i++) if (q.empty() || low(A[i].label).find(q) != string::npos) idx.push_back(i);
    int k = ui::in.key; bool act = false; int actId = -1;
    if (k == ui::K_DOWN) g.paletteSel++; else if (k == ui::K_UP) g.paletteSel--;
    if (g.paletteSel >= (int)idx.size()) g.paletteSel = (int)idx.size() - 1; if (g.paletteSel < 0) g.paletteSel = 0;
    float pw = std::min(ui::winW * 0.6f, 560 * s), px = (ui::winW - pw) / 2, py = ui::winH * 0.12f, ih = ui::lineH() + 16 * s, rh = ui::lineH() + 8 * s;
    float ph = ih + idx.size() * rh + 10 * s;
    ui::rect(px - 1, py - 1, pw + 2, ph + 2, th::BORDER); ui::rect(px, py, pw, ph, th::PANEL_HI);
    ui::rect(px + 8 * s, py + 8 * s, pw - 16 * s, ih - 4 * s, th::SUNKEN); ui::rectLine(px + 8 * s, py + 8 * s, pw - 16 * s, ih - 4 * s, th::ACCENT);
    ui::text(px + 16 * s, py + 8 * s + (ih - 4 * s - ui::lineH()) / 2, g.paletteQuery.empty() ? "type a command..." : g.paletteQuery, g.paletteQuery.empty() ? th::TEXT_MUT : th::TEXT);
    float ly = py + ih + 4 * s;
    for (int r = 0; r < (int)idx.size(); r++) { float ry = ly + r * rh; bool sel = r == g.paletteSel;
        if (sel) ui::rect(px + 4 * s, ry, pw - 8 * s, rh, th::SEL);
        ui::text(px + 16 * s, ry + (rh - ui::lineH()) / 2, A[idx[r]].label, sel ? th::TEXT : th::TEXT_DIM);
        if (ui::in.lPress && ui::inside(px + 4 * s, ry, pw - 8 * s, rh)) { act = true; actId = A[idx[r]].id; } }
    if (k == ui::K_ENTER) { if (g.paletteSel >= 0 && g.paletteSel < (int)idx.size()) { act = true; actId = A[idx[g.paletteSel]].id; } }
    else if (k == ui::K_ESC) g.paletteOpen = false;
    if (!paletteJustOpened && ui::in.lPress && !ui::inside(px, py, pw, ph)) g.paletteOpen = false;   // click outside closes (but not the click that opened it)
    if (act) { g.paletteOpen = false; runPaletteAction(actId); }   // run AFTER closing so an action that opens another overlay (Settings) sticks
    paletteJustOpened = false;
}
inline void processPending() {   // run deferred actions OUTSIDE the GL draw (modal dialogs, heavy export)
    if (g.pending == P_NONE) return; Pending p = g.pending; g.pending = P_NONE; string arg = g.pendingArg;
    if (p == P_OPEN) { if (pickFile) openBinary(pickFile()); }
    else if (p == P_OPENPATH) openBinary(arg);
    else if (p == P_EXPORT) exportBinary();
    else if (p == P_SAVEAS) { if (pickSavePath) { string np = pickSavePath(); if (!np.empty()) writePatched(np, false); } }
}

// ── right dock: xrefs + AI ───────────────────────────────────────────────────
inline void panelXrefs(float x, float y, float w, float h) {
    float s = ui::in.scale; float hh = panelHeader(x, y, w, ui::IC_XREF, "Cross-references", ""); y += hh; h -= hh;
    if (g.bin.funcs.empty() || g.selFn >= (int)g.bin.funcs.size()) { ui::pushClip(x, y, w, h); ui::popClip(); return; }
    auto& f = g.bin.funcs[g.selFn];
    float rh = ui::lineH() + 4 * s;
    int total = (int)((f.callers.empty() ? 1 : f.callers.size()) + (f.callees.empty() ? 1 : f.callees.size())) + 3;   // 2 headers + a gap row
    int vis = std::max(1, (int)(h / rh));
    if (ui::hovered(x, y, w, h)) g.xrefScroll += (int)ui::in.wheel;          // wheel before clamp (no negative-scroll OOB)
    if (g.xrefScroll > total - vis) g.xrefScroll = std::max(0, total - vis); if (g.xrefScroll < 0) g.xrefScroll = 0;
    ui::pushClip(x, y, w, h); float cy = y + 4 * s - g.xrefScroll * rh;
    auto visible = [&](float ry) { return ry + rh > y && ry < y + h; };
    auto section = [&](const char* title, vector<int>& v) {
        if (visible(cy)) ui::text(x + 8 * s, cy, title, th::TEXT_DIM); cy += rh;
        if (v.empty()) { if (visible(cy)) ui::text(x + 18 * s, cy, "(none)", th::TEXT_MUT); cy += rh; }
        for (int idx : v) { if (idx >= 0 && idx < (int)g.bin.funcs.size() && visible(cy)) { bool hov = ui::hovered(x, cy, w, rh); if (hov) ui::rect(x, cy, w, rh, th::HOVER);
            ui::icon(x + 10 * s, cy + 2 * s, 11 * s, ui::IC_FUNC, th::ACCENT); ui::textClip(x + 26 * s, cy, w - 30 * s, g.bin.funcs[idx].name, hov ? th::TEXT : th::Col{0.72f,0.74f,0.79f});
            if (ui::clicked(x, cy, w, rh)) { g.selFn = idx; g.codeScroll = 0; g.pseudoJump = true; } } cy += rh; } };
    section("CALLERS (xrefs to)", f.callers); cy += 4 * s; section("CALLEES (xrefs from)", f.callees);
    ui::popClip();
    ui::scrollbar(x + w - 7 * s, y, 7 * s, h, total, vis, &g.xrefScroll);
}
inline void panelAI(float x, float y, float w, float h) {
    float s = ui::in.scale; float hh = panelHeader(x, y, w, ui::IC_AI, "AI Assistant", g.aiOptIn ? "on" : "off");
    ui::icon(x + 8 * s, y + (hh - 13 * s) / 2, 13 * s, ui::IC_AI, g.aiOptIn ? th::ACCENT2 : th::TEXT_MUT);
    float by = y + hh, bh = h - hh; ui::rect(x, by, w, bh, th::PANEL);
    // ── opt-in toggle (AI is OFF by default; nothing is sent to Claude until this is on) ──
    float tRow = ui::lineH() + 10 * s, ty = by + 7 * s, boxh = ui::lineH() + 2 * s, boxw = boxh * 1.9f, tx = x + w - boxw - 10 * s;
    ui::textClip(x + 10 * s, ty + (boxh - ui::lineH()) / 2, w - boxw - 24 * s, "Use Claude (claude CLI)", th::TEXT);
    ui::rect(tx, ty, boxw, boxh, g.aiOptIn ? th::ACCENT : th::SUNKEN); ui::rectLine(tx, ty, boxw, boxh, th::BORDER);
    ui::rect(g.aiOptIn ? tx + boxw - boxh + 2 * s : tx + 2 * s, ty + 2 * s, boxh - 4 * s, boxh - 4 * s, g.aiOptIn ? th::Col{0.10f,0.09f,0.06f} : th::TEXT_MUT);
    if (ui::clicked(x + 8 * s, ty, w - 16 * s, boxh)) { g.aiOptIn = !g.aiOptIn; saveConfig(); g.log(g.aiOptIn ? "AI enabled (claude CLI rewrite)" : "AI disabled"); }
    float btnH = ui::lineH() + 10 * s, aby = by + 7 * s + tRow;
    bool canAI = g.bin.loaded && g.aiOptIn;
    const char* lbl = !g.aiOptIn ? "enable Claude above" : !g.bin.loaded ? "open a binary first" : aiBusy.load() ? "asking Claude..." : "Rewrite function (Claude)";
    if (ui::button(x + 8 * s, aby, w - 16 * s, btnH, lbl, canAI && !aiBusy.load(), canAI && !aiBusy.load())) {
        if (canAI && !aiBusy.load() && g.selFn < (int)g.bin.funcs.size()) { auto& f = g.bin.funcs[g.selFn];
            string code; for (auto& l : f.lines) code += l + "\n"; string dir = g.dir, fname = f.name;
            aiPush("> rewriting " + fname + " ..."); aiBusy = true;
            std::thread([dir, code]() {                              // off the UI thread (claude -p takes a few seconds)
                string cmd = "printf %s " + model::shq(code) + " | " + model::shq(dir + "ember-claude") + " 2>&1";
                string out = model::runCmd(cmd);
                { std::lock_guard<std::mutex> lk(aiMx); aiResult.clear(); size_t i = 0;
                  while (i < out.size()) { size_t e = out.find('\n', i); aiResult.push_back(out.substr(i, e == string::npos ? string::npos : e - i)); if (e == string::npos) break; i = e + 1; }
                  if (aiResult.empty()) aiResult = { "(no output — is the `claude` CLI installed + logged in?)" }; }
                aiBusy = false; aiDone = true;
            }).detach();
        }
    }
    // whole-program function renaming — works OFFLINE (heuristics) and uses Claude when enabled. + propagate across everything.
    float uby = aby + btnH + 6 * s, halfW = (w - 16 * s - 6 * s) / 2; bool busyU = understanding.load();
    const char* anl = busyU ? "naming..." : g.aiOptIn ? "Auto-Name (AI)" : "Auto-Name fns";
    if (ui::button(x + 8 * s, uby, halfW, btnH, anl, false, g.bin.loaded && !busyU)) runAutoName();
    if (ui::button(x + 8 * s + halfW + 6 * s, uby, halfW, btnH, "Propagate", false, g.bin.loaded)) runPropagate();
    float lh = ui::lineH(), adv = ui::advance(), obw = w - 16 * s; int cols = adv > 0 ? std::max(8, (int)((obw - 12 * s) / adv)) : 80;
    // wrap the chat INPUT (multi-line: Shift+Enter inserts a newline) so long text + newlines stay on-screen
    vector<string> inWrap; { string cur2; auto flush = [&]() { if (cur2.empty()) inWrap.push_back(""); else for (size_t i = 0; i < cur2.size(); i += cols) inWrap.push_back(cur2.substr(i, cols)); cur2.clear(); };
        for (char c : g.aiInput) { if (c == '\n') flush(); else cur2 += c; } flush(); }
    if (inWrap.empty()) inWrap.push_back("");
    int inputRows = std::min(4, std::max(1, (int)inWrap.size())); float inH = lh * inputRows + 12 * s;   // input box grows up to 4 lines
    float oy = uby + btnH + 8 * s, oh = bh - (oy - by) - inH - 14 * s; if (oh < lh * 3) oh = lh * 3;
    // word-wrap the transcript so chat answers are fully readable (not ".."-truncated)
    vector<std::pair<string,bool>> disp;                            // (text, isPromptLine)
    for (auto& l : g.aiOut) { bool pr = l.rfind(">", 0) == 0; if (l.empty()) { disp.push_back({"", pr}); continue; }
        for (size_t i = 0; i < l.size(); i += cols) disp.push_back({ l.substr(i, cols), pr && i == 0 }); }
    ui::rect(x + 8 * s, oy, obw, oh, th::SUNKEN); ui::rectLine(x + 8 * s, oy, obw, oh, th::BORDER);
    ui::pushClip(x + 8 * s, oy, obw, oh); int rows = (int)(oh / lh);
    if (ui::hovered(x + 8 * s, oy, obw, oh)) g.aiScroll -= (int)ui::in.wheel;   // wheel up = older
    int maxTop = std::max(0, (int)disp.size() - rows); if (g.aiScroll > maxTop) g.aiScroll = maxTop; if (g.aiScroll < 0) g.aiScroll = 0;
    if (disp.empty()) { ui::text(x + 14 * s, oy + 6 * s, "Ask Claude about this binary —", th::TEXT_MUT); ui::text(x + 14 * s, oy + 6 * s + lh, "type below, or use the buttons above.", th::TEXT_MUT); }
    ui::beginText();
    for (int r = 0; r < rows; r++) { int li = g.aiScroll + r; if (li >= (int)disp.size()) break; ui::emit(x + 14 * s, oy + 4 * s + r * lh, disp[li].first, disp[li].second ? th::ACCENT2 : th::Col{0.78f,0.80f,0.84f}); }
    ui::endText(); ui::popClip(); ui::scrollbar(x + 8 * s + obw - 7 * s, oy, 7 * s, oh, (int)disp.size(), rows, &g.aiScroll);
    // ── chat input box (multi-line: Enter = send · Shift+Enter = newline) ──
    float iy = oy + oh + 6 * s, iw = obw; bool foc = g.focus == F_AI, busy = aiBusy.load();
    if (foc && !g.paletteOpen && g.aiOptIn && !busy) {              // consume keys BEFORE drawing the field (no one-frame lag)
        if (ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) { if (!g.aiInput.empty()) g.aiInput.pop_back(); } else if (ui::in.ch >= 32 && ui::in.ch < 127) g.aiInput += (char)ui::in.ch; }
        if (ui::in.key == ui::K_ENTER) { if (ui::in.shift) g.aiInput += '\n'; else if (!model::trim(g.aiInput).empty()) { runAsk(model::trim(g.aiInput)); g.aiInput.clear(); } }
        if (ui::in.key == ui::K_ESC) g.focus = F_NONE;
    }
    ui::rect(x + 8 * s, iy, iw, inH, th::SUNKEN); ui::rectLine(x + 8 * s, iy, iw, inH, foc ? th::ACCENT2 : th::BORDER);
    ui::pushClip(x + 8 * s, iy, iw, inH);
    if (g.aiInput.empty() && !busy) ui::text(x + 14 * s, iy + 5 * s, !g.aiOptIn ? "enable Claude to chat" : "ask Claude  (Enter = send \xc2\xb7 Shift+Enter = newline)", th::TEXT_MUT);
    else { int startR = std::max(0, (int)inWrap.size() - inputRows);   // show the last lines so the caret stays on-screen
        ui::beginText(); for (int r = startR; r < (int)inWrap.size(); r++) ui::emit(x + 14 * s, iy + 5 * s + (r - startR) * lh, inWrap[r], busy ? th::TEXT_MUT : th::TEXT); ui::endText();
        if (foc && g.aiOptIn && !busy) { float cxp = x + 14 * s + ui::textW(inWrap.back()) + 1 * s; ui::rect(cxp, iy + 5 * s + ((int)inWrap.size() - 1 - startR) * lh, std::max(1.5f * s, 1.0f), lh, th::ACCENT2); } }
    ui::popClip();
    if (ui::clicked(x + 8 * s, iy, iw, inH) && g.aiOptIn) g.focus = F_AI;
}
inline void panelLog(float x, float y, float w, float h) {     // body only (tab strip provides the label)
    float s = ui::in.scale, lh = ui::lineH();
    ui::rect(x, y, w, h, th::SUNKEN); ui::pushClip(x, y, w, h); int rows = (int)(h / lh), n = (int)g.bin.log.size();
    if (g.logScroll > n - rows) g.logScroll = std::max(0, n - rows); if (g.logScroll < 0) g.logScroll = 0;
    if (ui::hovered(x, y, w, h)) g.logScroll += (int)ui::in.wheel;
    if (g.logScroll > n - rows) g.logScroll = std::max(0, n - rows); if (g.logScroll < 0) g.logScroll = 0;   // re-clamp after the wheel
    g.logStick = (n <= rows) || (g.logScroll >= n - rows);   // at the bottom -> keep following; scrolled up -> stay put
    static LineSel logSel; runLineSel(logSel, g.bin.log, x, y, w, h, x + 8 * s + ui::advance(), y, g.logScroll, ui::advance(), lh);   // drag-select + copy
    for (int r = 0; r < rows; r++) { int li = g.logScroll + r; if (li >= n) break; const string& m = g.bin.log[li];
        Col c = m.rfind("error", 0) == 0 ? th::RED : th::Col{0.66f,0.68f,0.72f}; ui::text(x + 8 * s + ui::advance(), y + r * lh, m, c); ui::text(x + 6 * s, y + r * lh, ">", th::TEXT_MUT); }
    ui::popClip(); ui::scrollbar(x + w - 7 * s, y, 7 * s, h, n, rows, &g.logScroll);
}
// map a 256-color palette index (-1 = default) to a screen color
inline th::Col termColor(int idx, th::Col def) {
    if (idx < 0) return def;
    static const float b16[16][3] = {
        {0.12f,0.13f,0.15f},{0.86f,0.30f,0.32f},{0.45f,0.80f,0.45f},{0.85f,0.74f,0.38f},
        {0.40f,0.58f,0.88f},{0.80f,0.50f,0.82f},{0.38f,0.78f,0.82f},{0.82f,0.84f,0.82f},
        {0.42f,0.44f,0.48f},{0.95f,0.45f,0.45f},{0.58f,0.92f,0.58f},{0.95f,0.88f,0.52f},
        {0.52f,0.70f,0.98f},{0.92f,0.62f,0.92f},{0.52f,0.90f,0.94f},{1.0f,1.0f,1.0f} };
    if (idx < 16) return th::Col{ b16[idx][0], b16[idx][1], b16[idx][2] };
    if (idx < 232) { int i = idx - 16, r = i / 36, g = (i / 6) % 6, bl = i % 6;
        auto v = [](int n) { return n == 0 ? 0.0f : 0.16f + n * 0.156f; };
        return th::Col{ v(r), v(g), v(bl) }; }
    float gr = 0.04f + (idx - 232) * 0.0385f; return th::Col{ gr, gr, gr };
}
// render the visible rows of a terminal session with per-cell color (grid VT)
inline void emitTermGrid(term::Pty& p, float x, float y, int rows, int top, int n, float adv, float lh, th::Col def, bool dead) {
    ui::beginText();
    for (int r = 0; r < rows; r++) { int li = top + r; if (li < 0 || li >= n) continue; float yy = y + r * lh;
        th::Col lineDef = (li == n - 1 && dead) ? th::TEXT_MUT : def;
        int c = 0;
        while (true) { term::Cell* cell = p.cell(li, c); if (!cell) break;
            if (cell->ch == " " && cell->fg < 0) { c++; continue; }            // default blank -> leave panel bg
            int fg = cell->fg; string run; int sc = c;
            while (true) { term::Cell* cc = p.cell(li, c); if (!cc || cc->fg != fg || (cc->ch == " " && cc->fg < 0)) break; run += cc->ch; c++; }
            if (run.empty()) { c++; continue; }
            ui::emit(x + sc * adv, yy, run, termColor(fg, lineDef)); } }
    ui::endText();
}
inline void panelTerminal(float x, float y, float w, float h) {
    float s = ui::in.scale, lh = ui::lineH() * 1.02f, adv = ui::advance();
    ui::rect(x, y, w, h, th::Col{0.055f, 0.057f, 0.066f});
    if (!term::started) { if (ui::button(x + 12 * s, y + 12 * s, 170 * s, ui::lineH() + 12 * s, "Start Terminal", true)) { term::start(); g.focus = F_TERM; }
        ui::text(x + 12 * s, y + 12 * s + ui::lineH() + 22 * s, "a real shell ($SHELL), themed to match", th::TEXT_MUT); return; }
    int rows = (int)(h / lh), n = (int)term::lines.size();
    if (!ui::menuActive && ui::inside(x, y, w, h) && ui::in.lPress) { g.focus = F_TERM; if (!term::alive) term::start(); n = (int)term::lines.size(); }   // click a dead shell -> restart; re-read n (start() resets the buffer) + don't leak overlay-dismiss clicks
    if (ui::hovered(x, y, w, h)) g.termScroll -= (int)ui::in.wheel;
    if (g.termScroll < 0) g.termScroll = 0; if (g.termScroll > std::max(0, n - rows)) g.termScroll = std::max(0, n - rows);
    int top = std::max(0, n - rows - g.termScroll);
    term::resize(rows, std::max(20, (int)((w - 16 * s) / adv)));   // keep the PTY sized to the panel
    ui::pushClip(x, y, w, h); bool foc = g.focus == F_TERM;
    static LineSel termSel; runLineSel(termSel, term::lines, x, y, w, h, x + 8 * s, y, top, adv, lh);   // drag-select + copy
    if (foc && term::alive && term::main_.vt.cursorVis) { int r = term::main_.caretLine() - top; if (r >= 0 && r < rows) { float cx = x + 8 * s + term::col * adv; ui::rect(cx, y + r * lh + 2 * s, std::max(1.5f * s, 1.0f), lh - 3 * s, th::GREEN); } }   // caret at the real cursor (row,col)
    emitTermGrid(term::main_, x + 8 * s, y, rows, top, n, adv, lh, th::Col{0.80f, 0.84f, 0.80f}, !term::alive);
    ui::popClip();
}
inline void termInput() {
    if (g.focus != F_TERM || g.paletteOpen || !term::started) return;
    if (!term::alive) { if (ui::in.key == ui::K_ENTER) term::start(); return; }   // restart an exited shell on Enter
    if (ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) term::input("\x7f"); else if (ui::in.ch >= 32) term::input(string(1, (char)ui::in.ch)); g.termScroll = 0; }
    switch (ui::in.key) { case ui::K_ENTER: term::input("\r"); g.termScroll = 0; break;
        case ui::K_UP: term::input(term::main_.arrow('A')); g.termScroll = 0; break; case ui::K_DOWN: term::input(term::main_.arrow('B')); g.termScroll = 0; break;
        case ui::K_LEFT: term::input(term::main_.arrow('D')); break; case ui::K_RIGHT: term::input(term::main_.arrow('C')); break;
        case ui::K_HOME: term::input(term::main_.arrow('H')); break; case ui::K_END: term::input(term::main_.arrow('F')); break;
        case ui::K_TAB: term::input("\t"); break; case ui::K_ESC: term::input("\x1b"); break; default: break; }
}
// ── Debugger panel: lldb control buttons + breakpoint field + its own PTY view ─
inline void panelDebugger(float x, float y, float w, float h) {
    float s = ui::in.scale, lh = ui::lineH() * 1.02f, adv = ui::advance();
    ui::rect(x, y, w, h, th::Col{0.055f, 0.057f, 0.066f});
    if (!g.bin.loaded) { ui::text(x + 12 * s, y + 12 * s, "open a binary, then click Debug (the bug icon) to launch lldb", th::TEXT_MUT); return; }
    // ── control bar ──
    float barH = ui::lineH() + 14 * s, bx = x + 8 * s, by = y + 7 * s, bh = ui::lineH() + 8 * s;
    bool live = term::dbg.started && term::dbg.alive;
    struct DB { const char* label; const char* cmd; }; DB btns[] = { {"Run","run"}, {"Cont","continue"}, {"Step","step"}, {"Next","next"}, {"Finish","finish"} };
    for (auto& b : btns) { float bw = ui::textW(b.label) + 16 * s; if (ui::button(bx, by, bw, bh, b.label, false, live)) dbgCmd(b.cmd); bx += bw + 4 * s; }
    if (ui::button(bx, by, ui::textW("Pause") + 16 * s, bh, "Pause", false, live)) term::dbg.input("\x03"); bx += ui::textW("Pause") + 20 * s;
    if (ui::button(bx, by, ui::textW("Quit") + 16 * s, bh, "Quit", false, live)) term::dbg.input("quit\r"); bx += ui::textW("Quit") + 20 * s;
    // breakpoint input ("b <where>")
    float ibx = bx, ibw = std::max(120.f * s, x + w - bx - 10 * s);
    bool bpActive = g.dbgBpFocus && g.focus == F_NONE;   // active only when no other text field owns the keys
    if (bpActive && !g.paletteOpen) { if (ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) { if (!g.dbgBreak.empty()) g.dbgBreak.pop_back(); } else if (ui::in.ch >= 32 && ui::in.ch < 127) g.dbgBreak += (char)ui::in.ch; }
        if (ui::in.key == ui::K_ENTER && !model::trim(g.dbgBreak).empty()) { dbgCmd("b " + model::trim(g.dbgBreak)); g.dbgBreak.clear(); ui::in.key = ui::K_NONE; }   // lldb `b` takes a fn name OR file:line; consume Enter so dbgInput doesn't also send "\r"
        if (ui::in.key == ui::K_ESC) g.dbgBpFocus = false; }
    ui::rect(ibx, by, ibw, bh, th::SUNKEN); ui::rectLine(ibx, by, ibw, bh, bpActive ? th::ACCENT : th::BORDER);
    string bh0 = g.dbgBreak.empty() ? "breakpoint at (fn or file:line)" : g.dbgBreak;
    ui::text(ibx + 6 * s, by + (bh - ui::lineH()) / 2, bh0, g.dbgBreak.empty() ? th::TEXT_MUT : th::TEXT);
    if (bpActive) { float cxp = ibx + 6 * s + ui::textW(g.dbgBreak) + 1 * s; ui::rect(cxp, by + 3 * s, std::max(1.5f * s, 1.0f), bh - 6 * s, th::ACCENT); }
    if (ui::clicked(ibx, by, ibw, bh)) { g.dbgBpFocus = true; g.focus = F_NONE; g.hexSel = -1; }   // grabbing the bp field releases any hex-byte selection (both key off F_NONE)
    else if (ui::in.lPress && !ui::inside(ibx, by, ibw, bh)) g.dbgBpFocus = false;
    // ── lldb output (its own PTY); click to type lldb commands directly ──
    float oy = y + barH, oh = h - barH; ui::hline(x, oy, w, th::BORDER);
    if (!live && !term::dbg.started) { ui::text(x + 12 * s, oy + 10 * s, "lldb not running — click Debug (bug icon) or Run above", th::TEXT_MUT); return; }
    int rows = (int)(oh / lh), n = (int)term::dbg.lines.size();
    if (!ui::menuActive && ui::inside(x, oy, w, oh) && ui::in.lPress) { g.focus = F_DEBUG; g.dbgBpFocus = false; if (!term::dbg.alive) dbgLaunch(); n = (int)term::dbg.lines.size(); }   // !menuActive: don't let an overlay-dismiss click leak here; re-read n after restart resets the buffer (OOB guard)
    if (ui::hovered(x, oy, w, oh)) g.dbgScroll -= (int)ui::in.wheel;
    if (g.dbgScroll < 0) g.dbgScroll = 0; if (g.dbgScroll > std::max(0, n - rows)) g.dbgScroll = std::max(0, n - rows);
    int top = std::max(0, n - rows - g.dbgScroll);
    term::dbg.resize(rows, std::max(20, (int)((w - 16 * s) / adv)));
    ui::pushClip(x, oy, w, oh); bool foc = g.focus == F_DEBUG;
    static LineSel dbgSel; runLineSel(dbgSel, term::dbg.lines, x, oy, w, oh, x + 8 * s, oy, top, adv, lh);   // drag-select + copy
    if (foc && term::dbg.alive && term::dbg.vt.cursorVis) { int r = term::dbg.caretLine() - top; if (r >= 0 && r < rows) {
        float cx = x + 8 * s + term::dbg.col * adv; ui::rect(cx, oy + r * lh + 2 * s, std::max(1.5f * s, 1.0f), lh - 3 * s, th::GREEN); } }
    emitTermGrid(term::dbg, x + 8 * s, oy, rows, top, n, adv, lh, th::Col{0.80f, 0.84f, 0.80f}, !term::dbg.alive);
    ui::popClip();
}
inline void dbgInput() {
    if (g.focus != F_DEBUG || g.paletteOpen || !term::dbg.started) return;
    if (!term::dbg.alive) { if (ui::in.key == ui::K_ENTER) dbgLaunch(); return; }   // restart re-launches lldb, not a bare shell
    if (ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) term::dbg.input("\x7f"); else if (ui::in.ch >= 32) term::dbg.input(string(1, (char)ui::in.ch)); g.dbgScroll = 0; }
    switch (ui::in.key) { case ui::K_ENTER: term::dbg.input("\r"); g.dbgScroll = 0; break;
        case ui::K_UP: term::dbg.input(term::dbg.arrow('A')); g.dbgScroll = 0; break; case ui::K_DOWN: term::dbg.input(term::dbg.arrow('B')); g.dbgScroll = 0; break;
        case ui::K_LEFT: term::dbg.input(term::dbg.arrow('D')); break; case ui::K_RIGHT: term::dbg.input(term::dbg.arrow('C')); break;
        case ui::K_HOME: term::dbg.input(term::dbg.arrow('H')); break; case ui::K_END: term::dbg.input(term::dbg.arrow('F')); break;
        case ui::K_TAB: term::dbg.input("\t"); break; case ui::K_ESC: term::dbg.input("\x1b"); break; default: break; }
}

// ── toolbar / tab strips / status / home ─────────────────────────────────────
inline void toolbar(float w) {
    float s = ui::in.scale, H = th::TOOLBAR_H * s, btn = H - 8 * s, bx = 6 * s, by = 4 * s;
    ui::rect(0, 0, w, H, th::TOOLBAR); ui::hline(0, H - s, w, th::BORDER);
    drawLogo(bx, by, btn); if (ui::clicked(bx, by, btn, btn)) goHome();   // real emblem -> home
    bx += btn + 4 * s;
    ui::text(bx, (H - ui::lineH()) / 2, "EmberDragon", th::ACCENT); bx += ui::textW("EmberDragon") + 14 * s;
    ui::vline(bx, 6 * s, H - 12 * s, th::BORDER); bx += 8 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_HOME, false, "Home  (return to start)")) goHome(); bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_OPEN, false, "Open binary…  (⌘O)")) g.pending = P_OPEN; bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_RELOAD, false, "Re-analyze  (⌘R)") && g.bin.loaded) { g.pending = P_OPENPATH; g.pendingArg = g.bin.path; } bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_EXPORT, false, "Export decomp/  (⌘E)") && g.bin.loaded) g.pending = P_EXPORT; bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_RUN, false, "EmberRun: execute the binary (native or via NXRT)") && g.bin.loaded) runEmberRun(); bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_BUILD, false, "Recompile the decompiled source") && g.bin.loaded) runRecompile(); bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_BUG, false, "Debug with lldb (in the terminal)") && g.bin.loaded) runDebug(); bx += btn + 2 * s;
    if (ui::iconButton(bx, by, btn, ui::IC_SETTINGS, g.settingsOpen, "Settings")) toggleSettings(); bx += btn + 8 * s;
    ui::vline(bx, 6 * s, H - 12 * s, th::BORDER); bx += 8 * s;
    if (g.bin.loaded) { string t = g.bin.name + "   [" + g.bin.arch + "  \xc2\xb7  " + (g.bin.cppOut ? "C++" : "C") + "]"; ui::text(bx, (H - ui::lineH()) / 2, t, th::TEXT); }
    // ── right side: panel toggles + the "Clean Up" button (collapse bloat -> clean source) ──
    float rx = w - btn - 6 * s;
    if (ui::iconButton(rx, by, btn, ui::IC_MENU, g.paletteOpen, "Command palette  (\xe2\x8c\x98\xe2\x87\xa7P)")) togglePalette(); rx -= btn + 2 * s;
    if (ui::iconButton(rx, by, btn, ui::IC_LOG, g.showBottom, "Toggle log / terminal")) { g.showBottom = !g.showBottom; saveConfig(); } rx -= btn + 2 * s;
#ifdef EMBER_AI
    if (ui::iconButton(rx, by, btn, ui::IC_AI, g.showRight, "Toggle AI / xrefs panel")) { g.showRight = !g.showRight; saveConfig(); } rx -= 12 * s;
#else
    if (ui::iconButton(rx, by, btn, ui::IC_XREF, g.showRight, "Toggle xrefs panel")) { g.showRight = !g.showRight; saveConfig(); } rx -= 12 * s;
#endif
    if (g.bin.loaded) {                                                  // the prominent action button: rewrite the raw output toward clean source
        bool busy = optimizing.load(); string lbl = busy ? "Analyzing..." : "Analyze";
        float ow = ui::textW(lbl) + btn + 14 * s, ox = rx - ow;
        bool hov = ui::hovered(ox, by, ow, btn) && !busy;
        ui::rect(ox, by, ow, btn, busy ? th::PANEL_HI : hov ? th::Col{1.0f,0.62f,0.28f} : th::ACCENT);
        ui::icon(ox + 7 * s, by + btn * 0.18f, btn * 0.64f, ui::IC_COLLAPSE, busy ? th::ACCENT2 : th::Col{0.10f,0.09f,0.06f});
        ui::text(ox + btn * 0.64f + 11 * s, by + (btn - ui::lineH()) / 2, lbl, busy ? th::TEXT_DIM : th::Col{0.10f,0.09f,0.06f});
        if (hov) ui::setTip(ox, by + btn + 4 * s, g.aiOptIn ? "Analyze: propagate your renames + deep Claude rewrite -> readable source" : "Analyze: propagate your renames + collapse to readable source (enable Claude in AI panel for the deep pass)");
        if (!busy && ui::clicked(ox, by, ow, btn)) runOptimize();
    }
}
inline void homeScreen(float w, float h) {
    float s = ui::in.scale; ui::rect(0, 0, w, h, th::WINDOW);
    float cw = std::min(w * 0.7f, 560 * s), cx = (w - cw) / 2, cy = h * 0.16f;
    drawLogo(cx, cy - 10 * s, 72 * s);
    ui::text(cx + 84 * s, cy + 6 * s, "EmberDragon", th::ACCENT);
    ui::text(cx + 84 * s, cy + 6 * s + ui::lineH() * 1.2f, "a from-scratch decompiler", th::TEXT_DIM);
    cy += 80 * s;
    float bh = ui::lineH() + 16 * s;
    if (ui::button(cx, cy, 180 * s, bh, "Open Binary", true)) g.pending = P_OPEN;
    if (ui::button(cx + 192 * s, cy, 120 * s, bh, "Quit")) exit(0);
    // reopen-last-session card
    if (!g.lastSource.empty()) { float ry = cy + bh + 14 * s, cardH = ui::lineH() * 2.4f + 12 * s;
        bool hov = ui::hovered(cx, ry, cw, cardH); ui::rect(cx, ry, cw, cardH, hov ? th::HOVER : th::PANEL_HI); ui::rect(cx, ry, 3 * s, cardH, th::ACCENT2);
        ui::text(cx + 14 * s, ry + 8 * s, "RESUME LAST SESSION", th::ACCENT2);
        ui::text(cx + 14 * s, ry + 8 * s + ui::lineH() * 1.15f, store::base(g.lastSource) + "  -  " + g.lastSource, hov ? th::TEXT : th::TEXT_DIM);
        if (ui::clicked(cx, ry, cw, cardH)) { g.pending = P_OPENPATH; g.pendingArg = g.lastSource; }
        cy = ry + cardH + 14 * s; }
    else cy += bh + 24 * s;
    ui::text(cx, cy, "RECENT", th::TEXT_DIM); cy += ui::lineH() * 1.5f;
    float rh = ui::lineH() + 10 * s;
    if (g.recent.empty()) ui::text(cx + 4 * s, cy, "no recent files - open one to begin", th::TEXT_MUT);
    for (size_t i = 0; i < g.recent.size() && i < 10; i++) { float ry = cy + i * rh; bool hov = ui::hovered(cx, ry, cw, rh);
        if (hov) ui::rect(cx, ry, cw, rh, th::HOVER);
        ui::icon(cx + 4 * s, ry + (rh - 13 * s) / 2, 13 * s, ui::IC_FILE, th::TEXT_DIM);
        ui::text(cx + 26 * s, ry + (rh - ui::lineH()) / 2, model::basename(g.recent[i]), hov ? th::TEXT : th::Col{0.74f,0.76f,0.8f});
        ui::textClip(cx + 26 * s + 180 * s, ry + (rh - ui::lineH()) / 2, cw - 210 * s, g.recent[i], th::TEXT_MUT);
        if (ui::clicked(cx, ry, cw, rh)) { g.pending = P_OPENPATH; g.pendingArg = g.recent[i]; }
    }
}

// ── context-menu dispatch ────────────────────────────────────────────────────
inline void dispatchMenu() {
    if (ui::menu.picked < 0) return; int p = ui::menu.picked;
    if (p == ACT_COPY) { if (setClipboard) { if (!g.ctxText.empty()) setClipboard(g.ctxText); else if (activeDoc && activeDoc->hasSel()) copySelection(); } g.ctxText.clear(); }
    else if (p == ACT_SELALL) { selectAllActive(); }
    else if (p == ACT_RENAME) { if (activeDoc == &docFile && docFile.editable) { g.focus = F_DOC; docFile.startRename(); } else { activeDoc = &docPseudo; g.focus = F_DOC; docPseudo.startRename(); } }
    else if (p == ACT_DEF) { if (!docFile.lspPath.empty()) { activeDoc = &docFile; g.focus = F_DOC; gotoDefinition(docFile); } }
    else if (p == ACT_FIX) { applyAvailableFix(); }
    else if (p == ACT_COPY_NAME) { if (setClipboard && g.ctxTarget >= 0 && g.ctxTarget < (int)g.bin.funcs.size()) setClipboard(g.bin.funcs[g.ctxTarget].name); }
    else if (p == ACT_COPY_FUNC) { if (setClipboard && g.ctxTarget >= 0 && g.ctxTarget < (int)g.bin.funcs.size()) { string t; for (auto& l : g.bin.funcs[g.ctxTarget].lines) t += l + "\n"; setClipboard(t); } }
    else if (p == ACT_PATCHFN) patchFnFromPseudo();   // edit the C -> recompile this function -> patch its bytes in place (palette id 23 too)
    else if (p == ACT_TODISASM) { int fi = g.selFn; if (fi >= 0 && fi < (int)g.bin.funcs.size() && g.bin.funcs[fi].addr) {   // jump from a pseudo line to its function's disassembly
        g.tabOpen[MAIN_DISASM] = true; g.pendingMainTab = MAIN_DISASM; g.disasmGotoAddr = g.bin.funcs[fi].addr; g.focus = F_DOC; } }
    else if (p == ACT_GOTO) { if (g.ctxTarget >= 0) { g.selFn = g.ctxTarget; g.codeScroll = 0; g.pseudoJump = true; } }
    else if (p == ACT_XREF) { g.showRight = true; }
    else if (p == ACT_EXPORT) g.pending = P_EXPORT;
    else if (p == ACT_TOBYTES) { if (g.asmOff != SIZE_MAX && g.asmOff < g.bin.bytes.size()) {   // jump from an asm instruction to its raw bytes
        g.tabOpen[MAIN_HEX] = true; g.mainTab = MAIN_HEX; g.hexSel = (int)g.asmOff; g.hexNibble = 0; g.hexScroll = std::max(0, (int)(g.asmOff / 16) - 3); g.focus = F_NONE; } }
    else if (p == ACT_NOP) { if (g.asmOff != SIZE_MAX && g.asmLen > 0 && g.asmOff + (size_t)g.asmLen <= g.bin.bytes.size()) {   // NOP: arm64=0xd503201f, x86=0x90 — fill the whole instruction
        static const unsigned char A64NOP[4] = { 0x1f, 0x20, 0x03, 0xd5 };
        for (int k = 0; k < g.asmLen; k++) setPatchByte(g.asmOff + k, isX86() ? 0x90 : A64NOP[k % 4]);
        patchCommit(); g.patchDirty = true; refreshDisasmFromPatched();
        char b[80]; snprintf(b, sizeof b, "NOP'd %d byte(s) at +0x%zx  (\xe2\x8c\x98S to write)", g.asmLen, g.asmOff); g.log(b); g.hexSel = (int)g.asmOff; } }
    else if (p == ACT_ASMPATCH) { if (g.asmOff != SIZE_MAX && g.asmLen > 0) { g.asmPatchPrompt = true; g.asmPatchText = g.asmText; g.asmPatchErr.clear(); g.focus = F_NONE; } }
}

// ── splitter dragging ────────────────────────────────────────────────────────
inline void splitter(float x, float y, float hgt, int id, float* val, bool horiz, float lo, float hi) {
    float s = ui::in.scale, t = 4 * s;
    bool hov = horiz ? ui::hovered(x - t, y, t * 2, hgt) : ui::hovered(x, y - t, hgt, t * 2);
    if (hov) (horiz ? ui::rect(x - s, y, 2 * s, hgt, th::ACCENT) : ui::rect(x, y - s, hgt, 2 * s, th::ACCENT));
    if (ui::in.lPress && hov) g.dragSplit = id;
    if (g.dragSplit == id && ui::in.lDown) { float v = horiz ? ui::in.mx : ui::in.my; *val = std::max(lo, std::min(hi, id == 2 ? (ui::winW - v) / s : id == 3 ? (ui::winH - v) / s : v / s)); }
}

// ── main entry ───────────────────────────────────────────────────────────────
inline void togglePalette() { g.paletteOpen = !g.paletteOpen; g.paletteQuery.clear(); g.paletteSel = 0; if (g.paletteOpen) { g.focus = F_NONE; paletteJustOpened = true; } }
inline void toggleSettings() { g.settingsOpen = !g.settingsOpen; if (g.settingsOpen) settingsJustOpened = true; }
inline void drawSettings() {
    if (!g.settingsOpen) return; float s = ui::in.scale;
    bool justOpened = settingsJustOpened; settingsJustOpened = false;   // the opening click is not an outside-click
    ui::rect(0, 0, ui::winW, ui::winH, th::Col{0, 0, 0, 0.5f});
    float w = 540 * s, h = 404 * s, x = (ui::winW - w) / 2, y = (ui::winH - h) / 2;
    // raw lPress hit-test (bypasses the menuActive gate that swallows clicked() while this overlay is up)
    auto hit = [&](float a, float b, float ww, float hh) { return ui::in.lPress && ui::in.mx >= a && ui::in.mx < a + ww && ui::in.my >= b && ui::in.my < b + hh; };
    if (!justOpened && ui::in.lPress && !(ui::in.mx >= x && ui::in.mx < x + w && ui::in.my >= y && ui::in.my < y + h)) { g.settingsOpen = false; return; }   // click outside closes
    if (ui::in.key == ui::K_ESC) { g.settingsOpen = false; return; }
    ui::rect(x, y, w, h, th::PANEL); ui::rectLine(x, y, w, h, th::ACCENT);
    ui::icon(x + 14 * s, y + 15 * s, 18 * s, ui::IC_SETTINGS, th::ACCENT); ui::text(x + 42 * s, y + 16 * s, "Settings", th::TEXT);
    { float cxb = x + w - 30 * s, cyb = y + 11 * s; bool hov = ui::hovered(cxb, cyb, 20 * s, 20 * s); if (hov) ui::rect(cxb, cyb, 20 * s, 20 * s, th::HOVER);
      ui::icon(cxb + 3 * s, cyb + 3 * s, 14 * s, ui::IC_CLOSE, hov ? th::TEXT : th::TEXT_DIM); if (hit(cxb, cyb, 20 * s, 20 * s)) { g.settingsOpen = false; return; } }
    float cy = y + 54 * s, rh = ui::lineH() + 14 * s;
    auto toggle = [&](const string& label, bool& val) {
        if (ui::hovered(x + 16 * s, cy, w - 32 * s, rh)) ui::rect(x + 16 * s, cy, w - 32 * s, rh, th::HOVER);
        ui::text(x + 22 * s, cy + (rh - ui::lineH()) / 2, label, th::TEXT);
        float bw = 44 * s, bh = ui::lineH() + 4 * s, tx = x + w - bw - 26 * s, ty = cy + (rh - bh) / 2;
        ui::rect(tx, ty, bw, bh, val ? th::ACCENT : th::SUNKEN); ui::rectLine(tx, ty, bw, bh, th::BORDER);
        ui::rect(val ? tx + bw - bh + 2 * s : tx + 2 * s, ty + 2 * s, bh - 4 * s, bh - 4 * s, val ? th::Col{0.1f,0.09f,0.06f} : th::TEXT_MUT);
        if (hit(x + 16 * s, cy, w - 32 * s, rh)) { val = !val; saveConfig(); } cy += rh; };
    toggle("Use Claude (claude CLI) for AI passes", g.aiOptIn);
#ifdef EMBER_AI
    toggle("Show right panel (AI / xrefs)", g.showRight);
#else
    toggle("Show right panel (xrefs)", g.showRight);
#endif
    toggle("Show bottom panel (log / terminal)", g.showBottom);
    // ── opt-in: precise struct-array field widths (best-effort; re-decompiles to apply) ──
    { if (ui::hovered(x + 16 * s, cy, w - 32 * s, rh)) ui::rect(x + 16 * s, cy, w - 32 * s, rh, th::HOVER);
      ui::text(x + 22 * s, cy + (rh - ui::lineH()) / 2, "Precise struct field widths (experimental)", th::TEXT);
      float bw = 44 * s, bh = ui::lineH() + 4 * s, tx = x + w - bw - 26 * s, ty = cy + (rh - bh) / 2;
      ui::rect(tx, ty, bw, bh, g.structWidths ? th::ACCENT : th::SUNKEN); ui::rectLine(tx, ty, bw, bh, th::BORDER);
      ui::rect(g.structWidths ? tx + bw - bh + 2 * s : tx + 2 * s, ty + 2 * s, bh - 4 * s, bh - 4 * s, g.structWidths ? th::Col{0.1f,0.09f,0.06f} : th::TEXT_MUT);
      if (hit(x + 16 * s, cy, w - 32 * s, rh)) { g.structWidths = !g.structWidths; saveConfig();
          g.log(g.structWidths ? "precise struct widths ON — field types inferred from observed accesses (best-effort; verify layout)" : "precise struct widths OFF (gap-based, conservative)");
          if (g.bin.loaded && !analyzing.load() && !anaPath.empty()) { g.log("re-decompiling to apply…"); openBinary(anaPath); g.settingsOpen = false; return; } }
      cy += rh;
      ui::textClip(x + 22 * s, cy, w - 44 * s, "inferred from observed accesses — unobserved bytes become padding; verify the layout before trusting it", th::TEXT_MUT); cy += ui::lineH() + 6 * s; }
    cy += 8 * s; ui::hline(x + 16 * s, cy, w - 32 * s, th::BORDER); cy += 12 * s;
    ui::text(x + 22 * s, cy, "Projects:  " + store::projectsRoot(), th::TEXT_MUT); cy += ui::lineH() + 5 * s;
    ui::text(x + 22 * s, cy, "Config:    " + store::root() + "/config.json", th::TEXT_MUT); cy += ui::lineH() + 12 * s;
    { float bw = 160 * s, bh = ui::lineH() + 10 * s; bool hov = ui::hovered(x + 22 * s, cy, bw, bh);
      ui::rect(x + 22 * s, cy, bw, bh, hov ? th::HOVER : th::PANEL_HI); ui::rectLine(x + 22 * s, cy, bw, bh, th::BORDER);
      ui::text(x + 22 * s + (bw - ui::textW("Reset layout")) / 2, cy + (bh - ui::lineH()) / 2, "Reset layout", th::TEXT);
      if (hit(x + 22 * s, cy, bw, bh)) { g.sideW = 260; g.rightW = 300; g.bottomH = 140; g.showRight = g.showBottom = true; saveConfig(); } }
}
// a button inside a modal overlay — uses a RAW lPress hit-test, because ui::clicked() is gated by
// menuActive (which is true while our overlay is up) and would otherwise swallow the click.
inline bool modalButton(float x, float y, float w, float h, const string& label, bool primary = false) {
    bool hov = ui::in.mx >= x && ui::in.mx < x + w && ui::in.my >= y && ui::in.my < y + h;
    ui::rect(x, y, w, h, primary ? th::ACCENT : (hov ? th::HOVER : th::PANEL_HI)); ui::rectLine(x, y, w, h, th::BORDER);
    ui::text(x + (w - ui::textW(label)) / 2, y + (h - ui::lineH()) / 2, label, primary ? th::Col{0.07f, 0.06f, 0.04f} : (hov ? th::TEXT : th::TEXT_DIM));
    return ui::in.lPress && hov;
}
// ── assemble-a-replacement-instruction overlay ────────────────────────────────
inline void drawAsmPatch() {
    if (!g.asmPatchPrompt) return; float s = ui::in.scale;
    bool justOpened = asmPatchJustOpened; asmPatchJustOpened = false;   // the Enter that opened this editor is not a "commit"
    if (ui::in.ch) { if (ui::in.ch == 8 || ui::in.ch == 127) { if (!g.asmPatchText.empty()) g.asmPatchText.pop_back(); } else if (ui::in.ch >= 32 && ui::in.ch < 127) g.asmPatchText += (char)ui::in.ch; }   // input first (no lag)
    bool doit = !justOpened && ui::in.key == ui::K_ENTER;
    ui::rect(0, 0, ui::winW, ui::winH, th::Col{0, 0, 0, 0.5f});
    float w = 540 * s, h = 206 * s, x = (ui::winW - w) / 2, y = (ui::winH - h) / 2, lh = ui::lineH();
    ui::rect(x, y, w, h, th::PANEL); ui::rectLine(x, y, w, h, th::ACCENT);
    ui::icon(x + 14 * s, y + 14 * s, 18 * s, ui::IC_BUILD, th::ACCENT); ui::text(x + 42 * s, y + 15 * s, "Patch instruction", th::TEXT);
    char hdr[96]; snprintf(hdr, sizeof hdr, "at +0x%zx   ·   %d-byte slot   ·   %s", g.asmOff, g.asmLen, g.bin.arch.c_str());
    ui::text(x + 16 * s, y + 44 * s, hdr, th::TEXT_MUT);
    ui::text(x + 16 * s, y + 44 * s + lh * 1.15f, "was:  " + g.asmText, th::TEXT_DIM);
    float iy = y + 96 * s, ih = lh + 12 * s; ui::rect(x + 16 * s, iy, w - 32 * s, ih, th::SUNKEN); ui::rectLine(x + 16 * s, iy, w - 32 * s, ih, th::ACCENT2);
    ui::text(x + 22 * s, iy + (ih - lh) / 2, g.asmPatchText.empty() ? "replacement instruction (e.g. mov w0, #0)" : g.asmPatchText, g.asmPatchText.empty() ? th::TEXT_MUT : th::TEXT);
    if (!g.asmPatchText.empty()) ui::rect(x + 22 * s + ui::textW(g.asmPatchText) + 1 * s, iy + 3 * s, std::max(1.5f * s, 1.0f), ih - 6 * s, th::ACCENT2);
    if (!g.asmPatchErr.empty()) ui::text(x + 16 * s, iy + ih + 4 * s, g.asmPatchErr, th::RED);
    float bh = lh + 12 * s, by = y + h - bh - 14 * s;
    float w1 = ui::textW("Assemble & Patch") + 28 * s; if (modalButton(x + 16 * s, by, w1, bh, "Assemble & Patch", true)) doit = true;
    float w3 = ui::textW("Cancel") + 28 * s; if (modalButton(x + w - w3 - 16 * s, by, w3, bh, "Cancel")) g.asmPatchPrompt = false;
    if (ui::in.key == ui::K_ESC) g.asmPatchPrompt = false;
    if (doit) {
        vector<unsigned char> nb = assembleInstr(g.asmPatchText, g.bin.arch);
        if (nb.empty()) g.asmPatchErr = "couldn't assemble that — check the syntax";
        else if (!isX86() && nb.size() % 4 != 0) g.asmPatchErr = "arm64 patches must be whole 4-byte instructions";
        else if ((int)nb.size() > g.asmLen) { char e[80]; snprintf(e, sizeof e, "%d bytes won't fit a %d-byte slot", (int)nb.size(), g.asmLen); g.asmPatchErr = e; }
        else if (patchInstrAt(g.asmOff, nb, g.asmLen)) { char b[96]; snprintf(b, sizeof b, "patched +0x%zx -> %s  (\xe2\x8c\x98S to write)", g.asmOff, g.asmPatchText.c_str()); g.log(b); g.asmPatchPrompt = false; g.hexSel = (int)g.asmOff; }
        else g.asmPatchErr = "patch failed (out of range)";
    }
}
// ── save-patches prompt: overwrite the original (keeps .bak) or save a new binary ──
inline void drawSavePrompt() {
    if (!g.savePrompt) return; float s = ui::in.scale;
    ui::rect(0, 0, ui::winW, ui::winH, th::Col{0, 0, 0, 0.5f});
    float w = 460 * s, h = 168 * s, x = (ui::winW - w) / 2, y = (ui::winH - h) / 2;
    auto hit = [&](float a, float b, float ww, float hh) { return ui::in.lPress && ui::in.mx >= a && ui::in.mx < a + ww && ui::in.my >= b && ui::in.my < b + hh; };
    ui::rect(x, y, w, h, th::PANEL); ui::rectLine(x, y, w, h, th::ACCENT);
    ui::icon(x + 14 * s, y + 14 * s, 18 * s, ui::IC_SAVE, th::ACCENT); ui::text(x + 42 * s, y + 15 * s, "Save " + std::to_string((int)g.patched.size()) + " byte patch(es)", th::TEXT);
    ui::text(x + 16 * s, y + 46 * s, g.bin.name, th::TEXT_DIM);
    ui::text(x + 16 * s, y + 46 * s + ui::lineH() * 1.2f, "Overwrite the original (a .bak is kept), or save a new binary?", th::TEXT_MUT);
    float bh = ui::lineH() + 12 * s, by = y + h - bh - 14 * s, bx = x + 16 * s;
    float w1 = ui::textW("Overwrite") + 28 * s; if (modalButton(bx, by, w1, bh, "Overwrite", true)) { writePatched(g.bin.path, true); g.savePrompt = false; } bx += w1 + 8 * s;
    float w2 = ui::textW("Save as new\xe2\x80\xa6") + 28 * s; if (modalButton(bx, by, w2, bh, "Save as new\xe2\x80\xa6")) { g.savePrompt = false; g.pending = P_SAVEAS; } bx += w2 + 8 * s;
    float w3 = ui::textW("Cancel") + 28 * s; if (modalButton(x + w - w3 - 16 * s, by, w3, bh, "Cancel")) g.savePrompt = false;
    if (ui::in.key == ui::K_ESC) g.savePrompt = false;
    if (hit(0, 0, ui::winW, ui::winH) && !(ui::in.mx >= x && ui::in.mx < x + w && ui::in.my >= y && ui::in.my < y + h)) g.savePrompt = false;
}
// ── AI-proposed byte patches: REVIEW & CONFIRM before a single byte is written ──
// Claude's @nop/@patch tools never touch the binary directly — they land here. The user
// sees exactly what bytes will change (and the original instruction) and must click Apply.
inline void drawAiPatch() {
    if (!g.aiPatchPrompt) return;
    if (g.aiPatchQ.empty()) { g.aiPatchPrompt = false; return; }
    float s = ui::in.scale, lh = ui::lineH(); int n = (int)g.aiPatchQ.size();
    bool justOpened = aiPatchJustOpened; aiPatchJustOpened = false;   // the opening frame's Enter is not an Apply (the modal can open the same frame a live Enter exists)
    bool apply = !justOpened && ui::in.key == ui::K_ENTER;
    if (!justOpened && ui::in.key == ui::K_ESC) { g.aiPatchQ.clear(); g.aiPatchPrompt = false; g.aiPatchScroll = 0; aiPush("  (patches cancelled)"); return; }
    ui::rect(0, 0, ui::winW, ui::winH, th::Col{0, 0, 0, 0.5f});
    float rowH = lh * 2 + 10 * s; int cap = std::min(n, 7); float listH = cap * rowH;   // window of up to 7 rows; scroll to review the rest
    int maxScroll = std::max(0, n - cap); if (g.aiPatchScroll > maxScroll) g.aiPatchScroll = maxScroll; if (g.aiPatchScroll < 0) g.aiPatchScroll = 0;
    float w = 620 * s, h = 104 * s + listH + 54 * s, x = (ui::winW - w) / 2, y = (ui::winH - h) / 2;
    ui::rect(x, y, w, h, th::PANEL); ui::rectLine(x, y, w, h, th::ACCENT2);
    ui::icon(x + 14 * s, y + 14 * s, 18 * s, ui::IC_BUILD, th::ACCENT2);
    char hd[80]; snprintf(hd, sizeof hd, "Claude wants to patch %d instruction%s", n, n == 1 ? "" : "s"); ui::text(x + 42 * s, y + 15 * s, hd, th::TEXT);
    ui::text(x + 16 * s, y + 44 * s, "This rewrites the binary's bytes. Nothing is written until you Apply.", th::TEXT_MUT);
    float ly = y + 72 * s; ui::rect(x + 14 * s, ly, w - 28 * s, listH, th::SUNKEN);
    if (ui::in.mx >= x + 14 * s && ui::in.mx < x + w - 14 * s && ui::in.my >= ly && ui::in.my < ly + listH) g.aiPatchScroll -= (int)ui::in.wheel;   // wheel scrolls the list
    if (g.aiPatchScroll > maxScroll) g.aiPatchScroll = maxScroll; if (g.aiPatchScroll < 0) g.aiPatchScroll = 0;
    ui::pushClip(x + 14 * s, ly, w - 28 * s, listH);
    for (int r = 0; r < cap; r++) { int i = g.aiPatchScroll + r; if (i >= n) break; auto& p = g.aiPatchQ[i]; float ry = ly + r * rowH + 5 * s;
        ui::text(x + 22 * s, ry, p.desc, p.nop ? th::ACCENT : th::GREEN);
        string hx = "bytes:"; char hb[8]; for (auto b : p.bytes) { snprintf(hb, sizeof hb, " %02x", b); hx += hb; }
        ui::text(x + 30 * s, ry + lh, hx, th::NUM); }
    ui::popClip();
    if (n > cap) { char m[64]; snprintf(m, sizeof m, "showing %d-%d of %d  ·  scroll to review all", g.aiPatchScroll + 1, std::min(n, g.aiPatchScroll + cap), n); ui::text(x + 16 * s, ly + listH + 2 * s, m, th::ACCENT2); }
    float bh = lh + 12 * s, by = y + h - bh - 14 * s;
    char ab[40]; snprintf(ab, sizeof ab, "Apply %d patch%s", n, n == 1 ? "" : "es");
    float w1 = ui::textW(ab) + 28 * s; if (modalButton(x + 16 * s, by, w1, bh, ab, true)) apply = true;
    float w3 = ui::textW("Cancel") + 28 * s; if (modalButton(x + w - w3 - 16 * s, by, w3, bh, "Cancel")) { g.aiPatchQ.clear(); g.aiPatchPrompt = false; g.aiPatchScroll = 0; aiPush("  (patches cancelled)"); return; }
    if (apply) {
        int ok = 0; for (auto& p : g.aiPatchQ) if (patchInstrAt(p.off, p.bytes, p.slot)) ok++;
        char b[96]; snprintf(b, sizeof b, "AI patch: applied %d/%d  (\xe2\x8c\x98S to write the binary)", ok, n); g.log(b);
        aiPush("  \xe2\x9c\x93 applied " + std::to_string(ok) + " of " + std::to_string(n) + " patch(es) — \xe2\x8c\x98S to save");
        if (ok) { g.tabOpen[MAIN_DIFF] = true; g.pendingMainTab = MAIN_DIFF; }   // jump to the side-by-side next frame (so its body actually draws)
        g.aiPatchQ.clear(); g.aiPatchPrompt = false; g.aiPatchScroll = 0;
    }
}

inline void render() {
    float s = ui::in.scale;
    pumpAnalysis();   // drain async-decompile progress + finalize a just-finished load
    pumpOptimize();   // drain background clean-up progress into the log
    pumpCompletion(); // apply a finished clangd autocomplete result
    pumpDefinition(); // jump to a finished go-to-definition result
    pumpRename();     // apply a finished clangd semantic-rename WorkspaceEdit
#ifdef EMBER_AI
    pumpAI();         // drain a finished claude rewrite into the AI panel
#endif
    if (g.pendingMainTab >= 0) { g.mainTab = (MainTab)g.pendingMainTab; g.pendingMainTab = -1; }   // honor a deferred tab switch BEFORE the center pane picks what to draw
#ifdef EMBER_AI
    pumpUnderstand(); // apply a finished whole-program rename
#endif
    ui::clipStack.clear(); glDisable(GL_SCISSOR_TEST); glDisable(GL_BLEND); glLineWidth(1);   // known-clean GL state each frame
    ui::rect(0, 0, ui::winW, ui::winH, th::WINDOW);
    if (!ui::in.lDown) g.dragSplit = 0;
    ui::menuActive = g.paletteOpen || g.settingsOpen || g.savePrompt || g.asmPatchPrompt || g.aiPatchPrompt || ui::menu.open;   // overlays swallow clicks to widgets beneath
    if (g.home) { homeScreen(ui::winW, ui::winH); ui::drawMenu(); drawPalette(); return; }
    // process editor keystrokes BEFORE the panels draw, so a typed char shows the same frame (not after the next event)
    if (g.focus == F_DOC && activeDoc && !g.paletteOpen) {
        if (activeDoc == &docPseudo) { if (g.bin.funcs.empty()) activeDoc = nullptr;                                  // refresh against live buffer (reload reallocates)
            else if (g.pseudoWhole) docPseudo.lines = &g.wholeLines;                                                  // whole-program mode -> the combined buffer
            else if (g.selFn < (int)g.bin.funcs.size()) docPseudo.lines = &g.bin.funcs[g.selFn].lines; else activeDoc = nullptr; }
        if (activeDoc) { activeDoc->keys(); if (ui::in.key == ui::K_ESC) { g.focus = F_NONE; activeDoc->clearSel(); } }
    }
    handleHexInput();   // hex-editor byte patching (when the Hex tab owns the keys)

    float W = ui::winW, Hh = ui::winH, tb = th::TOOLBAR_H * s, st = th::STATUS_H * s, tab = th::TAB_H * s;
    // clamp persisted layout to the LIVE window so a config saved on a bigger window can't wedge a smaller one
    g.sideW = std::max(160.f, std::min(g.sideW, W / s - 360));
    g.rightW = std::max(0.f, std::min(g.rightW, W / s - g.sideW - 260));
    g.bottomH = std::max(0.f, std::min(g.bottomH, Hh / s - 200));
    if (!g.showRight && g.focus == F_AI) g.focus = F_NONE;                                   // don't strand keystrokes in a hidden panel
    if (!g.showBottom && (g.focus == F_TERM || g.focus == F_DEBUG)) { g.focus = F_NONE; g.dbgBpFocus = false; }
    if (!(g.showBottom && g.bottomTab == BOTTOM_DEBUG)) g.dbgBpFocus = false;                 // the bp field only exists while the Debugger panel is visible
    toolbar(W);
    float bottom = g.showBottom ? g.bottomH * s : 0;
    float right = g.showRight ? g.rightW * s : 0;
    float side = g.sideW * s;
    float minC = 260 * s;                                  // keep the center pane usable on small/zoomed windows
    if (W - side - right < minC) right = std::max(0.f, W - side - minC);
    if (W - side - right < minC) side = std::max(140.f * s, W - right - minC);
    float bodyY = tb, bodyH = Hh - tb - st - bottom;

    // ── left dock (tab strip: Funcs / Syms / Strs / Files) ──
    float ltabW = side / 4;
    const char* lt[4] = { "Funcs", "Syms", "Strs", "Files" }; ui::Icon licon[4] = { ui::IC_FUNC, ui::IC_SYM, ui::IC_STR, ui::IC_OPEN };
    for (int i = 0; i < 4; i++) { float tw = (i == 3) ? side - ltabW * 3 : ltabW;
        if (ui::tab(ltabW * i, bodyY, tw, tab, licon[i], lt[i], g.sideTab == i)) { g.sideTab = (SideTab)i; g.sideScroll = 0; g.fileScroll = 0; } }
    float listY, listH;
    if (g.sideTab == SIDE_FILES) { listY = bodyY + tab; listH = bodyH - tab; ui::rect(0, listY, side, listH, th::PANEL); panelFiles(0, listY, side, listH); }
    else { float fb = ui::lineH() + 12 * s; filterBox(6 * s, bodyY + tab + 6 * s, side - 12 * s);
        listY = bodyY + tab + fb; listH = bodyH - tab - fb; ui::rect(0, listY, side, listH, th::PANEL);
        if (g.sideTab == SIDE_FUNCS) panelFunctions(0, listY, side, listH);
        else if (g.sideTab == SIDE_SYMS) panelSymbols(0, listY, side, listH);
        else panelStrings(0, listY, side, listH); }
    ui::vline(side, bodyY, bodyH, th::BORDER);

    // ── center (closable tabs) ──
    float cx = side + s, cw = W - side - right - s, ctx = cx;
    string ft = g.fileViewName;   // tab label: keep the extension visible, truncate the stem with ".."
    if (ft.size() > 16) { size_t d = ft.rfind('.'); string ext = (d != string::npos && ft.size() - d <= 5) ? ft.substr(d) : ""; ft = ft.substr(0, 14 - (int)ext.size() > 1 ? 14 - (int)ext.size() : 1) + ".." + ext; }
    struct TD { MainTab id; ui::Icon ic; const char* label; float w; bool avail; };
    TD tds[] = { { MAIN_PSEUDO, ui::IC_CODE, "Pseudocode", 132 * s, true }, { MAIN_GRAPH, ui::IC_GRAPH, "Graph", 96 * s, true }, { MAIN_DISASM, ui::IC_ASM, "Disassembly", 138 * s, true },
                 { MAIN_HEX, ui::IC_HEX, "Hex", 78 * s, true }, { MAIN_DIFF, ui::IC_COLLAPSE, "Diff", 84 * s, !g.patched.empty() }, { MAIN_FILE, ui::IC_FILE, ft.c_str(), 158 * s, !g.fileViewName.empty() } };
    auto isOpen = [&](MainTab t) { return g.tabOpen[t] && (t != MAIN_FILE || !g.fileViewName.empty()) && (t != MAIN_DIFF || !g.patched.empty()); };
    for (auto& td : tds) { if (!td.avail || !g.tabOpen[td.id]) continue; bool closed = false;
        if (ui::tabX(ctx, bodyY, td.w, tab, td.ic, td.label, g.mainTab == td.id, &closed)) g.mainTab = td.id;
        if (closed) { g.tabOpen[td.id] = false; if (td.id == MAIN_FILE) { g.fileViewName.clear(); g.fileViewPath.clear(); } }
        ctx += td.w; }
    bool anyClosed = !g.tabOpen[MAIN_PSEUDO] || !g.tabOpen[MAIN_GRAPH] || !g.tabOpen[MAIN_DISASM] || !g.tabOpen[MAIN_HEX];   // "+" restores closed standard views
    if (anyClosed && ui::iconButton(ctx + 2 * s, bodyY + (tab - 22 * s) / 2, 22 * s, ui::IC_PLUS, false, "Reopen closed tabs")) { g.tabOpen[MAIN_PSEUDO] = g.tabOpen[MAIN_GRAPH] = g.tabOpen[MAIN_DISASM] = g.tabOpen[MAIN_HEX] = true; }
    if (!isOpen(g.mainTab)) { for (MainTab t : { MAIN_PSEUDO, MAIN_GRAPH, MAIN_DISASM, MAIN_HEX, MAIN_DIFF, MAIN_FILE }) if (isOpen(t)) { g.mainTab = t; break; } }
    ui::rect(cx, bodyY + tab, cw, bodyH - tab, th::PANEL);
    float my = bodyY + tab, mh = bodyH - tab;
    if (!isOpen(g.mainTab)) ui::text(cx + 16 * s, my + 16 * s, "All views closed — click + to reopen", th::TEXT_MUT);
    else if (g.mainTab == MAIN_PSEUDO) panelPseudo(cx, my, cw, mh);
    else if (g.mainTab == MAIN_GRAPH) panelGraph(cx, my, cw, mh);
    else if (g.mainTab == MAIN_DISASM) panelDisasm(cx, my, cw, mh);
    else if (g.mainTab == MAIN_HEX) panelHex(cx, my, cw, mh);
    else if (g.mainTab == MAIN_DIFF) panelDiff(cx, my, cw, mh);
    else panelFileView(cx, my, cw, mh);

    // ── right dock ──
    if (g.showRight) { float rx = W - right; ui::vline(rx, bodyY, bodyH, th::BORDER); ui::rect(rx + s, bodyY, right - s, bodyH, th::PANEL);
#ifdef EMBER_AI
        float aiH = bodyH * 0.62f; panelAI(rx + s, bodyY, right - s, aiH); ui::hline(rx + s, bodyY + aiH, right - s, th::BORDER);
        panelXrefs(rx + s, bodyY + aiH + s, right - s, bodyH - aiH - s);
#else
        panelXrefs(rx + s, bodyY, right - s, bodyH);   // no-AI build: xrefs gets the full right dock (no AI panel)
#endif
        splitter(rx, bodyY, bodyH, 2, &g.rightW, true, 200, W / s - g.sideW - 200);
    }
    // ── bottom dock (tabbed: Log / Terminal) ──
    if (g.showBottom) { float byy = Hh - st - bottom; ui::hline(0, byy, W, th::BORDER); float tabH = th::TAB_H * s;
        if (ui::tab(0, byy, 95 * s, tabH, ui::IC_LOG, "Log", g.bottomTab == BOTTOM_LOG)) g.bottomTab = BOTTOM_LOG;
        if (ui::tab(95 * s, byy, 120 * s, tabH, ui::IC_TERM, "Terminal", g.bottomTab == BOTTOM_TERM)) { g.bottomTab = BOTTOM_TERM; if (!term::started) term::start(); g.focus = F_TERM; }
        if (ui::tab(215 * s, byy, 120 * s, tabH, ui::IC_BUG, "Debugger", g.bottomTab == BOTTOM_DEBUG)) { g.bottomTab = BOTTOM_DEBUG; g.focus = F_DEBUG; }
        float py = byy + tabH, ph = bottom - tabH;
        if (g.bottomTab == BOTTOM_LOG) panelLog(0, py, W, ph); else if (g.bottomTab == BOTTOM_TERM) panelTerminal(0, py, W, ph); else panelDebugger(0, py, W, ph);
        splitter(0, byy, W, 3, &g.bottomH, false, 90, Hh / s - 200); }
    // left splitter
    splitter(side, bodyY, bodyH, 1, &g.sideW, true, 160, 520);

    // ── status bar ──
    float sy = Hh - st; ui::rect(0, sy, W, st, th::TOOLBAR); ui::hline(0, sy, W, th::BORDER);
    if (g.bin.loaded && g.selFn < (int)g.bin.funcs.size()) { string l = g.bin.path; ui::text(8 * s, sy + (st - ui::lineH()) / 2, l, th::TEXT_DIM);
        char r[128]; snprintf(r, sizeof r, "%s   %d fns   %zu syms   %zu strs", g.bin.arch.c_str(), (int)g.bin.funcs.size(), g.bin.syms.size(), g.bin.strings.size());
        ui::textRight(W - 8 * s, sy + (st - ui::lineH()) / 2, r, th::TEXT_DIM); }

    termInput(); dbgInput();
    ui::drawMenu(); dispatchMenu(); drawPalette(); drawSettings(); drawSavePrompt(); drawAsmPatch(); drawAiPatch(); drawDecompiling(); drawCompletion(); ui::drawTooltip();
}
} // namespace app
#endif
